// Test cases from gateway-test-case-designs.md §D1 (Slide management). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory: schema validation only, round trips deferred to §6.

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
    bool Test_AddSlide_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.addSlide"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 1);
        return spec->validate(scope).isEmpty();
    }

    bool Test_RemoveSlides_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.removeSlides"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("start"), 1);
        scope.insert(QStringLiteral("count"), 1);
        return spec->validate(scope).isEmpty();
    }

    bool Test_RemoveSlides_ZeroCount_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.removeSlides"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("start"), 1);
        scope.insert(QStringLiteral("count"), 0);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_Duplicate_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.duplicate"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_MoveTo_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.moveTo"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("newIndex"), 2);
        return spec->validate(scope).isEmpty();
    }

    bool Test_MoveTo_MissingNewIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.moveTo"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddSlide_ValidScope_PassesValidation", Test_AddSlide_ValidScope_PassesValidation},
        {"RemoveSlides_ValidScope_PassesValidation", Test_RemoveSlides_ValidScope_PassesValidation},
        {"RemoveSlides_ZeroCount_SchemaInvalid", Test_RemoveSlides_ZeroCount_SchemaInvalid},
        {"Duplicate_ValidScope_PassesValidation", Test_Duplicate_ValidScope_PassesValidation},
        {"MoveTo_ValidScope_PassesValidation", Test_MoveTo_ValidScope_PassesValidation},
        {"MoveTo_MissingNewIndex_SchemaInvalid", Test_MoveTo_MissingNewIndex_SchemaInvalid},
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
