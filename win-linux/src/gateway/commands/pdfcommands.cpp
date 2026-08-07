#include "pdfcommands.h"
#include "../allowlist.h"

#include <QJsonArray>

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

        // --- E2. Annotations (stamp deferred) ---
        //
        // Api.CreateHighlightAnnot/CreateUnderlineAnnot/CreateStrikeoutAnnot/
        // CreateFreeTextAnnot/CreateInkAnnot (apiBuilder.js:887,1005,946,594,665)
        // create the annotation; ApiPage.AddObject (1605) attaches it. rect is a flat
        // [x1,y1,x2,y2] with x1<x2, y1<y2 (private_IsValidRect, 8129). CreateInkAnnot's
        // inkPaths needs an array of paths, each an array of {x,y} objects -- the
        // scope's `paths` field uses simpler [x,y] pairs, converted in script.
        // pdf.getAllAnnots (not in the original design) added as the read-back path
        // via ApiPage.GetAllAnnots() (1636) + GetClassType() per annotation, since
        // there was no allowlisted way to verify any of these commands' effects
        // otherwise. pdf.addStamp is deliberately NOT implemented -- AscPDF.STAMP_TYPES'
        // real enum values weren't located in this pass; see
        // gateway-test-case-designs.md §E2.

        auto resolvePage = QStringLiteral(R"js(
                    var page = Api.GetDocument().GetPage(scope.page);
                    if (!page) throw new Error("no page at index " + scope.page);
        )js");

        auto requireRect = [](const QJsonObject& scope) -> QString {
            const QJsonValue rectValue = scope.value(QStringLiteral("rect"));
            if (!rectValue.isArray() || rectValue.toArray().size() != 4)
                return QStringLiteral("scope.rect must be an array of 4 numbers [x1,y1,x2,y2]");
            for (const QJsonValue& v : rectValue.toArray())
                if (!v.isDouble())
                    return QStringLiteral("scope.rect must be an array of 4 numbers [x1,y1,x2,y2]");
            return QString();
        };

        table.Register(QStringLiteral("pdf.getAllAnnots"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("page"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolvePage + QStringLiteral(R"js(
                    return page.GetAllAnnots().map(function(a){ return a.GetClassType(); });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("pdf.addHighlight"), CommandSpec{
            [requireRect](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("page"));
                if (!err.isEmpty()) return err;
                return requireRect(scope);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolvePage + QStringLiteral(R"js(
                    var annot = Api.CreateHighlightAnnot(scope.rect);
                    page.AddObject(annot);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("pdf.addUnderline"), CommandSpec{
            [requireRect](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("page"));
                if (!err.isEmpty()) return err;
                return requireRect(scope);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolvePage + QStringLiteral(R"js(
                    var annot = Api.CreateUnderlineAnnot(scope.rect);
                    page.AddObject(annot);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("pdf.addStrikeout"), CommandSpec{
            [requireRect](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("page"));
                if (!err.isEmpty()) return err;
                return requireRect(scope);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolvePage + QStringLiteral(R"js(
                    var annot = Api.CreateStrikeoutAnnot(scope.rect);
                    page.AddObject(annot);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("pdf.addFreeText"), CommandSpec{
            [requireRect](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("page"));
                if (!err.isEmpty()) return err;
                err = requireRect(scope);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolvePage + QStringLiteral(R"js(
                    var annot = Api.CreateFreeTextAnnot(scope.rect);
                    annot.SetContents(scope.text);
                    page.AddObject(annot);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("pdf.addInk"), CommandSpec{
            [requireRect](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("page"));
                if (!err.isEmpty()) return err;
                err = requireRect(scope);
                if (!err.isEmpty()) return err;
                const QJsonValue pathsValue = scope.value(QStringLiteral("paths"));
                if (!pathsValue.isArray() || pathsValue.toArray().isEmpty())
                    return QStringLiteral("scope.paths must be a non-empty array of [x,y]-pair paths");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolvePage + QStringLiteral(R"js(
                    var inkPaths = scope.paths.map(function(path){
                        return path.map(function(pt){ return {x: pt[0], y: pt[1]}; });
                    });
                    var annot = Api.CreateInkAnnot(scope.rect, inkPaths);
                    page.AddObject(annot);
                    return true;
                })(%%SCOPE%%);
            )js")
        });
    }
}
