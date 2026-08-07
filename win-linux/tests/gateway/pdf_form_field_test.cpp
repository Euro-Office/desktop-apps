// Test cases from gateway-test-case-designs.md §E1 (PDF form field read/write). See
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
    bool Test_GetAllFields_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getAllFields"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_GetFieldValue_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getFieldValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("key"), QStringLiteral("Name"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetFieldValue_EmptyKey_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.getFieldValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("key"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetFieldValue_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.setFieldValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("key"), QStringLiteral("Name"));
        scope.insert(QStringLiteral("value"), QStringLiteral("Alice"));
        return spec->validate(scope).isEmpty();
    }

    // E1.3: checkbox "checked" state is a string export value, not a JSON boolean.
    bool Test_SetFieldValue_CheckboxStringValue_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.setFieldValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("key"), QStringLiteral("Agree"));
        scope.insert(QStringLiteral("value"), QStringLiteral("Yes"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetFieldValue_MissingValue_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("pdf.setFieldValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("key"), QStringLiteral("Name"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterPdfCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"GetAllFields_Registered_NoScopeRequired", Test_GetAllFields_Registered_NoScopeRequired},
        {"GetFieldValue_ValidScope_PassesValidation", Test_GetFieldValue_ValidScope_PassesValidation},
        {"GetFieldValue_EmptyKey_SchemaInvalid", Test_GetFieldValue_EmptyKey_SchemaInvalid},
        {"SetFieldValue_ValidScope_PassesValidation", Test_SetFieldValue_ValidScope_PassesValidation},
        {"SetFieldValue_CheckboxStringValue_PassesValidation", Test_SetFieldValue_CheckboxStringValue_PassesValidation},
        {"SetFieldValue_MissingValue_SchemaInvalid", Test_SetFieldValue_MissingValue_SchemaInvalid},
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
