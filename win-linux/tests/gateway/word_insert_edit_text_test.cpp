// Test cases from gateway-test-case-designs.md §B3 (Word insert/edit text). See
// word_document_properties_test.cpp's header comment for the scope/limits shared by
// every file in this directory: schema validation only, round trips deferred to §6.

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
    bool Test_AddText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("Hello"));
        return spec->validate(scope).isEmpty();
    }

    // B3.4: empty text is allowed.
    bool Test_AddText_EmptyText_Accepted()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QString());
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddText_MissingParaIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("text"), QStringLiteral("x"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddText_NegativeParaIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), -1);
        scope.insert(QStringLiteral("text"), QStringLiteral("x"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_GetText_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 1);
        scope.insert(QStringLiteral("runIndex"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetText_MissingRunIndex_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getText"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 1);
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddText_ValidScope_PassesValidation", Test_AddText_ValidScope_PassesValidation},
        {"AddText_EmptyText_Accepted", Test_AddText_EmptyText_Accepted},
        {"AddText_MissingParaIndex_SchemaInvalid", Test_AddText_MissingParaIndex_SchemaInvalid},
        {"AddText_NegativeParaIndex_SchemaInvalid", Test_AddText_NegativeParaIndex_SchemaInvalid},
        {"GetText_ValidScope_PassesValidation", Test_GetText_ValidScope_PassesValidation},
        {"GetText_MissingRunIndex_SchemaInvalid", Test_GetText_MissingRunIndex_SchemaInvalid},
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
