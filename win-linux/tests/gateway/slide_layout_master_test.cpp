// Test cases from gateway-test-case-designs.md §D3 (Slide layouts/masters; theme
// application deferred, see that section's header note). See
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
    bool Test_GetLayout_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.getLayout"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_ApplyLayout_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.applyLayout"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("fromIndex"), 1);
        return spec->validate(scope).isEmpty();
    }

    bool Test_ApplyLayout_MissingFromIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.applyLayout"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddMaster_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.addMaster"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("position"), 1);
        return spec->validate(scope).isEmpty();
    }

    // §D3: slide.applyTheme is deliberately not registered.
    bool Test_ApplyTheme_NotImplemented_NotAllowlisted()
    {
        return Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.applyTheme")) == nullptr;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"GetLayout_ValidScope_PassesValidation", Test_GetLayout_ValidScope_PassesValidation},
        {"ApplyLayout_ValidScope_PassesValidation", Test_ApplyLayout_ValidScope_PassesValidation},
        {"ApplyLayout_MissingFromIndex_SchemaInvalid", Test_ApplyLayout_MissingFromIndex_SchemaInvalid},
        {"AddMaster_ValidScope_PassesValidation", Test_AddMaster_ValidScope_PassesValidation},
        {"ApplyTheme_NotImplemented_NotAllowlisted", Test_ApplyTheme_NotImplemented_NotAllowlisted},
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
