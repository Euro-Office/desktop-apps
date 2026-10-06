// Test cases from gateway-test-case-designs.md §C15 (Cell create charts and edit data
// series). See word_document_properties_test.cpp's header comment for scope/limits
// shared by every file in this directory.

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
    bool Test_AddSeria_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addSeria"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("chartIndex"), 0);
        scope.insert(QStringLiteral("valuesRange"), QStringLiteral("Sheet1!B1:B10"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddSeria_MissingValuesRange_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addSeria"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("chartIndex"), 0);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetSeriaName_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setSeriaName"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("chartIndex"), 0);
        scope.insert(QStringLiteral("seriaIndex"), 0);
        scope.insert(QStringLiteral("name"), QStringLiteral("Revenue"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetSeriaName_MissingSeriaIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setSeriaName"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("chartIndex"), 0);
        scope.insert(QStringLiteral("name"), QStringLiteral("Revenue"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddSeria_ValidScope_PassesValidation", Test_AddSeria_ValidScope_PassesValidation},
        {"AddSeria_MissingValuesRange_SchemaInvalid", Test_AddSeria_MissingValuesRange_SchemaInvalid},
        {"SetSeriaName_ValidScope_PassesValidation", Test_SetSeriaName_ValidScope_PassesValidation},
        {"SetSeriaName_MissingSeriaIndex_SchemaInvalid", Test_SetSeriaName_MissingSeriaIndex_SchemaInvalid},
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
