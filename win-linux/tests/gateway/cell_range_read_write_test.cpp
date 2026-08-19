// Test cases from gateway-test-case-designs.md §C2 (Cell/range read & write). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory.

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
    bool Test_SetValue_NumericValue_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("value"), 42);
        return spec->validate(scope).isEmpty();
    }

    // C2.2: a formula is just a string value starting with "=" -- no separate field.
    bool Test_SetValue_FormulaString_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("value"), QStringLiteral("=1+1"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetValue_MissingValue_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetValue_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getValue"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetFormula_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getFormula"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetFormula_MissingRange_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getFormula"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetValue_NumericValue_PassesValidation", Test_SetValue_NumericValue_PassesValidation},
        {"SetValue_FormulaString_PassesValidation", Test_SetValue_FormulaString_PassesValidation},
        {"SetValue_MissingValue_SchemaInvalid", Test_SetValue_MissingValue_SchemaInvalid},
        {"GetValue_ValidScope_PassesValidation", Test_GetValue_ValidScope_PassesValidation},
        {"GetFormula_ValidScope_PassesValidation", Test_GetFormula_ValidScope_PassesValidation},
        {"GetFormula_MissingRange_SchemaInvalid", Test_GetFormula_MissingRange_SchemaInvalid},
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
