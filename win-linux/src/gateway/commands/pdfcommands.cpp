#include "pdfcommands.h"
#include "../allowlist.h"

// PDF command family, implemented in the sequential order set by
// cdp-gateway-cli-plan.md §4 ("PDF (fourth)"). Each command's test cases live in
// gateway-test-case-designs.md under the matching §E heading (this file: §E1).
//
// Script bodies were written against sdkjs/pdf/apiBuilder.js as it actually reads --
// not guessed. Every %%SCOPE%% is the sole substitution point, filled by
// GatewayCommandRunner via QJsonDocument(scope).toJson(Compact), never per-field
// string interpolation.

namespace Gateway::Commands
{
    void RegisterPdfCommands()
    {
        auto& table = AllowlistTable::Instance();

        // --- E1. Form field read/write ---
        //
        // ApiDocument.GetAllFields() (apiBuilder.js:1426) returns ApiField[]
        // (ApiTextField/ApiCheckboxField/ApiComboboxField/...), not JSON-serializable
        // -- returned as field names via GetFullName() (1828), same established
        // pattern. GetValue/SetValue (1921,1902) are on the shared ApiBaseField base
        // class -- uniform across field types; SetValue stringifies its argument, so
        // a checkbox's checked state is a string export value ("Yes"/"Off"), not a
        // JSON boolean. GetFieldByName (1452) throws a plain TypeError for an unknown
        // name (calls .IsWidget() on whatever GetField returns with no null-check) --
        // that's why an unknown key surfaces as SCRIPT_EXCEPTION with no extra logic
        // needed here.

        table.Register(QStringLiteral("pdf.getAllFields"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetAllFields().map(function(f){ return f.GetFullName(); });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("pdf.getFieldValue"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("key"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var field = Api.GetDocument().GetFieldByName(scope.key);
                    if (!field) throw new Error("no field named " + scope.key);
                    return field.GetValue();
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("pdf.setFieldValue"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("key"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("value"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var field = Api.GetDocument().GetFieldByName(scope.key);
                    if (!field) throw new Error("no field named " + scope.key);
                    return field.SetValue(scope.value);
                })(%%SCOPE%%);
            )js")
        });
    }
}
