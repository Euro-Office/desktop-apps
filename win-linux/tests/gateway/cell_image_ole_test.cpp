// Test cases from gateway-test-case-designs.md §C11 (Cell insert images/OLE objects;
// shapes deferred, see that section's header note). See
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
    QJsonObject PlacementFields()
    {
        QJsonObject scope;
        scope.insert(QStringLiteral("fromCol"), 0);
        scope.insert(QStringLiteral("colOffset"), 0);
        scope.insert(QStringLiteral("fromRow"), 0);
        scope.insert(QStringLiteral("rowOffset"), 0);
        return scope;
    }

    bool Test_AddImage_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addImage"));
        if (!spec) return false;
        QJsonObject scope = PlacementFields();
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("imageSrc"), QStringLiteral("data:image/png;base64,AAAA"));
        scope.insert(QStringLiteral("width"), 914400);
        scope.insert(QStringLiteral("height"), 914400);
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddImage_MissingPlacementField_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addImage"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("imageSrc"), QStringLiteral("data:image/png;base64,AAAA"));
        scope.insert(QStringLiteral("width"), 914400);
        scope.insert(QStringLiteral("height"), 914400);
        return !spec->validate(scope).isEmpty();
    }

    bool Test_AddOleObject_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addOleObject"));
        if (!spec) return false;
        QJsonObject scope = PlacementFields();
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("imageSrc"), QStringLiteral("data:image/png;base64,AAAA"));
        scope.insert(QStringLiteral("width"), 914400);
        scope.insert(QStringLiteral("height"), 914400);
        scope.insert(QStringLiteral("data"), QStringLiteral("payload"));
        scope.insert(QStringLiteral("appId"), QStringLiteral("x-office/binary"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_AddOleObject_MissingAppId_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addOleObject"));
        if (!spec) return false;
        QJsonObject scope = PlacementFields();
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("imageSrc"), QStringLiteral("data:image/png;base64,AAAA"));
        scope.insert(QStringLiteral("width"), 914400);
        scope.insert(QStringLiteral("height"), 914400);
        scope.insert(QStringLiteral("data"), QStringLiteral("payload"));
        return !spec->validate(scope).isEmpty();
    }

    // §C11: cell.addShape is deliberately not registered.
    bool Test_AddShape_NotImplemented_NotAllowlisted()
    {
        return Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.addShape")) == nullptr;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AddImage_ValidScope_PassesValidation", Test_AddImage_ValidScope_PassesValidation},
        {"AddImage_MissingPlacementField_SchemaInvalid", Test_AddImage_MissingPlacementField_SchemaInvalid},
        {"AddOleObject_ValidScope_PassesValidation", Test_AddOleObject_ValidScope_PassesValidation},
        {"AddOleObject_MissingAppId_SchemaInvalid", Test_AddOleObject_MissingAppId_SchemaInvalid},
        {"AddShape_NotImplemented_NotAllowlisted", Test_AddShape_NotImplemented_NotAllowlisted},
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
