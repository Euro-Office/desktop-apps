// Test cases from gateway-test-case-designs.md §E5 (PDF page operations). See
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
    bool Test_GetPageCount_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getPageCount"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_AddPage_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addPage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 1);
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddPage_MissingIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addPage"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_RemovePage_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.removePage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_RemovePage_NegativeIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.removePage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), -1);
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterPdfCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"GetPageCount_Registered_NoScopeRequired", Test_GetPageCount_Registered_NoScopeRequired},
        {"AddPage_ValidScope_PassesValidation", Test_AddPage_ValidScope_PassesValidation},
        {"AddPage_MissingIndex_SchemaInvalid", Test_AddPage_MissingIndex_SchemaInvalid},
        {"RemovePage_ValidScope_PassesValidation", Test_RemovePage_ValidScope_PassesValidation},
        {"RemovePage_NegativeIndex_SchemaInvalid", Test_RemovePage_NegativeIndex_SchemaInvalid},
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
