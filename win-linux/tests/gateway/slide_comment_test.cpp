// Test cases from gateway-test-case-designs.md §D10 (Slide comments). See
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
    bool Test_AddComment_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.addComment"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("x"), 0);
        scope.insert(QStringLiteral("y"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("fix typo"));
        scope.insert(QStringLiteral("author"), QStringLiteral("peter"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddComment_MissingY_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.addComment"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("x"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("fix typo"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetAllComments_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("presentation.getAllComments"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddComment_ValidScope_PassesValidation", Test_AddComment_ValidScope_PassesValidation},
        {"AddComment_MissingY_SchemaInvalid", Test_AddComment_MissingY_SchemaInvalid},
        {"GetAllComments_Registered_NoScopeRequired", Test_GetAllComments_Registered_NoScopeRequired},
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
