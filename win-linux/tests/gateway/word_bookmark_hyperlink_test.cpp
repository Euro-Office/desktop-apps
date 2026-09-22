// Test cases from gateway-test-case-designs.md §B11 (Word bookmarks and hyperlinks).
// See word_document_properties_test.cpp's header comment for scope/limits shared by
// every file in this directory.

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
    bool Test_AddBookmark_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addBookmark"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("name"), QStringLiteral("section1"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_GetBookmark_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getBookmark"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QStringLiteral("section1"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddHyperlink_HttpsUrl_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addHyperlink"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("link"));
        scope.insert(QStringLiteral("url"), QStringLiteral("https://example.com"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddHyperlink_MailtoUrl_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addHyperlink"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("mail me"));
        scope.insert(QStringLiteral("url"), QStringLiteral("mailto:someone@example.com"));
        return spec->validate(scope).isEmpty();
    }

    // B11.3: javascript: scheme is rejected as a security boundary.
    bool Test_AddHyperlink_JavascriptScheme_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addHyperlink"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("link"));
        scope.insert(QStringLiteral("url"), QStringLiteral("javascript:alert(1)"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddHyperlink_FileScheme_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.addHyperlink"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("paraIndex"), 0);
        scope.insert(QStringLiteral("text"), QStringLiteral("link"));
        scope.insert(QStringLiteral("url"), QStringLiteral("file:///etc/passwd"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddBookmark_ValidScope_PassesValidation", Test_AddBookmark_ValidScope_PassesValidation},
        {"GetBookmark_ValidScope_PassesValidation", Test_GetBookmark_ValidScope_PassesValidation},
        {"AddHyperlink_HttpsUrl_PassesValidation", Test_AddHyperlink_HttpsUrl_PassesValidation},
        {"AddHyperlink_MailtoUrl_PassesValidation", Test_AddHyperlink_MailtoUrl_PassesValidation},
        {"AddHyperlink_JavascriptScheme_SchemaInvalid", Test_AddHyperlink_JavascriptScheme_SchemaInvalid},
        {"AddHyperlink_FileScheme_SchemaInvalid", Test_AddHyperlink_FileScheme_SchemaInvalid},
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
