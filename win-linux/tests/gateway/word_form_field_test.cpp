// Test cases from gateway-test-case-designs.md §B12 (Word fillable form fields).
// word.addCheckBoxForm is deliberately not implemented -- see that section's header
// note for why. See word_document_properties_test.cpp's header comment for
// scope/limits shared by every file in this directory.

#include "../../src/gateway/allowlist.h"
#include "../../src/gateway/commands/wordcommands.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <cstdio>
#include <vector>
#include <functional>
#include <string>

namespace
{
    bool Test_AddTextForm_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addTextForm"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("key"), QStringLiteral("name"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddTextForm_EmptyKey_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addTextForm"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("key"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetAllForms_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getAllForms"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_SetFormsData_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setFormsData"));
        if (!spec) return false;
        QJsonObject data;
        data.insert(QStringLiteral("name"), QStringLiteral("Alice"));
        QJsonObject scope;
        scope.insert(QStringLiteral("data"), data);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetFormsData_NonObjectData_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setFormsData"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("data"), QStringLiteral("not-an-object"));
        return !spec->validate(scope).isEmpty();
    }

    // B12.4: word.addCheckBoxForm is intentionally not registered.
    bool Test_AddCheckBoxForm_NotImplemented_NotAllowlisted()
    {
        return Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addCheckBoxForm")) == nullptr;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddTextForm_ValidScope_PassesValidation", Test_AddTextForm_ValidScope_PassesValidation},
        {"AddTextForm_EmptyKey_SchemaInvalid", Test_AddTextForm_EmptyKey_SchemaInvalid},
        {"GetAllForms_Registered_NoScopeRequired", Test_GetAllForms_Registered_NoScopeRequired},
        {"SetFormsData_ValidScope_PassesValidation", Test_SetFormsData_ValidScope_PassesValidation},
        {"SetFormsData_NonObjectData_SchemaInvalid", Test_SetFormsData_NonObjectData_SchemaInvalid},
        {"AddCheckBoxForm_NotImplemented_NotAllowlisted", Test_AddCheckBoxForm_NotImplemented_NotAllowlisted},
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
