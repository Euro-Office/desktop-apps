// Test cases from gateway-test-case-designs.md §D2 (Slide enumerate content). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory.

#include "../../src/gateway/allowlist.h"
#include "../../src/gateway/commands/slidecommands.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <cstdio>
#include <vector>
#include <functional>
#include <string>

namespace
{
    bool Test_GetAllShapes_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.getAllShapes"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetAllImages_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.getAllImages"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetAllTables_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.getAllTables"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetAllCharts_MissingIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.getAllCharts"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"GetAllShapes_ValidScope_PassesValidation", Test_GetAllShapes_ValidScope_PassesValidation},
        {"GetAllImages_ValidScope_PassesValidation", Test_GetAllImages_ValidScope_PassesValidation},
        {"GetAllTables_ValidScope_PassesValidation", Test_GetAllTables_ValidScope_PassesValidation},
        {"GetAllCharts_MissingIndex_SchemaInvalid", Test_GetAllCharts_MissingIndex_SchemaInvalid},
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
