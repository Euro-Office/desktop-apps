#include "slidecommands.h"
#include "../allowlist.h"

// Slide command family, implemented in the sequential order set by
// cdp-gateway-cli-plan.md §4 ("Slide (third)"). Each command's test cases live in
// gateway-test-case-designs.md under the matching §D heading (this file: §D1).
//
// Script bodies were written against sdkjs/slide/apiBuilder.js as it actually reads --
// not guessed. Every %%SCOPE%% is the sole substitution point, filled by
// GatewayCommandRunner via QJsonDocument(scope).toJson(Compact), never per-field
// string interpolation.

namespace Gateway::Commands
{
    void RegisterSlideCommands()
    {
        auto& table = AllowlistTable::Instance();

        // --- D1. Slide management ---
        //
        // AddSlide(oSlide, nIndex) (apiBuilder.js:1365) needs an actual ApiSlide via
        // Api.CreateSlide() (805), not a bare index -- an out-of-range nIndex is
        // silently treated as append-at-end, not an error. RemoveSlides(nStart,
        // nCount) (1564) takes a contiguous start+count range, not an arbitrary
        // indices array, and returns false (not a throw) for an out-of-range nStart --
        // converted to a thrown error here. Duplicate/MoveTo (3853,3872) are called on
        // a resolved ApiSlide (GetSlideByIndex, 1324), not ApiPresentation directly.

        auto resolveSlide = QStringLiteral(R"js(
                    var presentation = Api.GetPresentation();
                    var slide = presentation.GetSlideByIndex(scope.index);
                    if (!slide) throw new Error("no slide at index " + scope.index);
        )js");

        table.Register(QStringLiteral("slide.addSlide"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    var newSlide = Api.CreateSlide();
                    Api.GetPresentation().AddSlide(newSlide, scope.index);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.removeSlides"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("start"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("count"), /*minimum=*/1);
            },
            QStringLiteral(R"js(
                (function(scope){
                    var removed = Api.GetPresentation().RemoveSlides(scope.start, scope.count);
                    if (!removed) throw new Error("RemoveSlides failed (start/count out of range)");
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.duplicate"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    var duplicate = slide.Duplicate();
                    return !!duplicate;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.moveTo"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("newIndex"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return slide.MoveTo(scope.newIndex);
                })(%%SCOPE%%);
            )js")
        });

        // --- D2. Enumerate slide content ---
        //
        // ApiSlide.GetAllShapes/GetAllImages/GetAllTables/GetAllCharts
        // (apiBuilder.js:4140,4155,4197,4169) return Api*[], not JSON-serializable --
        // returned as index arrays, same established pattern as word.getAllTables (§B2).

        table.Register(QStringLiteral("slide.getAllShapes"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return slide.GetAllShapes().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.getAllImages"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return slide.GetAllImages().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.getAllTables"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return slide.GetAllTables().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.getAllCharts"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return slide.GetAllCharts().map(function(_, idx){ return idx; });
                })(%%SCOPE%%);
            )js")
        });

        // --- D3. Apply layouts, masters, themes (theme application deferred) ---
        //
        // GetLayout/ApplyLayout (apiBuilder.js:4092,3800) work with real ApiLayout
        // objects -- no id-string addressing exists. applyLayout borrows an
        // already-resolved layout from another slide rather than looking one up by a
        // fabricated layoutId. AddMaster (1522) needs a real ApiMaster from
        // Api.CreateMaster() (553), called with no theme argument to use its own
        // documented fallback. slide.applyTheme is deliberately NOT implemented --
        // Api.CreateTheme (640) needs three further factory-built scheme objects not
        // confirmed in this pass; see gateway-test-case-designs.md §D3.

        table.Register(QStringLiteral("slide.getLayout"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return !!slide.GetLayout();
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.applyLayout"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("fromIndex"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    var presentation = Api.GetPresentation();
                    var slide = presentation.GetSlideByIndex(scope.index);
                    if (!slide) throw new Error("no slide at index " + scope.index);
                    var fromSlide = presentation.GetSlideByIndex(scope.fromIndex);
                    if (!fromSlide) throw new Error("no slide at fromIndex " + scope.fromIndex);
                    var layout = fromSlide.GetLayout();
                    if (!layout) throw new Error("slide at fromIndex has no layout to borrow");
                    return slide.ApplyLayout(layout);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.addMaster"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("position"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    var master = Api.CreateMaster();
                    if (!master) throw new Error("CreateMaster failed (no theme available)");
                    return Api.GetPresentation().AddMaster(scope.position, master);
                })(%%SCOPE%%);
            )js")
        });

        // --- D4. Set transitions (background deferred) ---
        //
        // Api.CreateSlideShowTransition() (apiBuilder.js:1075, no-arg) configured via
        // SetEntryEffect/SetDuration (4848,4902), then ApiSlide.SetSlideShowTransition
        // (4389). slide.setBackground intentionally NOT implemented -- SetBackground
        // (3723) needs a real ApiFill; no solid-fill factory was confirmed in this
        // file. See gateway-test-case-designs.md §D4.

        // --- D5. Insert shapes/text boxes with positioning ---
        //
        // Api.CreateShape(sType, nWidth, nHeight) (apiBuilder.js:870) has real
        // internal defaults for fill/stroke -- fully implementable, unlike Cell's
        // AddShape (§C11). Positioned separately via AddObject (3621) +
        // ApiDrawing.SetPosition (6100), since CreateShape itself takes no position.
        // Existing shapes resolved via slide.GetAllShapes()[shapeIndex], same index
        // space as slide.getAllShapes (§D2).

        auto resolveShape = QStringLiteral(R"js(
                    var presentation = Api.GetPresentation();
                    var slide = presentation.GetSlideByIndex(scope.index);
                    if (!slide) throw new Error("no slide at index " + scope.index);
                    var shape = slide.GetAllShapes()[scope.shapeIndex];
                    if (!shape) throw new Error("no shape at shapeIndex " + scope.shapeIndex);
        )js");

        table.Register(QStringLiteral("slide.createShape"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("type"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                for (const char* field : {"x", "y", "width", "height"})
                {
                    err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    var shape = Api.CreateShape(scope.type, scope.width, scope.height);
                    var added = slide.AddObject(shape);
                    if (!added) throw new Error("AddObject failed");
                    shape.SetPosition(scope.x, scope.y);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.setPosition"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("shapeIndex"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("x"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("y"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveShape + QStringLiteral(R"js(
                    shape.SetPosition(scope.x, scope.y);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.setRotation"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("shapeIndex"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("degrees"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveShape + QStringLiteral(R"js(
                    return shape.SetRotation(scope.degrees);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.setSize"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("shapeIndex"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("width"), /*minimum=*/0);
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("height"), /*minimum=*/0);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveShape + QStringLiteral(R"js(
                    shape.SetSize(scope.width, scope.height);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- D6. Text formatting ---
        //
        // ApiRun.SetBold/SetFontFamily are shared classes with Word (confirmed absent
        // from this file, so they come from a common file included by all editors),
        // per the plan's own note. Resolved via ApiShape.GetContent() (6975) +
        // GetElement(paraIndex).GetElement(runIndex), same chain as word.setBold
        // (§B4) -- the originally-planned scope was missing paraIndex, added here.

        auto resolveSlideRun = QStringLiteral(R"js(
                    var presentation = Api.GetPresentation();
                    var slide = presentation.GetSlideByIndex(scope.index);
                    if (!slide) throw new Error("no slide at index " + scope.index);
                    var shape = slide.GetAllShapes()[scope.shapeIndex];
                    if (!shape) throw new Error("no shape at shapeIndex " + scope.shapeIndex);
                    var para = shape.GetContent().GetElement(scope.paraIndex);
                    if (!para) throw new Error("no paragraph at paraIndex " + scope.paraIndex);
                    var run = para.GetElement(scope.runIndex);
                    if (!run) throw new Error("no run at runIndex " + scope.runIndex);
        )js");

        table.Register(QStringLiteral("slide.setBold"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                for (const char* field : {"index", "shapeIndex", "paraIndex", "runIndex"})
                {
                    const QString err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return RequireBool(scope, QStringLiteral("bold"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlideRun + QStringLiteral(R"js(
                    run.SetBold(scope.bold);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.setFontFamily"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                for (const char* field : {"index", "shapeIndex", "paraIndex", "runIndex"})
                {
                    const QString err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return RequireString(scope, QStringLiteral("font"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlideRun + QStringLiteral(R"js(
                    run.SetFontFamily(scope.font);
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- D7. Insert images ---
        //
        // Api.CreateImage(sImageSrc, nWidth, nHeight) (apiBuilder.js:825) -- URL/base64
        // data URI, matching the Word/Cell precedent; same two-step
        // AddObject+SetPosition pattern as slide.createShape (§D5).

        table.Register(QStringLiteral("slide.createImage"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("imageSrc"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                for (const char* field : {"x", "y", "width", "height"})
                {
                    err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    var image = Api.CreateImage(scope.imageSrc, scope.width, scope.height);
                    var added = slide.AddObject(image);
                    if (!added) throw new Error("AddObject failed");
                    image.SetPosition(scope.x, scope.y);
                    return true;
                })(%%SCOPE%%);
            )js")
        });

        // --- D8. Table editing (creation deferred) ---
        //
        // Api.CreateTable (apiBuilder.js:947) places the table on whatever
        // private_GetCurrentSlide() resolves to -- no public setter exists to target
        // an arbitrary slide by index first, so slide.createTable is deliberately NOT
        // implemented. AddRow/MergeCells (7412, 7314) operate on an existing table,
        // addressed via slide.GetAllTables()[tableIndex] (§D2). Cell resolution via
        // GetRow(r).GetCell(c) (7295, 7614) -- no GetCell(row,col) shortcut exists here.

        auto resolveSlideTable = QStringLiteral(R"js(
                    var presentation = Api.GetPresentation();
                    var slide = presentation.GetSlideByIndex(scope.index);
                    if (!slide) throw new Error("no slide at index " + scope.index);
                    var table = slide.GetAllTables()[scope.tableIndex];
                    if (!table) throw new Error("no table at tableIndex " + scope.tableIndex);
        )js");

        table.Register(QStringLiteral("slide.addRow"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("tableIndex"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlideTable + QStringLiteral(R"js(
                    var row = table.AddRow();
                    return !!row;
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.mergeCells"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                for (const char* field : {"tableIndex", "fromRow", "fromCol", "toRow", "toCol"})
                {
                    err = RequireInt(scope, QString::fromLatin1(field));
                    if (!err.isEmpty()) return err;
                }
                return QString();
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlideTable + QStringLiteral(R"js(
                    var cells = [];
                    for (var r = scope.fromRow; r <= scope.toRow; r++) {
                        var row = table.GetRow(r);
                        if (!row) continue;
                        for (var c = scope.fromCol; c <= scope.toCol; c++) {
                            var cell = row.GetCell(c);
                            if (cell) cells.push(cell);
                        }
                    }
                    var merged = table.MergeCells(cells);
                    if (!merged) throw new Error("merge failed for the given range");
                    return null;
                })(%%SCOPE%%);
            )js")
        });

        // --- D9. Speaker notes ---
        //
        // ApiSlide.AddNotesText(sText) (apiBuilder.js:4331) calls ApiParagraph.AddText
        // internally -- same append-only semantics as word.addText (§B3). Read-back
        // (slide.getNotesText, not in the original design) via
        // GetNotesPage().GetBodyShape().GetDocContent().GetElement(0).GetText() --
        // without it there was no way to verify addNotesText's effect at all.

        table.Register(QStringLiteral("slide.addNotesText"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/false);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return slide.AddNotesText(scope.text);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.getNotesText"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                return RequireInt(scope, QStringLiteral("index"));
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    var notesPage = slide.GetNotesPage();
                    if (!notesPage) return "";
                    var bodyShape = notesPage.GetBodyShape();
                    if (!bodyShape) return "";
                    var docContent = bodyShape.GetDocContent();
                    if (!docContent) return "";
                    var para = docContent.GetElement(0);
                    if (!para) return "";
                    return para.GetText();
                })(%%SCOPE%%);
            )js")
        });

        // --- D10. Comments ---
        //
        // ApiSlide.AddComment(posX, posY, text, author, userId) (apiBuilder.js:3649)
        // takes an EMU position -- x/y added to the scope, missing from the original
        // design. ApiPresentation.GetAllComments() (1697) returns ApiComment[] via the
        // same shared class as Word (GetText/GetAuthorName, §B13) -- returned as plain
        // {text,author} objects, same pattern as word.getAllComments.

        table.Register(QStringLiteral("slide.addComment"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("x"));
                if (!err.isEmpty()) return err;
                err = RequireInt(scope, QStringLiteral("y"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("text"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireString(scope, QStringLiteral("author"), /*allowEmpty=*/true);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    return slide.AddComment(scope.x, scope.y, scope.text, scope.author);
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("presentation.getAllComments"), CommandSpec{
            [](const QJsonObject&) -> QString { return QString(); },
            QStringLiteral(R"js(
                (function(scope){
                    return Api.GetPresentation().GetAllComments().map(function(c){
                        return {text: c.GetText(), author: c.GetAuthorName()};
                    });
                })(%%SCOPE%%);
            )js")
        });

        table.Register(QStringLiteral("slide.setTransition"), CommandSpec{
            [](const QJsonObject& scope) -> QString {
                QString err = RequireInt(scope, QStringLiteral("index"));
                if (!err.isEmpty()) return err;
                err = RequireString(scope, QStringLiteral("entryEffect"), /*allowEmpty=*/false);
                if (!err.isEmpty()) return err;
                return RequireInt(scope, QStringLiteral("duration"), /*minimum=*/0);
            },
            QStringLiteral(R"js(
                (function(scope){
                    )js") + resolveSlide + QStringLiteral(R"js(
                    var transition = Api.CreateSlideShowTransition();
                    if (!transition.SetEntryEffect(scope.entryEffect))
                        throw new Error("unrecognized entryEffect: " + scope.entryEffect);
                    transition.SetDuration(scope.duration);
                    return slide.SetSlideShowTransition(transition);
                })(%%SCOPE%%);
            )js")
        });
    }
}
