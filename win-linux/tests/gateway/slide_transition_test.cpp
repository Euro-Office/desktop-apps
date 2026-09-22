// Test cases from gateway-test-case-designs.md §D4 (Slide transitions; background
// deferred, see that section's header note). See
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
    bool Test_SetTransition_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setTransition"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("entryEffect"), QStringLiteral("effectFade"));
        scope.insert(QStringLiteral("duration"), 500);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetTransition_MissingEntryEffect_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setTransition"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("duration"), 500);
        return !spec->validate(scope).isEmpty();
    }

    // §D4: slide.setBackground is deliberately not registered.
    bool Test_SetBackground_NotImplemented_NotAllowlisted()
    {
        return Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setBackground")) == nullptr;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetTransition_ValidScope_PassesValidation", Test_SetTransition_ValidScope_PassesValidation},
        {"SetTransition_MissingEntryEffect_SchemaInvalid", Test_SetTransition_MissingEntryEffect_SchemaInvalid},
        {"SetBackground_NotImplemented_NotAllowlisted", Test_SetBackground_NotImplemented_NotAllowlisted},
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
