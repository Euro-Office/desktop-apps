// Test cases from gateway-test-case-designs.md §D5 (Slide insert shapes with
// positioning). See word_document_properties_test.cpp's header comment for
// scope/limits shared by every file in this directory.

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
    bool Test_CreateShape_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.createShape"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("type"), QStringLiteral("rect"));
        scope.insert(QStringLiteral("x"), 10);
        scope.insert(QStringLiteral("y"), 10);
        scope.insert(QStringLiteral("width"), 100);
        scope.insert(QStringLiteral("height"), 50);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetPosition_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setPosition"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("shapeIndex"), 0);
        scope.insert(QStringLiteral("x"), 200);
        scope.insert(QStringLiteral("y"), 200);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetRotation_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setRotation"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("shapeIndex"), 0);
        scope.insert(QStringLiteral("degrees"), 45);
        return spec->validate(scope).isEmpty();
    }

    // D5.4
    bool Test_SetSize_NegativeWidth_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setSize"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("shapeIndex"), 0);
        scope.insert(QStringLiteral("width"), -10);
        scope.insert(QStringLiteral("height"), 50);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetSize_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.setSize"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("shapeIndex"), 0);
        scope.insert(QStringLiteral("width"), 100);
        scope.insert(QStringLiteral("height"), 50);
        return spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"CreateShape_ValidScope_PassesValidation", Test_CreateShape_ValidScope_PassesValidation},
        {"SetPosition_ValidScope_PassesValidation", Test_SetPosition_ValidScope_PassesValidation},
        {"SetRotation_ValidScope_PassesValidation", Test_SetRotation_ValidScope_PassesValidation},
        {"SetSize_NegativeWidth_SchemaInvalid", Test_SetSize_NegativeWidth_SchemaInvalid},
        {"SetSize_ValidScope_PassesValidation", Test_SetSize_ValidScope_PassesValidation},
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
