// Test cases from gateway-test-case-designs.md §C5 (Cell font/fill/border/alignment
// formatting). See word_document_properties_test.cpp's header comment for
// scope/limits shared by every file in this directory.

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
    bool Test_SetFontName_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setFontName"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("font"), QStringLiteral("Calibri"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetFillColor_ValidHex_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setFillColor"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("color"), QStringLiteral("#FFFF00"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetFillColor_InvalidColor_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setFillColor"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("color"), QStringLiteral("yellow"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetBorders_AllEdges_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setBorders"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("edge"), QStringLiteral("all"));
        scope.insert(QStringLiteral("style"), QStringLiteral("Thin"));
        scope.insert(QStringLiteral("color"), QStringLiteral("#000000"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetBorders_UnknownEdge_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setBorders"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("edge"), QStringLiteral("Sideways"));
        scope.insert(QStringLiteral("style"), QStringLiteral("Thin"));
        scope.insert(QStringLiteral("color"), QStringLiteral("#000000"));
        return !spec->validate(scope).isEmpty();
    }

    bool Test_SetAlignHorizontal_ValidScope_PassesValidation()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setAlignHorizontal"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("align"), QStringLiteral("center"));
        return spec->validate(scope).isEmpty();
    }

    bool Test_SetAlignHorizontal_UnknownAlign_SchemaInvalid()
    {
        const Gateway::CommandSpec* spec = Gateway::AllowlistTable::Instance().Find(QStringLiteral("cell.setAlignHorizontal"));
        if (!spec) return false;
        QJsonObject scope;
        scope.insert(QStringLiteral("sheet"), QStringLiteral("Sheet1"));
        scope.insert(QStringLiteral("range"), QStringLiteral("A1"));
        scope.insert(QStringLiteral("align"), QStringLiteral("diagonal"));
        return !spec->validate(scope).isEmpty();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Gateway::Commands::RegisterCellCommands();

    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"SetFontName_ValidScope_PassesValidation", Test_SetFontName_ValidScope_PassesValidation},
        {"SetFillColor_ValidHex_PassesValidation", Test_SetFillColor_ValidHex_PassesValidation},
        {"SetFillColor_InvalidColor_SchemaInvalid", Test_SetFillColor_InvalidColor_SchemaInvalid},
        {"SetBorders_AllEdges_PassesValidation", Test_SetBorders_AllEdges_PassesValidation},
        {"SetBorders_UnknownEdge_SchemaInvalid", Test_SetBorders_UnknownEdge_SchemaInvalid},
        {"SetAlignHorizontal_ValidScope_PassesValidation", Test_SetAlignHorizontal_ValidScope_PassesValidation},
        {"SetAlignHorizontal_UnknownAlign_SchemaInvalid", Test_SetAlignHorizontal_UnknownAlign_SchemaInvalid},
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
