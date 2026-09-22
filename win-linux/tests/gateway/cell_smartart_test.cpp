// Test cases from gateway-test-case-designs.md §C16 (Cell read SmartArt object type).
// See word_document_properties_test.cpp's header comment for scope/limits shared by
// every file in this directory. This is the last of the 16 Cell command families per
// cdp-gateway-cli-plan.md §4.

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
    bool Test_GetSmartArtClassType_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getSmartArtClassType"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetSmartArtClassType_MissingIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.getSmartArtClassType"));
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
        {"GetSmartArtClassType_ValidScope_PassesValidation", Test_GetSmartArtClassType_ValidScope_PassesValidation},
        {"GetSmartArtClassType_MissingIndex_SchemaInvalid", Test_GetSmartArtClassType_MissingIndex_SchemaInvalid},
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
