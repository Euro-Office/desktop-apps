// Test cases from gateway-test-case-designs.md §E2 (PDF annotations). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory: schema validation only, round trips deferred to §6.
// pdf.addStamp is intentionally absent -- deferred, not implemented (see
// pdfcommands.cpp's §E2 header comment).

#include "../../src/gateway/allowlist.h"
#include "../../src/gateway/commands/pdfcommands.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <cstdio>
#include <vector>
#include <functional>
#include <string>

namespace
{
    QJsonArray SampleRect()
    {
        return QJsonArray{0, 0, 100, 50};
    }

    bool Test_GetAllAnnots_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getAllAnnots"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetAllAnnots_MissingPage_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getAllAnnots"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_AddHighlight_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addHighlight"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddHighlight_MissingRect_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addHighlight"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddHighlight_RectWrongSize_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addHighlight"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), QJsonArray{0, 0, 100});
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddUnderline_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addUnderline"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddStrikeout_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addStrikeout"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddFreeText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addFreeText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        scope.insert(QStringLiteral("text"), QStringLiteral("hello"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddFreeText_MissingText_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addFreeText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddInk_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addInk"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        QJsonArray path{QJsonArray{10, 10}, QJsonArray{20, 20}, QJsonArray{30, 10}};
        scope.insert(QStringLiteral("paths"), QJsonArray{path});
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddInk_MissingPaths_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addInk"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddInk_EmptyPaths_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addInk"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), SampleRect());
        scope.insert(QStringLiteral("paths"), QJsonArray{});
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddStamp_NotRegistered()
    {
        // Deliberately unimplemented -- see pdfcommands.cpp §E2 header comment.
        return Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addStamp")) == nullptr;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterPdfCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"GetAllAnnots_ValidScope_PassesValidation", Test_GetAllAnnots_ValidScope_PassesValidation},
        {"GetAllAnnots_MissingPage_SchemaInvalid", Test_GetAllAnnots_MissingPage_SchemaInvalid},
        {"AddHighlight_ValidScope_PassesValidation", Test_AddHighlight_ValidScope_PassesValidation},
        {"AddHighlight_MissingRect_SchemaInvalid", Test_AddHighlight_MissingRect_SchemaInvalid},
        {"AddHighlight_RectWrongSize_SchemaInvalid", Test_AddHighlight_RectWrongSize_SchemaInvalid},
        {"AddUnderline_ValidScope_PassesValidation", Test_AddUnderline_ValidScope_PassesValidation},
        {"AddStrikeout_ValidScope_PassesValidation", Test_AddStrikeout_ValidScope_PassesValidation},
        {"AddFreeText_ValidScope_PassesValidation", Test_AddFreeText_ValidScope_PassesValidation},
        {"AddFreeText_MissingText_SchemaInvalid", Test_AddFreeText_MissingText_SchemaInvalid},
        {"AddInk_ValidScope_PassesValidation", Test_AddInk_ValidScope_PassesValidation},
        {"AddInk_MissingPaths_SchemaInvalid", Test_AddInk_MissingPaths_SchemaInvalid},
        {"AddInk_EmptyPaths_SchemaInvalid", Test_AddInk_EmptyPaths_SchemaInvalid},
        {"AddStamp_NotRegistered", Test_AddStamp_NotRegistered},
    };

    int failures = 0;
    for (const auto& test : tests)
    {
        const bool passed = test.second();
        std::printf("[%s] %s\n", passed ? "PASS" : "FAIL", test.first.c_str());
        if (!passed) ++failures;
    }

    std::printf("%zu tests, %d failed\n", tests.size(), failures);
    return failures == 0 ? 0 : 1;
}
