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
    }
}
