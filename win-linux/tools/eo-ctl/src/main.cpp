// eo-ctl — thin client + process lifecycle manager for the DesktopEditors gateway.
// Per cdp-gateway-cli-plan.md §7: no business logic lives here. Every subcommand just
// frames a JSON request, sends it over the gateway's Unix-domain socket, and prints
// the JSON response — the same GatewayCommandRunner::Execute() call the gateway itself
// makes is what actually runs, per gateway-test-case-designs.md's "Scope of this
// document". If this file starts growing per-command validation logic, that logic
// belongs in the allowlist table instead.

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QThread>
#include <QTextStream>

#include <unistd.h>

#include "connectlogic.h"

namespace
{
    QString SocketPath()
    {
        QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
        if (runtimeDir.isEmpty())
            runtimeDir = QDir::tempPath();
        return QStringLiteral("%1/eo-gateway-%2.sock").arg(runtimeDir).arg(getuid());
    }

    QString TokenPath()
    {
        QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
        if (runtimeDir.isEmpty())
            runtimeDir = QDir::tempPath();
        return QStringLiteral("%1/eo-gateway-%2.token").arg(runtimeDir).arg(getuid());
    }

    // Launches DesktopEditors on `file` and waits (bounded) for its gateway socket to
    // appear, per §7: "if none is running for the target document, it launches
    // DesktopEditors <file> itself (waiting for the gateway socket to come up)".
    bool EnsureEditorRunning(const QString& file, QTextStream& err)
    {
        if (QFile::exists(SocketPath()))
            return true;

        if (!QProcess::startDetached(QStringLiteral("DesktopEditors"), {file}))
        {
            err << "eo-ctl: failed to launch DesktopEditors\n";
            return false;
        }

        const int timeoutMs = 30000;
        const int pollIntervalMs = 200;
        for (int waited = 0; waited < timeoutMs; waited += pollIntervalMs)
        {
            if (QFile::exists(SocketPath()))
                return true;
            QThread::msleep(pollIntervalMs);
        }

        err << "eo-ctl: timed out waiting for the gateway socket to appear\n";
        return false;
    }

    // Shared by `call` and `allowlist` — connect, send one framed request, read one
    // framed response, disconnect. Matches GatewayServer's one-shot-per-connection
    // protocol (gatewayserver.cpp).
    bool SendRequest(const QJsonObject& request, QJsonObject& outResponse, QTextStream& err)
    {
        QFile tokenFile(TokenPath());
        if (!tokenFile.open(QIODevice::ReadOnly))
        {
            err << "eo-ctl: could not read auth token at " << TokenPath() << "\n";
            return false;
        }
        const QString token = QString::fromUtf8(tokenFile.readAll());

        QLocalSocket socket;
        socket.connectToServer(SocketPath());
        if (!socket.waitForConnected(5000))
        {
            err << "eo-ctl: could not connect to gateway socket: " << socket.errorString() << "\n";
            return false;
        }

        QJsonObject framed = request;
        framed.insert(QStringLiteral("auth"), token);
        socket.write(QJsonDocument(framed).toJson(QJsonDocument::Compact) + '\n');
        socket.flush();

        if (!socket.waitForReadyRead(10000) || !socket.canReadLine())
        {
            err << "eo-ctl: no response from gateway (timed out)\n";
            return false;
        }

        outResponse = QJsonDocument::fromJson(socket.readLine().trimmed()).object();
        return true;
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("eo-ctl"));

    QTextStream out(stdout);
    QTextStream err(stderr);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Thin client for the DesktopEditors gateway"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("subcommand"), QStringLiteral("connect | call | allowlist"));
    parser.parse(QCoreApplication::arguments());

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty())
    {
        err << "usage: eo-ctl <connect <file> | call <command> --scope '<json>' | allowlist>\n";
        return 1;
    }

    const QString subcommand = args.first();

    if (subcommand == QStringLiteral("connect"))
    {
        if (args.size() < 2)
        {
            err << "usage: eo-ctl connect <file>\n";
            return 1;
        }

        const QString file = args.at(1);
        const bool socketAlreadyExists = QFile::exists(SocketPath());

        // gateway.connect is a pure resolver (never opens anything itself, see
        // gatewayserver.cpp) -- launching DesktopEditors <file> is what actually
        // opens the document, via SingleApplication's cold-start-or-forward
        // behavior; see connectlogic.h for the full rationale.
        auto resolveViewId = [&file, &err]() -> int {
            QJsonObject request;
            request.insert(QStringLiteral("id"), QStringLiteral("eo-ctl-connect"));
            request.insert(QStringLiteral("command"), QStringLiteral("gateway.connect"));
            QJsonObject scope;
            scope.insert(QStringLiteral("path"), file);
            request.insert(QStringLiteral("scope"), scope);

            QJsonObject response;
            if (!SendRequest(request, response, err) || !response.value(QStringLiteral("ok")).toBool())
                return -1;
            return response.value(QStringLiteral("result")).toObject()
                .value(QStringLiteral("targetViewId")).toInt(-1);
        };

        const int viewId = EoCtl::ConnectAndResolveViewId(
            socketAlreadyExists,
            [&file, &err]() { return EnsureEditorRunning(file, err); },
            resolveViewId,
            [&file]() { QProcess::startDetached(QStringLiteral("DesktopEditors"), {file}); },
            [](int ms) { QThread::msleep(static_cast<unsigned long>(ms)); });

        if (viewId == -1)
        {
            err << "eo-ctl: timed out resolving a view for " << file << "\n";
            return 1;
        }

        QJsonObject result;
        result.insert(QStringLiteral("targetViewId"), viewId);
        out << QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact)) << "\n";
        return 0;
    }

    if (subcommand == QStringLiteral("call"))
    {
        if (args.size() < 2)
        {
            err << "usage: eo-ctl call <command> --scope '<json>'\n";
            return 1;
        }

        QCommandLineOption scopeOption(QStringLiteral("scope"), QStringLiteral("JSON scope object"), QStringLiteral("json"), QStringLiteral("{}"));
        QCommandLineOption targetOption(QStringLiteral("target"), QStringLiteral("target view id"), QStringLiteral("id"), QStringLiteral("-1"));
        QCommandLineParser callParser;
        callParser.addOption(scopeOption);
        callParser.addOption(targetOption);
        callParser.parse(QCoreApplication::arguments());

        const QJsonObject scope = QJsonDocument::fromJson(callParser.value(scopeOption).toUtf8()).object();

        QJsonObject request;
        request.insert(QStringLiteral("id"), QStringLiteral("eo-ctl-1"));
        request.insert(QStringLiteral("command"), args.at(1));
        request.insert(QStringLiteral("scope"), scope);
        request.insert(QStringLiteral("targetViewId"), callParser.value(targetOption).toInt());

        QJsonObject response;
        if (!SendRequest(request, response, err))
            return 1;

        out << QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact)) << "\n";
        return response.value(QStringLiteral("ok")).toBool() ? 0 : 1;
    }

    if (subcommand == QStringLiteral("allowlist"))
    {
        QJsonObject request;
        request.insert(QStringLiteral("id"), QStringLiteral("eo-ctl-allowlist"));
        request.insert(QStringLiteral("command"), QStringLiteral("gateway.listCommands"));
        request.insert(QStringLiteral("scope"), QJsonObject());

        QJsonObject response;
        if (!SendRequest(request, response, err))
            return 1;

        out << QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact)) << "\n";
        return response.value(QStringLiteral("ok")).toBool() ? 0 : 1;
    }

    err << "eo-ctl: unknown subcommand '" << subcommand << "'\n";
    return 1;
}
