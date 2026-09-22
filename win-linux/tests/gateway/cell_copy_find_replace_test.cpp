// Test cases from gateway-test-case-designs.md §C4 (Cell copy/paste, find/replace).
// See word_document_properties_test.cpp's header comment for scope/limits shared by
// every file in this directory.

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
    bool Test_Copy_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.copy"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("from"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("to"), QStringLiteral("B1"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_Copy_MissingTo_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.copy"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("from"), QStringLiteral("A1"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_Find_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.find"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("text"), QStringLiteral("foo"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_Replace_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.replace"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("find"), QStringLiteral("foo"));
        scope.insert(QStringLiteral("replace"), QStringLiteral("baz"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_Replace_MissingFind_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.replace"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("replace"), QStringLiteral("baz"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"Copy_ValidScope_PassesValidation", Test_Copy_ValidScope_PassesValidation},
        {"Copy_MissingTo_SchemaInvalid", Test_Copy_MissingTo_SchemaInvalid},
        {"Find_ValidScope_PassesValidation", Test_Find_ValidScope_PassesValidation},
        {"Replace_ValidScope_PassesValidation", Test_Replace_ValidScope_PassesValidation},
        {"Replace_MissingFind_SchemaInvalid", Test_Replace_MissingFind_SchemaInvalid},
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
