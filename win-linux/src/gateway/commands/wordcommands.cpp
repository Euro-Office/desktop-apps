#include "wordcommands.h"
#include "../allowlist.h"

#include <QRegularExpression>

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

        // --- B3. Insert/edit text ---
        //
        // ApiDocumentContent.GetElement(nPos) (apiBuilder.js:6028) resolves a
        // paragraph by index; ApiParagraph.GetElement(nPos) (apiBuilder.js:10315)
        // resolves a run within it the same way. ApiParagraph.AddText(text)
        // (apiBuilder.js:10146) always appends a *new* run carrying `text` -- it does
        // not edit an existing run in place -- and ApiRun.GetText (apiBuilder.js:12883)
        // reads one run's text back.

        table.Register(QStringLiteral("word.addText"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    para.AddText(scope.text);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getText"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("runIndex"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    var run = para.GetElement(scope.runIndex);
                    if (!run) throw new Error("no run at runIndex " + scope.runIndex);
                    return run.GetText();
                })(%%SCOPE%%);
            )js")
        });

        // --- B4. Character formatting ---
        //
        // All four are ApiRun methods (apiBuilder.js:12457,12550,12614; SetColor at
        // 12507 forwards to ApiTextPr.SetColor at 15918, whose non-ApiColor branch
        // still accepts plain (r,g,b) ints -- used here instead of constructing an
        // ApiColor object, since the scope's hex-string shape already has to be
        // decomposed into r/g/b for that call regardless.

        auto resolveRun = QStringLiteral(R"js(
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    var run = para.GetElement(scope.runIndex);
                    if (!run) throw new Error("no run at runIndex " + scope.runIndex);
        )js");

        table.Register(QStringLiteral("word.setBold"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("runIndex"));
                if (!err.isEmpty()) return err;
                return RequireBool(scope, QStringLiteral("bold"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRun + QStringLiteral(R"js(
                    run.SetBold(scope.bold);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setItalic"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("runIndex"));
                if (!err.isEmpty()) return err;
                return RequireBool(scope, QStringLiteral("italic"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRun + QStringLiteral(R"js(
                    run.SetItalic(scope.italic);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setFontFamily"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("runIndex"));
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("font"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRun + QStringLiteral(R"js(
                    run.SetFontFamily(scope.font);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setColor"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("runIndex"));
                if (!err.isEmpty()) return err;
                static const QRegularExpression hexColor(QStringLiteral("^#[0-9A-Fa-f]{6}$"));
                const QJsonValue colorValue = scope.value(QStringLiteral("color"));
                if (!colorValue.isString() || !hexColor.match(colorValue.toString()).hasMatch())
                    return QStringLiteral("scope.color must be a #RRGGBB hex string");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveRun + QStringLiteral(R"js(
                    var hex = scope.color.replace('#', '');
                    var r = parseInt(hex.substring(0, 2), 16);
                    var g = parseInt(hex.substring(2, 4), 16);
                    var b = parseInt(hex.substring(4, 6), 16);
                    run.SetColor(r, g, b);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- B5. Paragraph formatting ---
        //
        // ApiParagraph.GetParaPr() (apiBuilder.js:10253) returns the ApiParaPr;
        // SetJc/SetSpacingBefore/SetIndLeft (16677, 16863, 16580) all take twips
        // (1/1440 inch) -- scope field named `twips`, not `points`, to match.

        static const QStringList validAlignments = {
            QStringLiteral("left"), QStringLiteral("right"),
            QStringLiteral("center"), QStringLiteral("both")
        };

        table.Register(QStringLiteral("word.setJc"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("align"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!validAlignments.contains(scope.value(QStringLiteral("align")).toString()))
                    return QStringLiteral("scope.align must be one of left, right, center, both");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    para.GetParaPr().SetJc(scope.align);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setSpacingBefore"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                if (!scope.contains(QStringLiteral("twips")) || !scope.value(QStringLiteral("twips")).isDouble())
                    return QStringLiteral("scope.twips must be an integer");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    para.GetParaPr().SetSpacingBefore(scope.twips);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setIndLeft"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                // Negative twips is valid (hanging indent) -- only type-check, no
                // minimum, unlike RequireInt's default floor of 0.
                if (!scope.contains(QStringLiteral("twips")) || !scope.value(QStringLiteral("twips")).isDouble())
                    return QStringLiteral("scope.twips must be an integer");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    para.GetParaPr().SetIndLeft(scope.twips);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- B6. Search & replace ---
        //
        // ApiDocument.Search (apiBuilder.js:8253) returns an array of ApiRange
        // objects, not JSON-serializable over returnByValue (same issue as §B2) --
        // returns the match count instead. ApiDocument.SearchAndReplace
        // (apiBuilder.js:7598) takes {searchString, replaceString, matchCase=true}.

        table.Register(QStringLiteral("word.search"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().Search(scope.text, false).length;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.searchAndReplace"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("find"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("replace"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().SearchAndReplace({
                        searchString: scope.find,
                        replaceString: scope.replace
                    });
                })(%%SCOPE%%);
            )js")
        });

        // --- B7. Table creation and editing ---
        //
        // tableIndex indexes into GetAllTables() (apiBuilder.js:6275), the same index
        // space word.getAllTables (§B2) already exposes -- not raw document-content
        // position, which would also count paragraphs. ApiTable.AddRow/AddColumn
        // (13755, 13811) take a *cell* to insert relative to, not a row/column number
        // directly -- resolved via ApiTable.GetCell(row, col) (13589) then inserted
        // after it (isBefore=false), which is what "insert after rowIndex/colIndex"
        // means here. MergeCells (13609) takes an array of ApiTableCell -- built from
        // the fromRow/fromCol..toRow/toCol rectangle. SetStyle (13666) takes an
        // ApiStyle object, not a style id string -- resolved via
        // ApiDocument.GetStyle(sStyleName) (apiBuilder.js:7091).

        auto resolveTable = QStringLiteral(R"js(
                    var table = Api.GetDocument().GetAllTables()[scope.tableIndex];
                    if (!table) throw new Error("no table at tableIndex " + scope.tableIndex);
        )js");

        table.Register(QStringLiteral("word.addRow"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("tableIndex"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("rowIndex"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveTable + QStringLiteral(R"js(
                    var cell = table.GetCell(scope.rowIndex, 0);
                    if (!cell) throw new Error("no row at rowIndex " + scope.rowIndex);
                    table.AddRow(cell, false);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.addColumn"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("tableIndex"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("colIndex"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveTable + QStringLiteral(R"js(
                    var cell = table.GetCell(0, scope.colIndex);
                    if (!cell) throw new Error("no column at colIndex " + scope.colIndex);
                    table.AddColumn(cell, false);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.mergeCells"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                for (const char* field : {"tableIndex", "fromRow", "fromCol", "toRow", "toCol"})
                {
                    const QString err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveTable + QStringLiteral(R"js(
                    var cells = [];
                    for (var r = scope.fromRow; r <= scope.toRow; r++) {
                        for (var c = scope.fromCol; c <= scope.toCol; c++) {
                            var cell = table.GetCell(r, c);
                            if (cell) cells.push(cell);
                        }
                    }
                    var merged = table.MergeCells(cells);
                    if (!merged) throw new Error("merge failed for the given range");
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setStyle"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("tableIndex"));
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("styleId"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveTable + QStringLiteral(R"js(
                    var style = Api.GetDocument().GetStyle(scope.styleId);
                    if (!style) throw new Error("no such style " + scope.styleId);
                    if (!table.SetStyle(style)) throw new Error("SetStyle failed");
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- B8. Style creation and application ---
        //
        // ApiDocument.CreateStyle/GetStyle (apiBuilder.js:7106,7091);
        // ApiStyle.SetTextPr (15424) requires an actual ApiTextPr, built via
        // Api.CreateTextPr() (27443). getStyle returns a boolean (ApiStyle.Style is
        // undefined when the named style doesn't exist) rather than the unserializable
        // ApiStyle handle -- same pattern as §B2/§B6.

        static const QStringList validStyleTypes = {
            QStringLiteral("paragraph"), QStringLiteral("table"),
            QStringLiteral("run"), QStringLiteral("numbering")
        };

        table.Register(QStringLiteral("word.createStyle"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("type"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!validStyleTypes.contains(scope.value(QStringLiteral("type")).toString()))
                    return QStringLiteral("scope.type must be one of paragraph, table, run, numbering");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    Api.GetDocument().CreateStyle(scope.name, scope.type);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getStyle"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var style = Api.GetDocument().GetStyle(scope.name);
                    return !!(style && style.Style);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setStyleTextPr"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("styleId"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireBool(scope, QStringLiteral("bold"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    var style = Api.GetDocument().GetStyle(scope.styleId);
                    if (!style || !style.Style) throw new Error("no such style " + scope.styleId);
                    var textPr = Api.CreateTextPr();
                    textPr.SetBold(scope.bold);
                    style.SetTextPr(textPr);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- B9. Insert images/shapes with positioning ---
        //
        // Api.CreateImage(imageSrc, width, height) (apiBuilder.js:4674) -- imageSrc is
        // a URL or base64 data URI, not a local file path (per the method's own doc
        // comment); width/height are EMU. ApiParagraph.AddDrawing (10486) inserts the
        // created ApiImage (extends ApiDrawing, 3665) into a paragraph.
        // SetWrappingStyle/SetHorPosition (18651, 18771) operate on a drawing resolved
        // the same way word.getAllDrawingObjects (§B2) indexes them.

        static const QStringList validWrappingStyles = {
            QStringLiteral("inline"), QStringLiteral("square"), QStringLiteral("tight"),
            QStringLiteral("through"), QStringLiteral("topAndBottom"),
            QStringLiteral("behind"), QStringLiteral("inFront")
        };
        static const QStringList validRelativeFromH = {
            QStringLiteral("character"), QStringLiteral("column"), QStringLiteral("leftMargin"),
            QStringLiteral("rightMargin"), QStringLiteral("margin"), QStringLiteral("page")
        };

        table.Register(QStringLiteral("word.createImage"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("imageSrc"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("width"), /*minimum=*/1);
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("height"), /*minimum=*/1);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    var image = Api.CreateImage(scope.imageSrc, scope.width, scope.height);
                    para.AddDrawing(image);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setWrappingStyle"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("drawingIndex"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("style"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!validWrappingStyles.contains(scope.value(QStringLiteral("style")).toString()))
                    return QStringLiteral("scope.style is not a recognized wrapping style");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var drawing = Api.GetDocument().GetAllDrawingObjects()[scope.drawingIndex];
                    if (!drawing) throw new Error("no drawing at drawingIndex " + scope.drawingIndex);
                    return drawing.SetWrappingStyle(scope.style);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setHorPosition"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("drawingIndex"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("distanceEmu"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("relativeTo"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!validRelativeFromH.contains(scope.value(QStringLiteral("relativeTo")).toString()))
                    return QStringLiteral("scope.relativeTo is not a recognized reference point");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var drawing = Api.GetDocument().GetAllDrawingObjects()[scope.drawingIndex];
                    if (!drawing) throw new Error("no drawing at drawingIndex " + scope.drawingIndex);
                    return drawing.SetHorPosition(scope.relativeTo, scope.distanceEmu, false);
                })(%%SCOPE%%);
            )js")
        });

        // --- B10. Headers/footers, page setup ---
        //
        // ApiDocument has no GetSection(index), only GetFinalSection()
        // (apiBuilder.js:7191) -- these operate on the document's one section, correct
        // for every fixture used across this file. ApiSection.GetHeader (13289)
        // returns an ApiDocumentContent with no direct text-insertion method;
        // populated via Api.CreateParagraph() (4575) + ApiParagraph.AddText (§B3) +
        // ApiDocumentContent.AddElement(0, paragraph) (6052). SetPageMargins/
        // SetPageSize (13181, 13141) take twips.

        static const QStringList validHeaderTypes = {
            QStringLiteral("title"), QStringLiteral("even"), QStringLiteral("default")
        };

        table.Register(QStringLiteral("word.setHeaderText"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireString(scope, QStringLiteral("type"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                if (!validHeaderTypes.contains(scope.value(QStringLiteral("type")).toString()))
                    return QStringLiteral("scope.type must be one of title, even, default");
                return RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var section = Api.GetDocument().GetFinalSection();
                    var header = section.GetHeader(scope.type, true);
                    if (!header) throw new Error("could not get/create header of type " + scope.type);
                    var para = Api.CreateParagraph();
                    para.AddText(scope.text);
                    header.AddElement(0, para);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setPageMargins"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                for (const char* field : {"left", "top", "right", "bottom"})
                {
                    const QString err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetFinalSection().SetPageMargins(scope.left, scope.top, scope.right, scope.bottom);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setPageSize"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("width"), /*minimum=*/1);
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("height"), /*minimum=*/1);
            },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetFinalSection().SetPageSize(scope.width, scope.height, true);
                })(%%SCOPE%%);
            )js")
        });

        // --- B11. Bookmarks and hyperlinks ---
        //
        // Bookmark creation is only on ApiRange (apiBuilder.js:1581), not
        // ApiParagraph directly -- resolved via ApiParagraph.GetRange(0,0) (10604).
        // ApiParagraph.AddHyperlink(sLink, sScreenTipText, sBookmarkName)
        // (apiBuilder.js:10578) takes no display-text parameter -- it wraps whatever
        // text is already in the paragraph (via an internal SelectAll), so the text is
        // added first via AddText (§B3), then wrapped. getBookmark returns a boolean,
        // matching the unserializable-handle pattern already used in §B6/§B8. The URL
        // scheme allowlist below is defense-in-depth at the gateway's own schema layer,
        // independent of whatever scheme handling AddHyperlink does internally.

        static const QRegularExpression allowedUrlScheme(
            QStringLiteral("^(https?://|mailto:)"), QRegularExpression::CaseInsensitiveOption);

        table.Register(QStringLiteral("word.addBookmark"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    var range = para.GetRange(0, 0);
                    if (!range) throw new Error("could not get a range for paraIndex " + scope.paraIndex);
                    range.AddBookmark(scope.name);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getBookmark"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireString(scope, QStringLiteral("name"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    return !!Api.GetDocument().GetBookmark(scope.name);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.addHyperlink"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                const QJsonValue urlValue = scope.value(QStringLiteral("url"));
                if (!urlValue.isString() || !allowedUrlScheme.match(urlValue.toString()).hasMatch())
                    return QStringLiteral("scope.url must start with http://, https://, or mailto:");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    para.AddText(scope.text);
                    var link = para.AddHyperlink(scope.url, "", null);
                    if (!link) throw new Error("AddHyperlink rejected the given url/text");
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- B12. Fillable form fields / content controls ---
        //
        // ApiDocument.InsertTextForm (sdkjs-forms/apiBuilder.js:452) inserts at the
        // current cursor/selection -- positioned via ApiRange.Select()
        // (sdkjs/word/apiBuilder.js:1737) on paragraph.GetRange(0,0) first.
        // GetAllForms/SetFormsData confirmed (apiBuilder.js:8613,7963); SetFormsData
        // takes Array<{key,value}>. word.addCheckBoxForm is deliberately NOT
        // implemented here -- no confirmed insertion method exists in the vendored
        // source for a created ApiCheckBoxForm; see gateway-test-case-designs.md §B12
        // for the full reasoning. Do not add it without finding/confirming a real
        // insertion path first.

        table.Register(QStringLiteral("word.addTextForm"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("paraIndex"));
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("key"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var para = Api.GetDocument().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    var range = para.GetRange(0, 0);
                    if (!range) throw new Error("could not get a range for paraIndex " + scope.paraIndex);
                    range.Select();
                    var form = Api.GetDocument().InsertTextForm({key: scope.key});
                    if (!form) throw new Error("InsertTextForm failed");
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.getAllForms"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetDocument().GetAllForms().map(function(form){ return form.GetFormKey(); });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("word.setFormsData"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                if (!scope.contains(QStringLiteral("data")) || !scope.value(QStringLiteral("data")).isObject())
                    return QStringLiteral("scope.data must be an object of {key: value}");
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    var arrData = Object.keys(scope.data).map(function(key){
                        return {key: key, value: scope.data[key]};
                    });
                    return Api.GetDocument().SetFormsData(arrData);
                })(%%SCOPE%%);
            )js")
        });
    }
}
