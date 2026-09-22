// Test cases from gateway-test-case-designs.md §E3 (PDF text search/extraction). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory: schema validation only, round trips deferred to §6.

#include "../../src/gateway/allowlist.h"
#include "../../src/gateway/commands/pdfcommands.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <cstdio>
#include <vector>
#include <functional>
#include <string>

namespace
{
    QJsonObject SamplePoint(int x, int y)
    {
        QJsonObject point;
        point.insert(QStringLiteral("x"), x);
        point.insert(QStringLiteral("y"), y);
        return point;
    }

    bool Test_SearchText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.searchText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("Total"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SearchText_OptionalFlags_PassValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.searchText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("Total"));
        scope.insert(QStringLiteral("matchCase"), true);
        scope.insert(QStringLiteral("wholeWords"), false);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SearchText_EmptyText_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.searchText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("text"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SearchText_MatchCaseWrongType_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.searchText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("Total"));
        scope.insert(QStringLiteral("matchCase"), QStringLiteral("yes"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetSelection_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.setSelection"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("startPoint"), SamplePoint(10, 10));
        scope.insert(QStringLiteral("endPoint"), SamplePoint(50, 20));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetSelection_MissingEndPoint_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.setSelection"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("startPoint"), SamplePoint(10, 10));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetSelection_PointMissingY_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.setSelection"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        QJsonObject badPoint;
        badPoint.insert(QStringLiteral("x"), 10);
        scope.insert(QStringLiteral("startPoint"), badPoint);
        scope.insert(QStringLiteral("endPoint"), SamplePoint(50, 20));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetSelectedText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getSelectedText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetSelectedText_MissingPage_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getSelectedText"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_RecognizeContent_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.recognizeContent"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_RecognizeContent_MissingPage_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.recognizeContent"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterPdfCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SearchText_ValidScope_PassesValidation", Test_SearchText_ValidScope_PassesValidation},
        {"SearchText_OptionalFlags_PassValidation", Test_SearchText_OptionalFlags_PassValidation},
        {"SearchText_EmptyText_SchemaInvalid", Test_SearchText_EmptyText_SchemaInvalid},
        {"SearchText_MatchCaseWrongType_SchemaInvalid", Test_SearchText_MatchCaseWrongType_SchemaInvalid},
        {"SetSelection_ValidScope_PassesValidation", Test_SetSelection_ValidScope_PassesValidation},
        {"SetSelection_MissingEndPoint_SchemaInvalid", Test_SetSelection_MissingEndPoint_SchemaInvalid},
        {"SetSelection_PointMissingY_SchemaInvalid", Test_SetSelection_PointMissingY_SchemaInvalid},
        {"GetSelectedText_ValidScope_PassesValidation", Test_GetSelectedText_ValidScope_PassesValidation},
        {"GetSelectedText_MissingPage_SchemaInvalid", Test_GetSelectedText_MissingPage_SchemaInvalid},
        {"RecognizeContent_ValidScope_PassesValidation", Test_RecognizeContent_ValidScope_PassesValidation},
        {"RecognizeContent_MissingPage_SchemaInvalid", Test_RecognizeContent_MissingPage_SchemaInvalid},
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
