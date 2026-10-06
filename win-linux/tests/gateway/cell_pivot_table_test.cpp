// Test cases from gateway-test-case-designs.md §C9 (Cell PivotTable). See
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
    bool Test_AddPivotTable_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addPivotTable"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sourceSheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("sourceRange"), QStringLiteral("A1:B10"));
        scope.insert(QStringLiteral("pivotSheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("pivotRange"), QStringLiteral("D1"));
        scope.insert(QStringLiteral("name"), QStringLiteral("MyPivot"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddPivotTable_MissingName_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addPivotTable"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sourceSheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("sourceRange"), QStringLiteral("A1:B10"));
        scope.insert(QStringLiteral("pivotSheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("pivotRange"), QStringLiteral("D1"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddPivotDataField_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addPivotDataField"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("pivotName"), QStringLiteral("MyPivot"));
        scope.insert(QStringLiteral("field"), QStringLiteral("Sales"));
        scope.insert(QStringLiteral("func"), QStringLiteral("Sum"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetPivotFieldFunction_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setPivotFieldFunction"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("pivotName"), QStringLiteral("MyPivot"));
        scope.insert(QStringLiteral("field"), QStringLiteral("Sales"));
        scope.insert(QStringLiteral("func"), QStringLiteral("Average"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetPivotFieldFunction_MissingFunc_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setPivotFieldFunction"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("pivotName"), QStringLiteral("MyPivot"));
        scope.insert(QStringLiteral("field"), QStringLiteral("Sales"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddPivotTable_ValidScope_PassesValidation", Test_AddPivotTable_ValidScope_PassesValidation},
        {"AddPivotTable_MissingName_SchemaInvalid", Test_AddPivotTable_MissingName_SchemaInvalid},
        {"AddPivotDataField_ValidScope_PassesValidation", Test_AddPivotDataField_ValidScope_PassesValidation},
        {"SetPivotFieldFunction_ValidScope_PassesValidation", Test_SetPivotFieldFunction_ValidScope_PassesValidation},
        {"SetPivotFieldFunction_MissingFunc_SchemaInvalid", Test_SetPivotFieldFunction_MissingFunc_SchemaInvalid},
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
