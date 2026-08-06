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
    }
}
