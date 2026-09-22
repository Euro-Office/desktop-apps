// Test cases from gateway-test-case-designs.md §E4 (PDF redaction). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory: schema validation only, round trips deferred to §6.

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
    bool Test_AddRedact_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addRedact"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        scope.insert(QStringLiteral("rect"), QJsonArray{0, 0, 100, 20});
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddRedact_MissingRect_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.addRedact"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("page"), 0);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SearchAndRedact_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.searchAndRedact"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("text"), QStringLiteral("SSN:"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SearchAndRedact_EmptyText_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.searchAndRedact"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("text"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SearchAndRedact_NoPageRequired()
    {
        // Document-wide, unlike pdf.searchText -- no `page` field expected/required.
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.searchAndRedact"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("text"), QStringLiteral("SSN:"));
        scope.insert(QStringLiteral("wholeWords"), true);
        return spec->validate(scope).isEmpty();
    }

    bool Test_ApplyRedact_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.applyRedact"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterPdfCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddRedact_ValidScope_PassesValidation", Test_AddRedact_ValidScope_PassesValidation},
        {"AddRedact_MissingRect_SchemaInvalid", Test_AddRedact_MissingRect_SchemaInvalid},
        {"SearchAndRedact_ValidScope_PassesValidation", Test_SearchAndRedact_ValidScope_PassesValidation},
        {"SearchAndRedact_EmptyText_SchemaInvalid", Test_SearchAndRedact_EmptyText_SchemaInvalid},
        {"SearchAndRedact_NoPageRequired", Test_SearchAndRedact_NoPageRequired},
        {"ApplyRedact_NoScopeRequired", Test_ApplyRedact_NoScopeRequired},
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
