#include "gatewaycommandrunner.h"
#include "allowlist.h"

#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QDir>

#include "applicationmanager.h"
#include "cefview.h"

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

        CCefView* view = m_manager ? m_manager->GetViewById(targetViewId) : nullptr;
        if (!view)
            return Result::Failure(ErrorCode::TargetNotFound,
                                    QStringLiteral("no open document with view id %1").arg(targetViewId));

        QString script = spec->script;
        script.replace(QStringLiteral("%%SCOPE%%"),
                        QString::fromUtf8(QJsonDocument(scope).toJson(QJsonDocument::Compact)));

        const int messageId = m_nextMessageId++;

        QJsonObject params;
        params.insert(QStringLiteral("expression"), script);
        params.insert(QStringLiteral("returnByValue"), true);
        params.insert(QStringLiteral("awaitPromise"), false);

        QJsonObject request;
        request.insert(QStringLiteral("id"), messageId);
        request.insert(QStringLiteral("method"), QStringLiteral("Runtime.evaluate"));
        request.insert(QStringLiteral("params"), params);

        const QByteArray requestBytes = QJsonDocument(request).toJson(QJsonDocument::Compact);

        // CCefView::SendGatewayDevToolsMessage's callback fires on the CEF browser
        // process UI thread -- which, in this single-process-embedded-CEF app, is the
        // same thread pumping the Qt event loop we spin below. A nested QEventLoop is
        // therefore the correct way to make this call look synchronous to
        // GatewayCommandRunner's own caller, matching every functional test case in
        // gateway-test-case-designs.md, which expects Execute() to just return a Result.
        QEventLoop loop;
        Result result = Result::Failure(ErrorCode::ScriptException, QStringLiteral("CDP call timed out"));

        view->SendGatewayDevToolsMessage(requestBytes.toStdString(), messageId,
            [&result, &loop](bool ok, const std::string& jsonResponseOrError) {
                if (!ok)
                {
                    result = Result::Failure(ErrorCode::ScriptException, QString::fromStdString(jsonResponseOrError));
                    loop.quit();
                    return;
                }

                const QJsonObject response = QJsonDocument::fromJson(
                    QByteArray::fromStdString(jsonResponseOrError)).object();
                const QJsonObject cdpResult = response.value(QStringLiteral("result")).toObject();
                const QJsonObject exceptionDetails = cdpResult.value(QStringLiteral("exceptionDetails")).toObject();

                if (!exceptionDetails.isEmpty())
                {
                    result = Result::Failure(ErrorCode::ScriptException,
                                              exceptionDetails.value(QStringLiteral("text")).toString());
                }
                else
                {
                    const QJsonObject remoteObject = cdpResult.value(QStringLiteral("result")).toObject();
                    result = Result::Success(remoteObject.value(QStringLiteral("value")));
                }
                loop.quit();
            });

        QTimer::singleShot(5000, &loop, &QEventLoop::quit); // bounded wait; see test case A6 (no hang on a dead target)
        loop.exec();

        return result;
    }

    int GatewayCommandRunner::ResolveViewIdByPath(const QString& path) const
    {
        if (!m_manager || path.isEmpty())
            return -1;

        QString normalized = QDir::isAbsolutePath(path) ? path : QDir::current().absoluteFilePath(path);
        normalized = QDir::cleanPath(normalized);

        CCefView* view = m_manager->GetViewByUrl(normalized.toStdWString());
        return view ? view->GetId() : -1;
    }
}
