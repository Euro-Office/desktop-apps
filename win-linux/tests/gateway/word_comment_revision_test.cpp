// Test cases from gateway-test-case-designs.md §B13 (Word comments and track
// changes). See word_document_properties_test.cpp's header comment for scope/limits
// shared by every file in this directory.

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
    bool Test_AddComment_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addComment"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("needs review"));
        scope.insert(QStringLiteral("author"), QStringLiteral("peter"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddComment_EmptyText_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addComment"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QString());
        scope.insert(QStringLiteral("author"), QStringLiteral("peter"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetAllComments_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getAllComments"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_SetTrackRevisions_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setTrackRevisions"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("enabled"), true);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetTrackRevisions_MissingEnabled_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setTrackRevisions"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_AcceptAllRevisionChanges_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.acceptAllRevisionChanges"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddComment_ValidScope_PassesValidation", Test_AddComment_ValidScope_PassesValidation},
        {"AddComment_EmptyText_SchemaInvalid", Test_AddComment_EmptyText_SchemaInvalid},
        {"GetAllComments_Registered_NoScopeRequired", Test_GetAllComments_Registered_NoScopeRequired},
        {"SetTrackRevisions_ValidScope_PassesValidation", Test_SetTrackRevisions_ValidScope_PassesValidation},
        {"SetTrackRevisions_MissingEnabled_SchemaInvalid", Test_SetTrackRevisions_MissingEnabled_SchemaInvalid},
        {"AcceptAllRevisionChanges_Registered_NoScopeRequired", Test_AcceptAllRevisionChanges_Registered_NoScopeRequired},
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
