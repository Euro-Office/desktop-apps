// Test cases from gateway-test-case-designs.md §C8 (Cell AutoFilter). See
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
    bool Test_ApplyFilter_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.applyFilter"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1:C10"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_ApplyFilter_MissingRange_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.applyFilter"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetFilters_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getFilters"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetFilters_MissingSheet_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getFilters"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"ApplyFilter_ValidScope_PassesValidation", Test_ApplyFilter_ValidScope_PassesValidation},
        {"ApplyFilter_MissingRange_SchemaInvalid", Test_ApplyFilter_MissingRange_SchemaInvalid},
        {"GetFilters_ValidScope_PassesValidation", Test_GetFilters_ValidScope_PassesValidation},
        {"GetFilters_MissingSheet_SchemaInvalid", Test_GetFilters_MissingSheet_SchemaInvalid},
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
