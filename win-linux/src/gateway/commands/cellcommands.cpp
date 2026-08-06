#include "cellcommands.h"
#include "../allowlist.h"

// Cell command family, implemented in the sequential order set by
// cdp-gateway-cli-plan.md §4 ("Cell (second)"). Each command's test cases live in
// gateway-test-case-designs.md under the matching §C heading (this file: §C1).
//
// Script bodies were written against sdkjs/cell/apiBuilder.js as it actually reads --
// not guessed. Every %%SCOPE%% is the sole substitution point, filled by
// GatewayCommandRunner via QJsonDocument(scope).toJson(Compact), never per-field
// string interpolation.

namespace Gateway::Commands
{
    void RegisterCellCommands()
    {
        auto& table = AllowlistTable::Instance();

        // --- C1. Sheet management ---
        //
        // Api.AddSheet(sName) (apiBuilder.js:777, top-level Api, not ApiWorkbook)
        // throws if a sheet with that name already exists. Api.GetSheets() (799)
        // returns ApiWorksheet[], not JSON-serializable -- returned as an array of
        // names via ApiWorksheet.GetName() (8546), same pattern as word.getAllTables
        // etc. ApiWorksheet.SetActive() (8332) is on the worksheet, not
        // ApiWorkbook.SetActiveSheet (no such method exists). Api.GetSheet(name)
        // (867) resolves a sheet by name for target resolution.

        table.Register(QStringLiteral("cell.addSheet"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    Api.AddSheet(scope.name);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.getSheets"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetSheets().map(function(ws){ return ws.GetName(); });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.setActiveSheet"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.name);
                    if (!ws) throw new Error("no sheet named " + scope.name);
                    ws.SetActive();
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.getActiveSheet"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetActiveSheet().GetName();
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.setVisible"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireBool(scope, QStringLiteral("visible"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.name);
                    if (!ws) throw new Error("no sheet named " + scope.name);
                    ws.SetVisible(scope.visible);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.setName"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("oldName"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("newName"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.oldName);
                    if (!ws) throw new Error("no sheet named " + scope.oldName);
                    ws.SetName(scope.newName);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- C2. Cell/range read & write ---
        //
        // ApiWorksheet.GetRange(Range1) (apiBuilder.js:8602) resolves a range string
        // on a given sheet, throwing if it can't. ApiRange.SetValue (10161) is the
        // only setter -- no separate SetFormula exists; a value string starting with
        // "=" becomes a formula through this same call. GetFormula (10241) returns
        // "= " + the formula text, with a literal space -- preserved, not assumed away.

        auto resolveRange = QStringLiteral(R"js(
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var range = ws.GetRange(scope.range);
                    if (!range) throw new Error("could not resolve range " + scope.range);
        )js");

        table.Register(QStringLiteral("cell.setValue"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!scope.contains(QStringLiteral("value")) ||
                    !(scope.value(QStringLiteral("value")).isString() ||
                      scope.value(QStringLiteral("value")).isDouble() ||
                      scope.value(QStringLiteral("value")).isBool()))
                    return QStringLiteral("scope.value must be a string, number, or boolean");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    if (!range.SetValue(scope.value)) throw new Error("SetValue failed (protected sheet or invalid range)");
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.getValue"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    return range.GetValue();
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.getFormula"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    return range.GetFormula();
                })(%%SCOPE%%);
            )js")
        });
    }
}
