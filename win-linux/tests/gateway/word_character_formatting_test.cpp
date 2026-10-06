// Test cases from gateway-test-case-designs.md §B4 (Word character formatting). See
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
    QJsonObject RunTarget()
    {
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("runIndex"), 0);
        return scope;
    }

    bool Test_SetBold_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setBold"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("bold"), true);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetBold_NonBoolBold_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setBold"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("bold"), QStringLiteral("yes"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetItalic_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setItalic"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("italic"), true);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetFontFamily_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setFontFamily"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("font"), QStringLiteral("Arial"));
        return spec->validate(scope).isEmpty();
    }

    // B4.4: a nonexistent font name is still schema-valid -- font substitution is a
    // rendering concern, not a gateway validation concern.
    bool Test_SetFontFamily_UnknownFontName_StillValid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setFontFamily"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("font"), QStringLiteral("NotARealFontXYZ"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetColor_ValidHex_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setColor"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("color"), QStringLiteral("#FF0000"));
        return spec->validate(scope).isEmpty();
    }

    // B4.6
    bool Test_SetColor_InvalidColorString_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setColor"));
        if (!spec) return false;
        QJsonObject scope = RunTarget();
        scope.insert(QStringLiteral("color"), QStringLiteral("not-a-color"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetBold_ValidScope_PassesValidation", Test_SetBold_ValidScope_PassesValidation},
        {"SetBold_NonBoolBold_SchemaInvalid", Test_SetBold_NonBoolBold_SchemaInvalid},
        {"SetItalic_ValidScope_PassesValidation", Test_SetItalic_ValidScope_PassesValidation},
        {"SetFontFamily_ValidScope_PassesValidation", Test_SetFontFamily_ValidScope_PassesValidation},
        {"SetFontFamily_UnknownFontName_StillValid", Test_SetFontFamily_UnknownFontName_StillValid},
        {"SetColor_ValidHex_PassesValidation", Test_SetColor_ValidHex_PassesValidation},
        {"SetColor_InvalidColorString_SchemaInvalid", Test_SetColor_InvalidColorString_SchemaInvalid},
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
