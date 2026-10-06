// Test cases from gateway-test-case-designs.md §A (cross-cutting dispatch) and §B1
// (Word document properties), implemented against AllowlistTable/CommandSpec directly.
//
// SCOPE OF WHAT THIS FILE CAN VERIFY RIGHT NOW: schema validation and allowlist lookup
// only (A1-A4, B1.2 negative half, B1.4). The cases that require a real CDP round trip
// against a running document (A5-A9, B1.1, B1.3, B1.5) are NOT executable from this
// standalone binary — GatewayCommandRunner now resolves targets and drives CDP for
// real (CAscApplicationManager::GetViewById + CCefView::SendGatewayDevToolsMessage,
// see gatewaycommandrunner.cpp), but that needs an actual running CCefView backed by
// a live CEF browser, which this lightweight CTest target deliberately does not spin
// up (would mean linking the whole ascdocumentscore/CEF stack into a "unit" test).
// Those round-trip cases are exercised instead at the per-editor gate
// (cdp-gateway-cli-plan.md §6): build the real DesktopEditors binary, run it, and
// drive these same commands through it end-to-end -- that's what "verify: test
// passes... against a running editor instance" (§5 step 4) actually means for this
// command family.
//
// No test framework dependency was added for this (repo has none currently - see
// investigation notes referenced from the plan); this is a minimal self-contained
// harness: each Test_* function returns true/false, main() aggregates and reports.

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
    bool Test_A1_UnknownCommand_NotAllowlisted()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("not.a.real.command"));
        return spec == nullptr;
    }

    bool Test_A2_MissingRequiredField_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setTitle"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty(); // "title" missing -> non-empty error string
    }

    bool Test_A3_WrongFieldType_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setCustomProperty"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), 42); // should be a string
        scope.insert(QStringLiteral("value"), QStringLiteral("x"));
        return !spec->validate(scope).isEmpty();
    }

    // B1.1's positive half (setTitle/getTitle round trip against a live document) is
    // blocked on the target-resolution gap — see file header. This only verifies the
    // command accepts a well-formed scope at the validation layer.
    bool Test_B1_1_SetTitle_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setTitle"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("title"), QStringLiteral("Q3 Report"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_B1_2_SetTitle_EmptyString_Accepted()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setTitle"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("title"), QString());
        return spec->validate(scope).isEmpty(); // empty string is allowed per B1.2
    }

    bool Test_B1_4_SetCustomProperty_EmptyName_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.setCustomProperty"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("name"), QString());
        scope.insert(QStringLiteral("value"), QStringLiteral("x"));
        return !spec->validate(scope).isEmpty(); // empty name rejected, per corrected B1.4
    }

    bool Test_GetCustomProperty_RequiresName()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("word.getCustomProperty"));
        if (!spec) return false;
        return !spec->validate(QJsonObject{}).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv); // QJsonObject/QString need a QCoreApplication-less
                                       // runtime technically, but keeping this consistent
                                       // with how the rest of the app initializes Qt types.

    Gateway::Commands::RegisterWordCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"A1_UnknownCommand_NotAllowlisted", Test_A1_UnknownCommand_NotAllowlisted},
        {"A2_MissingRequiredField_SchemaInvalid", Test_A2_MissingRequiredField_SchemaInvalid},
        {"A3_WrongFieldType_SchemaInvalid", Test_A3_WrongFieldType_SchemaInvalid},
        {"B1_1_SetTitle_ValidScope_PassesValidation", Test_B1_1_SetTitle_ValidScope_PassesValidation},
        {"B1_2_SetTitle_EmptyString_Accepted", Test_B1_2_SetTitle_EmptyString_Accepted},
        {"B1_4_SetCustomProperty_EmptyName_SchemaInvalid", Test_B1_4_SetCustomProperty_EmptyName_SchemaInvalid},
        {"GetCustomProperty_RequiresName", Test_GetCustomProperty_RequiresName},
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
