// Test cases from gateway-test-case-designs.md §D11 (Slide document properties). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory. This is the last of the 11 Slide command families per
// cdp-gateway-cli-plan.md §4.

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
    bool Test_GetDocumentInfo_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("presentation.getDocumentInfo"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_GetCustomProperty_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("presentation.getCustomProperty"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("Reviewed"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetCustomProperty_EmptyName_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("presentation.getCustomProperty"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QString());
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"GetDocumentInfo_Registered_NoScopeRequired", Test_GetDocumentInfo_Registered_NoScopeRequired},
        {"GetCustomProperty_ValidScope_PassesValidation", Test_GetCustomProperty_ValidScope_PassesValidation},
        {"GetCustomProperty_EmptyName_SchemaInvalid", Test_GetCustomProperty_EmptyName_SchemaInvalid},
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
