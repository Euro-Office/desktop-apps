// Test cases from gateway-test-case-designs.md §B6 (Word search & replace). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory.

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
    bool Test_Search_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.search"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("text"), QStringLiteral("foo"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_Search_EmptyText_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.search"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("text"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SearchAndReplace_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.searchAndReplace"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("find"), QStringLiteral("foo"));
        scope.insert(QStringLiteral("replace"), QStringLiteral("bar"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SearchAndReplace_MissingFind_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.searchAndReplace"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("replace"), QStringLiteral("bar"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"Search_ValidScope_PassesValidation", Test_Search_ValidScope_PassesValidation},
        {"Search_EmptyText_SchemaInvalid", Test_Search_EmptyText_SchemaInvalid},
        {"SearchAndReplace_ValidScope_PassesValidation", Test_SearchAndReplace_ValidScope_PassesValidation},
        {"SearchAndReplace_MissingFind_SchemaInvalid", Test_SearchAndReplace_MissingFind_SchemaInvalid},
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
