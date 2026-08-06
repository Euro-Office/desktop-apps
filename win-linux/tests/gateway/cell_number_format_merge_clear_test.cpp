// Test cases from gateway-test-case-designs.md §C3 (Cell number formats, merge,
// clear). See word_document_properties_test.cpp's header comment for scope/limits
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
    bool Test_SetNumberFormat_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setNumberFormat"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("format"), QStringLiteral("0.00"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetNumberFormat_EmptyFormat_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setNumberFormat"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("format"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_Merge_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.merge"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1:B2"));
        scope.insert(QStringLiteral("across"), false);
        return spec->validate(scope).isEmpty();
    }

    bool Test_Merge_MissingAcross_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.merge"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1:B2"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_ClearContents_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.clearContents"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        return spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetNumberFormat_ValidScope_PassesValidation", Test_SetNumberFormat_ValidScope_PassesValidation},
        {"SetNumberFormat_EmptyFormat_SchemaInvalid", Test_SetNumberFormat_EmptyFormat_SchemaInvalid},
        {"Merge_ValidScope_PassesValidation", Test_Merge_ValidScope_PassesValidation},
        {"Merge_MissingAcross_SchemaInvalid", Test_Merge_MissingAcross_SchemaInvalid},
        {"ClearContents_ValidScope_PassesValidation", Test_ClearContents_ValidScope_PassesValidation},
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
