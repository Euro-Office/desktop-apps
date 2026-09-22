// Test cases from gateway-test-case-designs.md §C13 (Cell insert/delete rows and
// columns). See word_document_properties_test.cpp's header comment for scope/limits
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
    bool Test_InsertEntireRow_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.insertEntireRow"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("rowIndex"), 1);
        return spec->validate(scope).isEmpty();
    }

    // C13.3
    bool Test_InsertEntireRow_NegativeIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.insertEntireRow"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("rowIndex"), -1);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_DeleteEntireColumn_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.deleteEntireColumn"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("colIndex"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_DeleteEntireColumn_MissingColIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.deleteEntireColumn"));
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
        {"InsertEntireRow_ValidScope_PassesValidation", Test_InsertEntireRow_ValidScope_PassesValidation},
        {"InsertEntireRow_NegativeIndex_SchemaInvalid", Test_InsertEntireRow_NegativeIndex_SchemaInvalid},
        {"DeleteEntireColumn_ValidScope_PassesValidation", Test_DeleteEntireColumn_ValidScope_PassesValidation},
        {"DeleteEntireColumn_MissingColIndex_SchemaInvalid", Test_DeleteEntireColumn_MissingColIndex_SchemaInvalid},
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
