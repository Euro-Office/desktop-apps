// Test cases from gateway-test-case-designs.md §C1 (Cell sheet management). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory: schema validation only, round trips deferred to §6.

#include "../../src/gateway/allowlist.h"
#include "../../src/gateway/commands/cellcommands.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <cstdio>
#include <vector>
#include <functional>
#include <string>

namespace
{
    bool Test_AddSheet_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addSheet"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("Data"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddSheet_EmptyName_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addSheet"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetSheets_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getSheets"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_SetActiveSheet_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setActiveSheet"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("Data"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetVisible_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setVisible"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("visible"), false);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetVisible_MissingVisible_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setVisible"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("Sheet1"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetName_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setName"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("oldName"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("newName"), QStringLiteral("Renamed"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetName_MissingNewName_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setName"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("oldName"), QStringLiteral("Sheet1"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddSheet_ValidScope_PassesValidation", Test_AddSheet_ValidScope_PassesValidation},
        {"AddSheet_EmptyName_SchemaInvalid", Test_AddSheet_EmptyName_SchemaInvalid},
        {"GetSheets_Registered_NoScopeRequired", Test_GetSheets_Registered_NoScopeRequired},
        {"SetActiveSheet_ValidScope_PassesValidation", Test_SetActiveSheet_ValidScope_PassesValidation},
        {"SetVisible_ValidScope_PassesValidation", Test_SetVisible_ValidScope_PassesValidation},
        {"SetVisible_MissingVisible_SchemaInvalid", Test_SetVisible_MissingVisible_SchemaInvalid},
        {"SetName_ValidScope_PassesValidation", Test_SetName_ValidScope_PassesValidation},
        {"SetName_MissingNewName_SchemaInvalid", Test_SetName_MissingNewName_SchemaInvalid},
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
