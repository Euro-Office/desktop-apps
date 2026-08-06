#include "gatewaycommandrunner.h"
#include "allowlist.h"

#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QWebSocket>

namespace Gateway
{
    GatewayCommandRunner::GatewayCommandRunner(CAscApplicationManager* manager, QObject* parent)
        : QObject(parent)
        , m_manager(manager)
    {
    }

    Result GatewayCommandRunner::Execute(const QString& command, const QJsonObject& scope, int targetViewId)
    {
        const CommandSpec* spec = AllowlistTable::Instance().Find(command);
        if (!spec)
            return Result::Failure(ErrorCode::NotAllowlisted, QStringLiteral("unknown command: %1").arg(command));

        const QString validationError = spec->validate(scope);
        if (!validationError.isEmpty())
            return Result::Failure(ErrorCode::SchemaInvalid, validationError);

        Error targetError{ErrorCode::TargetNotFound, QString()};
        const QString wsUrl = ResolveTargetWebSocketUrl(targetViewId, targetError);
        if (wsUrl.isEmpty())
            return Result::Failure(targetError.code, targetError.message);

        QString script = spec->script;
        script.replace(QStringLiteral("%%SCOPE%%"),
                        QString::fromUtf8(QJsonDocument(scope).toJson(QJsonDocument::Compact)));

        // Drive one CDP Runtime.evaluate round-trip over a fresh WebSocket connection.
        // A connection-per-call is deliberate for now (simplicity over the more
        // efficient long-lived-connection-per-target design) — revisit only if the
        // per-editor gate's test suite shows this is a real latency problem.
        QWebSocket socket;
        QEventLoop loop;
        Result result = Result::Failure(ErrorCode::ScriptException, QStringLiteral("CDP call timed out"));
        int requestId = 1;

        QObject::connect(&socket, &QWebSocket::connected, &loop, [&]() {
            QJsonObject params;
            params.insert(QStringLiteral("expression"), script);
            params.insert(QStringLiteral("returnByValue"), true);
            params.insert(QStringLiteral("awaitPromise"), false);

            QJsonObject request;
            request.insert(QStringLiteral("id"), requestId);
            request.insert(QStringLiteral("method"), QStringLiteral("Runtime.evaluate"));
            request.insert(QStringLiteral("params"), params);

            socket.sendTextMessage(QString::fromUtf8(QJsonDocument(request).toJson(QJsonDocument::Compact)));
        });

        QObject::connect(&socket, &QWebSocket::textMessageReceived, &loop, [&](const QString& message) {
            const QJsonObject response = QJsonDocument::fromJson(message.toUtf8()).object();
            if (response.value(QStringLiteral("id")).toInt() != requestId)
                return; // not our request/response — ignore (defensive; single in-flight request per call)

            const QJsonObject cdpResult = response.value(QStringLiteral("result")).toObject();
            const QJsonObject exceptionDetails = cdpResult.value(QStringLiteral("exceptionDetails")).toObject();
            if (!exceptionDetails.isEmpty())
            {
                const QString message = exceptionDetails.value(QStringLiteral("text")).toString();
                result = Result::Failure(ErrorCode::ScriptException, message);
            }
            else
            {
                const QJsonObject remoteObject = cdpResult.value(QStringLiteral("result")).toObject();
                result = Result::Success(remoteObject.value(QStringLiteral("value")));
            }
            loop.quit();
        });

        QObject::connect(&socket, &QWebSocket::errorOccurred, &loop, [&]() {
            result = Result::Failure(ErrorCode::ScriptException,
                                      QStringLiteral("CDP WebSocket error: %1").arg(socket.errorString()));
            loop.quit();
        });

        QTimer::singleShot(5000, &loop, &QEventLoop::quit); // bounded wait; see A6 test case (no hang on a dead target)

        socket.open(QUrl(wsUrl));
        loop.exec();

        return result;
    }

    QString GatewayCommandRunner::ResolveTargetWebSocketUrl(int /*targetViewId*/, Error& outErrorIfNotFound)
    {
        // See the KNOWN GAP note in gatewaycommandrunner.h — not yet wired to
        // CAscApplicationManager's m_mapViews. Fails closed (TARGET_NOT_FOUND) rather
        // than guessing a target, so callers get test case A5's documented behavior
        // instead of a silent wrong-document write.
        outErrorIfNotFound = Error{ErrorCode::TargetNotFound,
                                    QStringLiteral("target resolution not yet implemented")};
        return QString();
    }
}
