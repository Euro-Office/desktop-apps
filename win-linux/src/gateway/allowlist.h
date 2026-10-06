#ifndef GATEWAY_ALLOWLIST_H
#define GATEWAY_ALLOWLIST_H

#include <QString>
#include <QJsonObject>
#include <functional>
#include <map>

#include "gatewaytypes.h"

// One entry per allowlisted command. `validate` is a hand-rolled schema check (no
// generic JSON-schema engine — see cdp-gateway-cli-plan.md §9/YAGNI: this app has no
// existing JSON-schema dependency, and the schemas in gateway-test-case-designs.md are
// simple enough not to need one). `script` is the apiBuilder.js-backed script template;
// its only substitution point is the literal token %%SCOPE%%, filled by
// GatewayCommandRunner with QJsonDocument(scope).toJson(Compact) — never by
// interpolating individual scope fields into the template string. See §2 of the plan.
namespace Gateway
{
    struct CommandSpec
    {
        // Returns an empty QString if `scope` is valid, otherwise a human-readable reason.
        std::function<QString(const QJsonObject& scope)> validate;
        QString script;
    };

    class AllowlistTable
    {
    public:
        static AllowlistTable& Instance();

        // Registers a command. Called from each *Commands.cpp's static registration
        // block (WordCommands.cpp, CellCommands.cpp, SlideCommands.cpp, PdfCommands.cpp).
        void Register(const QString& command, CommandSpec spec);

        // Returns nullptr if `command` is not allowlisted.
        const CommandSpec* Find(const QString& command) const;

        // eo-ctl's `allowlist` subcommand and the gateway's introspection endpoint both
        // read this — command names only, no schema/script internals leaked.
        std::vector<QString> ListCommandNames() const;

    private:
        std::map<QString, CommandSpec> m_commands;
    };

    // --- small reusable scope-field validators, shared across command families ---

    inline QString RequireInt(const QJsonObject& scope, const QString& field, int minimum = 0)
    {
        if (!scope.contains(field) || !scope.value(field).isDouble())
            return QStringLiteral("scope.%1 must be an integer").arg(field);
        double v = scope.value(field).toDouble();
        if (v != static_cast<int>(v) || static_cast<int>(v) < minimum)
            return QStringLiteral("scope.%1 must be an integer >= %2").arg(field).arg(minimum);
        return QString();
    }

    inline QString RequireString(const QJsonObject& scope, const QString& field, bool allowEmpty = true)
    {
        if (!scope.contains(field) || !scope.value(field).isString())
            return QStringLiteral("scope.%1 must be a string").arg(field);
        if (!allowEmpty && scope.value(field).toString().isEmpty())
            return QStringLiteral("scope.%1 must not be empty").arg(field);
        return QString();
    }

    inline QString RequireBool(const QJsonObject& scope, const QString& field)
    {
        if (!scope.contains(field) || !scope.value(field).isBool())
            return QStringLiteral("scope.%1 must be a boolean").arg(field);
        return QString();
    }
}

#endif // GATEWAY_ALLOWLIST_H
