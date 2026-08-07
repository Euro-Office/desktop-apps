#include "gatewayserver.h"
#include "gatewaycommandrunner.h"
#include "gatewaytypes.h"
#include "allowlist.h"
#include "commands/wordcommands.h"
#include "commands/cellcommands.h"
#include "commands/slidecommands.h"

#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QStandardPaths>

#include <unistd.h>
#include <sys/stat.h>

namespace Gateway
{
    GatewayServer::GatewayServer(CAscApplicationManager* manager, QObject* parent)
        : QObject(parent)
        , m_runner(new GatewayCommandRunner(manager, this))
    {
    }

    QString GatewayServer::SocketPath()
    {
        QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
        if (runtimeDir.isEmpty())
            runtimeDir = QDir::tempPath();
        return QStringLiteral("%1/eo-gateway-%2.sock").arg(runtimeDir).arg(getuid());
    }

    QString GatewayServer::TokenPath()
    {
        QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
        if (runtimeDir.isEmpty())
            runtimeDir = QDir::tempPath();
        return QStringLiteral("%1/eo-gateway-%2.token").arg(runtimeDir).arg(getuid());
    }

    QString GatewayServer::GenerateAndPersistToken()
    {
        // QUuid is a convenient source of 128 bits of randomness already linked via
        // QtCore — no new dependency for token generation.
        const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);

        const QString path = TokenPath();
        QFile::remove(path); // drop any stale token from a previous run before recreating
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return QString();
        file.write(token.toUtf8());
        file.close();

        // 0600: readable/writable only by the owning user, matching the auth-token
        // decision in cdp-gateway-cli-plan.md §0.
        ::chmod(path.toUtf8().constData(), S_IRUSR | S_IWUSR);

        return token;
    }

    bool GatewayServer::Start()
    {
        // Registers each implemented command family into AllowlistTable::Instance().
        // Grows in the plan's build order (Word first) as each family lands --
        // Commands::RegisterCellCommands()/RegisterSlideCommands()/RegisterPdfCommands()
        // join this list when those families exist.
        Commands::RegisterWordCommands();
        Commands::RegisterCellCommands();
        Commands::RegisterSlideCommands();

        const QString socketPath = SocketPath();
        QLocalServer::removeServer(socketPath); // clear a stale socket file from a crashed prior run

        m_token = GenerateAndPersistToken();
        if (m_token.isEmpty())
            return false;

        if (!m_server.listen(socketPath))
            return false;

        // QLocalServer on Linux creates the socket file with the process umask; tighten
        // it explicitly to 0600 rather than relying on the caller's umask being correct.
        ::chmod(socketPath.toUtf8().constData(), S_IRUSR | S_IWUSR);

        connect(&m_server, &QLocalServer::newConnection, this, &GatewayServer::OnNewConnection);
        return true;
    }

    void GatewayServer::Stop()
    {
        m_server.close();
        QFile::remove(SocketPath());
        QFile::remove(TokenPath());
    }

    void GatewayServer::OnNewConnection()
    {
        while (QLocalSocket* client = m_server.nextPendingConnection())
        {
            // One-shot request/response per connection for now, matching eo-ctl's
            // `call` subcommand (connect, send one command, read one response,
            // disconnect) — see cdp-gateway-cli-plan.md §7. A persistent-connection
            // mode is not needed until a caller actually wants one.
            connect(client, &QLocalSocket::readyRead, this, [this, client]() {
                if (!client->canReadLine())
                    return;

                const QByteArray line = client->readLine().trimmed();
                const QJsonObject request = QJsonDocument::fromJson(line).object();

                QJsonObject response;
                response.insert(QStringLiteral("id"), request.value(QStringLiteral("id")));

                if (request.value(QStringLiteral("auth")).toString() != m_token)
                {
                    QJsonObject error;
                    error.insert(QStringLiteral("code"), ErrorCodeToString(ErrorCode::Unauthenticated));
                    error.insert(QStringLiteral("message"), QStringLiteral("invalid or missing auth token"));
                    response.insert(QStringLiteral("ok"), false);
                    response.insert(QStringLiteral("error"), error);
                }
                else
                {
                    const QString command = request.value(QStringLiteral("command")).toString();

                    // Meta command, not part of any editor's allowlist table and not
                    // dispatched through CDP — it just reads AllowlistTable's own
                    // registry. Kept as a special case here rather than in
                    // GatewayCommandRunner so Execute() stays "one command == one CDP
                    // round trip", matching every functional test case's assumption.
                    if (command == QStringLiteral("gateway.listCommands"))
                    {
                        QJsonArray names;
                        for (const QString& name : AllowlistTable::Instance().ListCommandNames())
                            names.append(name);
                        response.insert(QStringLiteral("ok"), true);
                        response.insert(QStringLiteral("result"), names);
                    }
                    else
                    {
                        const QJsonObject scope = request.value(QStringLiteral("scope")).toObject();
                        const int targetViewId = request.value(QStringLiteral("targetViewId")).toInt(-1);

                        const Result result = m_runner->Execute(command, scope, targetViewId);
                        response.insert(QStringLiteral("ok"), result.ok);
                        if (result.ok)
                        {
                            response.insert(QStringLiteral("result"), result.value);
                        }
                        else
                        {
                            QJsonObject error;
                            error.insert(QStringLiteral("code"), ErrorCodeToString(result.error.code));
                            error.insert(QStringLiteral("message"), result.error.message);
                            response.insert(QStringLiteral("error"), error);
                        }
                    }
                }

                client->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
                client->flush();
                client->disconnectFromServer();
            });

            connect(client, &QLocalSocket::disconnected, client, &QLocalSocket::deleteLater);
        }
    }
}
