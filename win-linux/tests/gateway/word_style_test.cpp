// Test cases from gateway-test-case-designs.md §B8 (Word style creation and
// application). See word_document_properties_test.cpp's header comment for
// scope/limits shared by every file in this directory.

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
    bool Test_CreateStyle_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.createStyle"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("MyHeading"));
        scope.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_CreateStyle_InvalidType_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.createStyle"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("MyHeading"));
        scope.insert(QStringLiteral("type"), QStringLiteral("bogus"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetStyle_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getStyle"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("MyHeading"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetStyleTextPr_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setStyleTextPr"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("styleId"), QStringLiteral("MyHeading"));
        scope.insert(QStringLiteral("bold"), true);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetStyleTextPr_MissingBold_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setStyleTextPr"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("styleId"), QStringLiteral("MyHeading"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"CreateStyle_ValidScope_PassesValidation", Test_CreateStyle_ValidScope_PassesValidation},
        {"CreateStyle_InvalidType_SchemaInvalid", Test_CreateStyle_InvalidType_SchemaInvalid},
        {"GetStyle_ValidScope_PassesValidation", Test_GetStyle_ValidScope_PassesValidation},
        {"SetStyleTextPr_ValidScope_PassesValidation", Test_SetStyleTextPr_ValidScope_PassesValidation},
        {"SetStyleTextPr_MissingBold_SchemaInvalid", Test_SetStyleTextPr_MissingBold_SchemaInvalid},
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
