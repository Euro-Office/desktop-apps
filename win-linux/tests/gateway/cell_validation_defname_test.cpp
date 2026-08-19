// Test cases from gateway-test-case-designs.md §C7 (Cell data validation and named
// ranges). See word_document_properties_test.cpp's header comment for scope/limits
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
    bool Test_AddValidation_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addValidation"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("type"), QStringLiteral("xlValidateWholeNumber"));
        scope.insert(QStringLiteral("operator"), QStringLiteral("xlBetween"));
        scope.insert(QStringLiteral("formula1"), QStringLiteral("1"));
        scope.insert(QStringLiteral("formula2"), QStringLiteral("10"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddValidation_MissingFormula1_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addValidation"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("type"), QStringLiteral("xlValidateWholeNumber"));
        scope.insert(QStringLiteral("operator"), QStringLiteral("xlBetween"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddDefName_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addDefName"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("MyRange"));
        scope.insert(QStringLiteral("refersTo"), QStringLiteral("Sheet1!$A$1:$A$5"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddDefName_MissingRefersTo_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addDefName"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("MyRange"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddValidation_ValidScope_PassesValidation", Test_AddValidation_ValidScope_PassesValidation},
        {"AddValidation_MissingFormula1_SchemaInvalid", Test_AddValidation_MissingFormula1_SchemaInvalid},
        {"AddDefName_ValidScope_PassesValidation", Test_AddDefName_ValidScope_PassesValidation},
        {"AddDefName_MissingRefersTo_SchemaInvalid", Test_AddDefName_MissingRefersTo_SchemaInvalid},
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
