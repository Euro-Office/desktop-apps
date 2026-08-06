#include "cellcommands.h"
#include "../allowlist.h"

#include <QRegularExpression>
#include <vector>

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

        // --- C3. Number formats, merge, clear ---
        //
        // SetNumberFormat/Merge/ClearContents (apiBuilder.js:10828,10897,9759) all
        // return null/undefined on success -- no boolean result to surface.

        table.Register(QStringLiteral("cell.setNumberFormat"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("format"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    range.SetNumberFormat(scope.format);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.merge"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireBool(scope, QStringLiteral("across"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    range.Merge(scope.across);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.clearContents"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    range.ClearContents();
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- C4. Copy/paste, find/replace ---
        //
        // ApiRange.Copy(destination) (apiBuilder.js:11338) needs a real ApiRange
        // destination, not a string. Find/Replace (11616, 11764) are per-range
        // methods, not document-wide search -- Find returns a single ApiRange | null
        // (first match), not a list; both are called here against
        // ApiWorksheet.GetUsedRange() (8524) as the search scope. Results reported as
        // an address string (ApiRange.GetAddress(), 10043) or null, not the
        // unserializable ApiRange handle.

        table.Register(QStringLiteral("cell.copy"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("from"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("to"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var src = ws.GetRange(scope.from);
                    var dst = ws.GetRange(scope.to);
                    if (!src || !dst) throw new Error("could not resolve from/to range");
                    src.Copy(dst);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.find"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var found = ws.GetUsedRange().Find({What: scope.text});
                    return found ? found.GetAddress() : null;
                })(%%SCOPE%%);
            )js")
        });

        // --- C5. Font/fill/border/alignment formatting ---
        //
        // SetFontName (apiBuilder.js:10513) takes a plain string. SetFillColor (10772)
        // and SetBorders (§C3's investigation, same file) both need a real ApiColor,
        // built via Api.CreateColorFromRGB (925) from the scope's #RRGGBB hex, same
        // decomposition as word.setColor (§B4). SetBorders has no "all" edge mode --
        // the script loops over the four outer edges itself. SetAlignHorizontal (10575)
        // returns false (not a throw) for an unrecognized value; the script throws in
        // that case to keep this gateway's own error contract consistent.

        table.Register(QStringLiteral("cell.setFontName"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("font"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    range.SetFontName(scope.font);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.setFillColor"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                static const QRegularExpression hexColor(QStringLiteral("^#[0-9A-Fa-f]{6}$"));
                const QJsonValue colorValue = scope.value(QStringLiteral("color"));
                if (!colorValue.isString() || !hexColor.match(colorValue.toString()).hasMatch())
                    return QStringLiteral("scope.color must be a #RRGGBB hex string");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    var hex = scope.color.replace('#', '');
                    var r = parseInt(hex.substring(0, 2), 16);
                    var g = parseInt(hex.substring(2, 4), 16);
                    var b = parseInt(hex.substring(4, 6), 16);
                    range.SetFillColor(Api.CreateColorFromRGB(r, g, b));
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.setBorders"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                static const QStringList validEdges = {
                    QStringLiteral("all"), QStringLiteral("DiagonalDown"), QStringLiteral("DiagonalUp"),
                    QStringLiteral("Bottom"), QStringLiteral("Left"), QStringLiteral("Right"), QStringLiteral("Top"),
                    QStringLiteral("InsideHorizontal"), QStringLiteral("InsideVertical")
                };
                err = RequireString(scope, QStringLiteral("edge"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!validEdges.contains(scope.value(QStringLiteral("edge")).toString()))
                    return QStringLiteral("scope.edge is not a recognized border edge");
                err = RequireString(scope, QStringLiteral("style"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                static const QRegularExpression hexColor(QStringLiteral("^#[0-9A-Fa-f]{6}$"));
                const QJsonValue colorValue = scope.value(QStringLiteral("color"));
                if (!colorValue.isString() || !hexColor.match(colorValue.toString()).hasMatch())
                    return QStringLiteral("scope.color must be a #RRGGBB hex string");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    var hex = scope.color.replace('#', '');
                    var r = parseInt(hex.substring(0, 2), 16);
                    var g = parseInt(hex.substring(2, 4), 16);
                    var b = parseInt(hex.substring(4, 6), 16);
                    var color = Api.CreateColorFromRGB(r, g, b);
                    var edges = (scope.edge === 'all') ? ['Top', 'Bottom', 'Left', 'Right'] : [scope.edge];
                    for (var i = 0; i < edges.length; i++) {
                        range.SetBorders(edges[i], scope.style, color);
                    }
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.setAlignHorizontal"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                static const QStringList validAligns = {
                    QStringLiteral("left"), QStringLiteral("right"),
                    QStringLiteral("center"), QStringLiteral("justify")
                };
                err = RequireString(scope, QStringLiteral("align"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!validAligns.contains(scope.value(QStringLiteral("align")).toString()))
                    return QStringLiteral("scope.align must be one of left, right, center, justify");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    if (!range.SetAlignHorizontal(scope.align)) throw new Error("SetAlignHorizontal rejected the given alignment");
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- C6. Conditional formatting ---
        //
        // ApiRange.GetFormatConditions() (apiBuilder.js:12827) returns the
        // ApiFormatConditions collection; Add* methods (AddColorScale, AddDatabar,
        // AddIconSetCondition, 21119/21229/21299) return the created rule or null,
        // not JSON-serializable -- surfaced as a boolean. AddIconSetCondition takes no
        // parameters -- the originally-planned iconSet scope field dropped rather than
        // guessed at (see gateway-test-case-designs.md §C6).

        table.Register(QStringLiteral("cell.addColorScale"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("scaleType"), /*minimum=*/2);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    return !!range.GetFormatConditions().AddColorScale(scope.scaleType);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.addDatabar"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    return !!range.GetFormatConditions().AddDatabar();
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.addIconSetCondition"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    return !!range.GetFormatConditions().AddIconSetCondition();
                })(%%SCOPE%%);
            )js")
        });

        // --- C7. Data validation and named ranges ---
        //
        // ApiRange.GetValidation().Add(...) (apiBuilder.js:12793, 19898) takes the
        // real internal enum strings (FromXlValidationTypeTo/FromXlValidationOperatorTo,
        // 19653/19747), e.g. "xlValidateWholeNumber"/"xlBetween" -- accepted verbatim
        // in scope rather than inventing a translation layer. ApiWorksheet.AddDefName
        // (8974) returns false (not a throw) for an invalid name/ref -- converted to a
        // thrown error here to keep this gateway's own error contract consistent.

        table.Register(QStringLiteral("cell.addValidation"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("type"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("operator"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("formula1"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("formula2"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    var v = range.GetValidation().Add(scope.type, undefined, scope.operator, scope.formula1, scope.formula2 || undefined);
                    if (!v) throw new Error("Add validation failed (unrecognized type/operator, or validation already exists)");
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.addDefName"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("refersTo"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var refersTo = scope.refersTo;
                    var sheetName = refersTo.split('!')[0];
                    var ws = Api.GetSheet(sheetName);
                    if (!ws) throw new Error("no sheet named " + sheetName);
                    if (!ws.AddDefName(scope.name, refersTo, false))
                        throw new Error("AddDefName rejected the given name/refersTo");
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        // --- C8. AutoFilter ---
        //
        // ApiAutoFilter.ApplyFilter() (apiBuilder.js:27379) only re-evaluates an
        // *existing* AutoFilter's criteria -- it does not create one. Establishing a
        // new AutoFilter range is ApiRange.SetAutoFilter() with no arguments (12216),
        // which toggles: creates one if none exists, deletes the existing one if
        // called again. GetFilters() (27421) returns unserializable ApiFilter[] --
        // cell.getFilters returns GetFilterMode()'s boolean instead.

        table.Register(QStringLiteral("cell.applyFilter"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    range.SetAutoFilter();
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.getFilters"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    return ws.GetAutoFilter().GetFilterMode();
                })(%%SCOPE%%);
            )js")
        });

        // --- C9. PivotTable ---
        //
        // Three real, distinct steps (apiBuilder.js:7676,16192,17582): create via
        // Api.InsertPivotExistingWorksheet(dataRef, pivotRef, confirmation) (real
        // ApiRange objects, not strings), name it (SetName, 16782) so later commands
        // can re-resolve it via ApiWorksheet.GetPivotByName (9412) -- there's no
        // addressing by source range. AddDataField (16192) returns an
        // ApiPivotDataField; ApiPivotField.SetFunction (the field type AddFields
        // works with) is a hardcoded-error stub -- the real setter is
        // ApiPivotDataField.SetFunction (17582), re-resolved via GetDataFields
        // (16633) for a later, separate call. private_MakeError really throws
        // (throwException), so AddDataField on an unknown field already propagates
        // as SCRIPT_EXCEPTION with no extra null-check needed.

        table.Register(QStringLiteral("cell.addPivotTable"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                for (const char* field : {"sourceSheet", "sourceRange", "pivotSheet", "pivotRange", "name"})
                {
                    const QString err = RequireString(scope, QString::fromLatin1(field), /*allowEmpty=*/false);
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var srcWs = Api.GetSheet(scope.sourceSheet);
                    if (!srcWs) throw new Error("no sheet named " + scope.sourceSheet);
                    var pivotWs = Api.GetSheet(scope.pivotSheet);
                    if (!pivotWs) throw new Error("no sheet named " + scope.pivotSheet);
                    var dataRef = srcWs.GetRange(scope.sourceRange);
                    var pivotRef = pivotWs.GetRange(scope.pivotRange);
                    if (!dataRef || !pivotRef) throw new Error("could not resolve source/pivot range");
                    var table = Api.InsertPivotExistingWorksheet(dataRef, pivotRef, true);
                    if (!table) throw new Error("InsertPivotExistingWorksheet failed");
                    table.SetName(scope.name);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.addPivotDataField"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("pivotName"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("field"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("func"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var table = ws.GetPivotByName(scope.pivotName);
                    if (!table) throw new Error("no pivot table named " + scope.pivotName);
                    var dataField = table.AddDataField(scope.field);
                    dataField.SetFunction(scope.func);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.setPivotFieldFunction"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("pivotName"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("field"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("func"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var table = ws.GetPivotByName(scope.pivotName);
                    if (!table) throw new Error("no pivot table named " + scope.pivotName);
                    var dataField = table.GetDataFields(scope.field);
                    if (!dataField) throw new Error("no data field " + scope.field);
                    dataField.SetFunction(scope.func);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        // --- C10. Freeze panes ---
        //
        // ApiWorksheet.GetFreezePanes().FreezeAt(range) (apiBuilder.js:9474,15780) --
        // range resolved explicitly via ws.GetRange() first rather than relying on
        // FreezeAt's own string-overload, which resolves against the *active* sheet,
        // not necessarily the sheet this command's `sheet` scope field names.

        table.Register(QStringLiteral("cell.freezeAt"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("range"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRange + QStringLiteral(R"js(
                    ws.GetFreezePanes().FreezeAt(range);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- C11. Insert images/OLE objects (shapes deferred) ---
        //
        // ApiWorksheet.AddImage/AddOleObject (apiBuilder.js:9167,9228) place objects
        // by column/row + EMU offset, not a range; sImageSrc is a URL/base64 data URI
        // (same as word.createImage, §B9), not a local file path. cell.addShape is
        // deliberately NOT implemented -- AddShape needs real ApiFill/ApiStroke
        // objects whose constructors weren't confirmed in this file this pass; see
        // gateway-test-case-designs.md §C11.

        auto imagePlacementFields = std::vector<const char*>{
            "fromCol", "colOffset", "fromRow", "rowOffset"
        };

        table.Register(QStringLiteral("cell.addImage"), CommandSpec{
            [imagePlacementFields](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("imageSrc"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("width"), /*minimum=*/1);
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("height"), /*minimum=*/1);
                if (!err.isEmpty()) return err;
                for (const char* field : imagePlacementFields)
                {
                    err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var image = ws.AddImage(scope.imageSrc, scope.width, scope.height,
                        scope.fromCol, scope.colOffset, scope.fromRow, scope.rowOffset);
                    return !!image;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.addOleObject"), CommandSpec{
            [imagePlacementFields](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("imageSrc"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("width"), /*minimum=*/1);
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("height"), /*minimum=*/1);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("data"), /*allowEmpty=*/true);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("appId"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                for (const char* field : imagePlacementFields)
                {
                    err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var ole = ws.AddOleObject(scope.imageSrc, scope.width, scope.height,
                        scope.data, scope.appId, scope.fromCol, scope.colOffset, scope.fromRow, scope.rowOffset);
                    return !!ole;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("cell.replace"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("sheet"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("find"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("replace"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var ws = Api.GetSheet(scope.sheet);
                    if (!ws) throw new Error("no sheet named " + scope.sheet);
                    var found = ws.GetUsedRange().Replace({What: scope.find, Replacement: scope.replace, ReplaceAll: true});
                    return found ? found.GetAddress() : null;
                })(%%SCOPE%%);
            )js")
        });
    }
}
