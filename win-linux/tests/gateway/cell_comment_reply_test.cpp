// Test cases from gateway-test-case-designs.md §C12 (Cell comments with replies). See
// word_document_properties_test.cpp's header comment for scope/limits shared by every
// file in this directory.

#include "../../src/gateway/allowlist.h"
#include "../../src/gateway/commands/cellcommands.h"

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
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addComment"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("text"), QStringLiteral("check this"));
        scope.insert(QStringLiteral("author"), QStringLiteral("peter"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddComment_EmptyText_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addComment"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("text"), QString());
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddReply_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addReply"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("commentId"), QStringLiteral("comment-1"));
        scope.insert(QStringLiteral("text"), QStringLiteral("done"));
        scope.insert(QStringLiteral("author"), QStringLiteral("jane"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddReply_MissingCommentId_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addReply"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("text"), QStringLiteral("done"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetSolved_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setSolved"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("commentId"), QStringLiteral("comment-1"));
        scope.insert(QStringLiteral("solved"), true);
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetSolved_MissingSolved_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setSolved"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("commentId"), QStringLiteral("comment-1"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddComment_ValidScope_PassesValidation", Test_AddComment_ValidScope_PassesValidation},
        {"AddComment_EmptyText_SchemaInvalid", Test_AddComment_EmptyText_SchemaInvalid},
        {"AddReply_ValidScope_PassesValidation", Test_AddReply_ValidScope_PassesValidation},
        {"AddReply_MissingCommentId_SchemaInvalid", Test_AddReply_MissingCommentId_SchemaInvalid},
        {"SetSolved_ValidScope_PassesValidation", Test_SetSolved_ValidScope_PassesValidation},
        {"SetSolved_MissingSolved_SchemaInvalid", Test_SetSolved_MissingSolved_SchemaInvalid},
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
