// Test cases from gateway-test-case-designs.md §B5 (Word paragraph formatting). See
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
    bool Test_SetJc_ValidAlignment_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setJc"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("align"), QStringLiteral("center"));
        return spec->validate(scope).isEmpty();
    }

    // B5.2
    bool Test_SetJc_InvalidAlignment_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setJc"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("align"), QStringLiteral("diagonal"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetSpacingBefore_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setSpacingBefore"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("twips"), 240);
        return spec->validate(scope).isEmpty();
    }

    // B5.4: negative twips (hanging indent) is valid for setIndLeft.
    bool Test_SetIndLeft_NegativeTwips_StillValid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setIndLeft"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("twips"), -100);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetIndLeft_MissingTwips_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setIndLeft"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetJc_ValidAlignment_PassesValidation", Test_SetJc_ValidAlignment_PassesValidation},
        {"SetJc_InvalidAlignment_SchemaInvalid", Test_SetJc_InvalidAlignment_SchemaInvalid},
        {"SetSpacingBefore_ValidScope_PassesValidation", Test_SetSpacingBefore_ValidScope_PassesValidation},
        {"SetIndLeft_NegativeTwips_StillValid", Test_SetIndLeft_NegativeTwips_StillValid},
        {"SetIndLeft_MissingTwips_SchemaInvalid", Test_SetIndLeft_MissingTwips_SchemaInvalid},
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
