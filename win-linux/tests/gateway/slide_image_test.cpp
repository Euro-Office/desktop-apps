// Test cases from gateway-test-case-designs.md §D7 (Slide insert images). See
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
    bool Test_CreateImage_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.createImage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("imageSrc"), QStringLiteral("data:image/png;base64,AAAA"));
        scope.insert(QStringLiteral("x"), 0);
        scope.insert(QStringLiteral("y"), 0);
        scope.insert(QStringLiteral("width"), 914400);
        scope.insert(QStringLiteral("height"), 914400);
        return spec->validate(scope).isEmpty();
    }

    bool Test_CreateImage_MissingImageSrc_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.createImage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("x"), 0);
        scope.insert(QStringLiteral("y"), 0);
        scope.insert(QStringLiteral("width"), 914400);
        scope.insert(QStringLiteral("height"), 914400);
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"CreateImage_ValidScope_PassesValidation", Test_CreateImage_ValidScope_PassesValidation},
        {"CreateImage_MissingImageSrc_SchemaInvalid", Test_CreateImage_MissingImageSrc_SchemaInvalid},
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
