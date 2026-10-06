// Test cases from gateway-test-case-designs.md §B10 (Word headers/footers, page
// setup). See word_document_properties_test.cpp's header comment for scope/limits
// shared by every file in this directory.

#include "../../src/gateway/allowlist.h"
#include "../../src/gateway/commands/wordcommands.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <cstdio>
#include <vector>
#include <functional>
#include <string>

namespace
{
    bool Test_SetHeaderText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setHeaderText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("type"), QStringLiteral("default"));
        scope.insert(QStringLiteral("text"), QStringLiteral("Confidential"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetHeaderText_UnknownType_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setHeaderText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("type"), QStringLiteral("bogus"));
        scope.insert(QStringLiteral("text"), QStringLiteral("x"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetPageMargins_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setPageMargins"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("left"), 1440);
        scope.insert(QStringLiteral("top"), 1440);
        scope.insert(QStringLiteral("right"), 1440);
        scope.insert(QStringLiteral("bottom"), 1440);
        return spec->validate(scope).isEmpty();
    }

    // B10.3
    bool Test_SetPageSize_ZeroWidth_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setPageSize"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("width"), 0);
        scope.insert(QStringLiteral("height"), 0);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetPageSize_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setPageSize"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("width"), 12240);
        scope.insert(QStringLiteral("height"), 15840);
        return spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetHeaderText_ValidScope_PassesValidation", Test_SetHeaderText_ValidScope_PassesValidation},
        {"SetHeaderText_UnknownType_SchemaInvalid", Test_SetHeaderText_UnknownType_SchemaInvalid},
        {"SetPageMargins_ValidScope_PassesValidation", Test_SetPageMargins_ValidScope_PassesValidation},
        {"SetPageSize_ZeroWidth_SchemaInvalid", Test_SetPageSize_ZeroWidth_SchemaInvalid},
        {"SetPageSize_ValidScope_PassesValidation", Test_SetPageSize_ValidScope_PassesValidation},
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
