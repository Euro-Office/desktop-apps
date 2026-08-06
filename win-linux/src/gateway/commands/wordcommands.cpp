#include "wordcommands.h"
#include "../allowlist.h"

// Word command family, implemented in the sequential order set by
// cdp-gateway-cli-plan.md §4 ("Word (first)"). Each command's test cases live in
// gateway-test-case-designs.md under the matching §B heading (this file: §B1).
//
// Script bodies were written against sdkjs/word/apiBuilder.js as it actually reads
// (ApiCore.SetTitle/GetTitle, ApiCustomProperties.Add/Get) — not guessed. Every
// %%SCOPE%% is the sole substitution point, filled by GatewayCommandRunner via
// QJsonDocument(scope).toJson(Compact), never per-field string interpolation.

namespace Gateway::Commands
{
    void RegisterWordCommands()
    {
        auto& table = AllowlistTable::Instance();

        // --- B1. Document properties ---

        table.Register(QStringLiteral("word.getTitle"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); }, // no scope fields required
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetCore().GetTitle();
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setTitle"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("title"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    Api.GetDocument().GetCore().SetTitle(scope.title);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setCustomProperty"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
                if (!err.isEmpty())
                    return err;
                if (!scope.contains(QStringLiteral("value")) ||
                    !(scope.value(QStringLiteral("value")).isString() ||
                      scope.value(QStringLiteral("value")).isDouble() ||
                      scope.value(QStringLiteral("value")).isBool()))
                    return QStringLiteral("scope.value must be a string, number, or boolean");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ok = Api.GetDocument().GetCustomProperties().Add(scope.name, scope.value);
                    if (!ok) throw new Error("unsupported custom property value type");
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getCustomProperty"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetCustomProperties().Get(scope.name);
                })(%%SCOPE%%);
            )js")
        });

        // --- B2. Content enumeration ---
        //
        // ApiDocument inherits ApiDocumentContent (apiBuilder.js:3112), so
        // GetAllParagraphs/GetAllTables/GetAllDrawingObjects/GetAllCharts are real,
        // inherited methods (apiBuilder.js:6193-6289). Each returns an array of Api*
        // object instances, which aren't themselves JSON-serializable over CDP's
        // returnByValue -- returned as an array of 0-based indices instead, matching
        // gateway-test-case-designs.md §B2's corrected expectation and the same index
        // space paraIndex/tableIndex/etc. scope fields elsewhere already use.

        table.Register(QStringLiteral("word.getAllParagraphs"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetAllParagraphs().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getAllTables"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetAllTables().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getAllDrawingObjects"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetAllDrawingObjects().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getAllCharts"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetAllCharts().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });
    }
}
