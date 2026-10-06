// Test cases from gateway-test-case-designs.md §B9 (Word insert images/shapes with
// positioning). See word_document_properties_test.cpp's header comment for
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
    bool Test_CreateImage_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.createImage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("imageSrc"), QStringLiteral("data:image/png;base64,AAAA"));
        scope.insert(QStringLiteral("width"), 914400);
        scope.insert(QStringLiteral("height"), 914400);
        return spec->validate(scope).isEmpty();
    }

    bool Test_CreateImage_ZeroWidth_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.createImage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("imageSrc"), QStringLiteral("data:image/png;base64,AAAA"));
        scope.insert(QStringLiteral("width"), 0);
        scope.insert(QStringLiteral("height"), 914400);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetWrappingStyle_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setWrappingStyle"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("drawingIndex"), 0);
        scope.insert(QStringLiteral("style"), QStringLiteral("square"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetWrappingStyle_UnknownStyle_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setWrappingStyle"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("drawingIndex"), 0);
        scope.insert(QStringLiteral("style"), QStringLiteral("bogus"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetHorPosition_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setHorPosition"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("drawingIndex"), 0);
        scope.insert(QStringLiteral("distanceEmu"), 200000);
        scope.insert(QStringLiteral("relativeTo"), QStringLiteral("page"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetHorPosition_UnknownRelativeTo_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setHorPosition"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("drawingIndex"), 0);
        scope.insert(QStringLiteral("distanceEmu"), 200000);
        scope.insert(QStringLiteral("relativeTo"), QStringLiteral("bogus"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"CreateImage_ValidScope_PassesValidation", Test_CreateImage_ValidScope_PassesValidation},
        {"CreateImage_ZeroWidth_SchemaInvalid", Test_CreateImage_ZeroWidth_SchemaInvalid},
        {"SetWrappingStyle_ValidScope_PassesValidation", Test_SetWrappingStyle_ValidScope_PassesValidation},
        {"SetWrappingStyle_UnknownStyle_SchemaInvalid", Test_SetWrappingStyle_UnknownStyle_SchemaInvalid},
        {"SetHorPosition_ValidScope_PassesValidation", Test_SetHorPosition_ValidScope_PassesValidation},
        {"SetHorPosition_UnknownRelativeTo_SchemaInvalid", Test_SetHorPosition_UnknownRelativeTo_SchemaInvalid},
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
