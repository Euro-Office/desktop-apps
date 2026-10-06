#ifndef GATEWAYTYPES_H
#define GATEWAYTYPES_H

#include <QString>
#include <QJsonValue>

// Shared result/error shapes for GatewayCommandRunner::Execute(), used identically
// by the CLI (eo-ctl) and the external gateway wire protocol — both are thin shells
// around the same call, see cdp-gateway-cli-plan.md.

namespace Gateway
{
    enum class ErrorCode
    {
        NotAllowlisted,
        SchemaInvalid,
        TargetNotFound,
        ScriptException,
        Unauthenticated
    };

    inline const char* ErrorCodeToString(ErrorCode code)
    {
        switch (code)
        {
        case ErrorCode::NotAllowlisted: return "NOT_ALLOWLISTED";
        case ErrorCode::SchemaInvalid:  return "SCHEMA_INVALID";
        case ErrorCode::TargetNotFound: return "TARGET_NOT_FOUND";
        case ErrorCode::ScriptException:return "SCRIPT_EXCEPTION";
        case ErrorCode::Unauthenticated:return "UNAUTHENTICATED";
        }
        return "UNKNOWN";
    }

    struct Error
    {
        ErrorCode code;
        QString message;
    };

    struct Result
    {
        bool ok = false;
        QJsonValue value;   // present when ok == true
        Error error{ErrorCode::ScriptException, QString()}; // present when ok == false

        static Result Success(const QJsonValue& v) { Result r; r.ok = true; r.value = v; return r; }
        static Result Failure(ErrorCode code, const QString& message)
        {
            Result r; r.ok = false; r.error = Error{code, message}; return r;
        }
    };
}

#endif // GATEWAYTYPES_H
