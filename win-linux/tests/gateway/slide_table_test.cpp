// Test cases from gateway-test-case-designs.md §D8 (Slide table editing; creation
// deferred, see that section's header note). See
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
    bool Test_AddRow_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.addRow"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("tableIndex"), 0);
        return spec->validate(scope).isEmpty();
    }

    bool Test_MergeCells_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.mergeCells"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("tableIndex"), 0);
        scope.insert(QStringLiteral("fromRow"), 0);
        scope.insert(QStringLiteral("fromCol"), 0);
        scope.insert(QStringLiteral("toRow"), 0);
        scope.insert(QStringLiteral("toCol"), 1);
        return spec->validate(scope).isEmpty();
    }

    bool Test_MergeCells_MissingToCol_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.mergeCells"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("index"), 0);
        scope.insert(QStringLiteral("tableIndex"), 0);
        scope.insert(QStringLiteral("fromRow"), 0);
        scope.insert(QStringLiteral("fromCol"), 0);
        scope.insert(QStringLiteral("toRow"), 0);
        return !spec->validate(scope).isEmpty();
    }

    // §D8: slide.createTable is deliberately not registered.
    bool Test_CreateTable_NotImplemented_NotAllowlisted()
    {
        return Gateway::AllowlistTable::Instance().Find(QStringLiteral("slide.createTable")) == nullptr;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterSlideCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddRow_ValidScope_PassesValidation", Test_AddRow_ValidScope_PassesValidation},
        {"MergeCells_ValidScope_PassesValidation", Test_MergeCells_ValidScope_PassesValidation},
        {"MergeCells_MissingToCol_SchemaInvalid", Test_MergeCells_MissingToCol_SchemaInvalid},
        {"CreateTable_NotImplemented_NotAllowlisted", Test_CreateTable_NotImplemented_NotAllowlisted},
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
