// Test cases from gateway-test-case-designs.md §B2 (Word content enumeration),
// implemented against AllowlistTable/CommandSpec directly. See
// word_document_properties_test.cpp's header comment for what this harness can and
// cannot verify without a live CCefView -- the same limits apply here: only allowlist
// registration and scope validation are checked; the round-trip cases (does
// word.getAllParagraphs actually return [0,1,2] against a 3-paragraph document) are
// exercised at the plan's §6 per-editor build/deploy/test gate instead.

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
    bool Test_GetAllParagraphs_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getAllParagraphs"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_GetAllTables_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getAllTables"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_GetAllDrawingObjects_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getAllDrawingObjects"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }

    bool Test_GetAllCharts_Registered_NoScopeRequired()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getAllCharts"));
        return spec && spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"GetAllParagraphs_Registered_NoScopeRequired", Test_GetAllParagraphs_Registered_NoScopeRequired},
        {"GetAllTables_Registered_NoScopeRequired", Test_GetAllTables_Registered_NoScopeRequired},
        {"GetAllDrawingObjects_Registered_NoScopeRequired", Test_GetAllDrawingObjects_Registered_NoScopeRequired},
        {"GetAllCharts_Registered_NoScopeRequired", Test_GetAllCharts_Registered_NoScopeRequired},
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
