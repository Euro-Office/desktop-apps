// Test cases from gateway-test-case-designs.md §C6 (Cell conditional formatting). See
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
    bool Test_AddColorScale_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addColorScale"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1:A10"));
        scope.insert(QStringLiteral("scaleType"), 3);
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddColorScale_ScaleTypeTooLow_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addColorScale"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1:A10"));
        scope.insert(QStringLiteral("scaleType"), 1);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddDatabar_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addDatabar"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1:A10"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddIconSetCondition_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addIconSetCondition"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1:A10"));
        return spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddColorScale_ValidScope_PassesValidation", Test_AddColorScale_ValidScope_PassesValidation},
        {"AddColorScale_ScaleTypeTooLow_SchemaInvalid", Test_AddColorScale_ScaleTypeTooLow_SchemaInvalid},
        {"AddDatabar_ValidScope_PassesValidation", Test_AddDatabar_ValidScope_PassesValidation},
        {"AddIconSetCondition_ValidScope_PassesValidation", Test_AddIconSetCondition_ValidScope_PassesValidation},
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
