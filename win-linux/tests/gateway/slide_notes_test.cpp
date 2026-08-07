// Test cases from gateway-test-case-designs.md §D9 (Slide speaker notes). See
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
    bool Test_AddNotesText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.addNotesText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("Remember to mention Q3"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddNotesText_EmptyText_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.addNotesText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("text"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetNotesText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.getNotesText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddNotesText_ValidScope_PassesValidation", Test_AddNotesText_ValidScope_PassesValidation},
        {"AddNotesText_EmptyText_SchemaInvalid", Test_AddNotesText_EmptyText_SchemaInvalid},
        {"GetNotesText_ValidScope_PassesValidation", Test_GetNotesText_ValidScope_PassesValidation},
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
