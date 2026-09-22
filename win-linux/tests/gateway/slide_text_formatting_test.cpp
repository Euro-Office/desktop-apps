// Test cases from gateway-test-case-designs.md §D6 (Slide text formatting). See
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
    QJsonObject RunTarget()
    {
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("shapeIndex"), 0);
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("runIndex"), 0);
        return scope;
    }

    bool Test_SetBold_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setBold"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("bold"), true);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetBold_MissingParaIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setBold"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("shapeIndex"), 0);
        scope.insert(QStringLiteral("runIndex"), 0);
        scope.insert(QStringLiteral("bold"), true);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetFontFamily_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setFontFamily"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("font"), QStringLiteral("Georgia"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetFontFamily_EmptyFont_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setFontFamily"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("font"), QString());
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetBold_ValidScope_PassesValidation", Test_SetBold_ValidScope_PassesValidation},
        {"SetBold_MissingParaIndex_SchemaInvalid", Test_SetBold_MissingParaIndex_SchemaInvalid},
        {"SetFontFamily_ValidScope_PassesValidation", Test_SetFontFamily_ValidScope_PassesValidation},
        {"SetFontFamily_EmptyFont_SchemaInvalid", Test_SetFontFamily_EmptyFont_SchemaInvalid},
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
