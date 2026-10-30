/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include "utils.h"

// Pure helpers from src/utils.cpp: URL handling, string helpers, scaling
// conversion and command line arguments.

namespace {

bool touch(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly);
}

} // namespace

TEST(UtilsReplaceBackslash, ReplacesEveryBackslash)
{
    EXPECT_EQ(Utils::replaceBackslash("C:\\Users\\me\\doc.docx"), "C:/Users/me/doc.docx");
}

TEST(UtilsReplaceBackslash, LeavesForwardSlashesAlone)
{
    EXPECT_EQ(Utils::replaceBackslash("/home/me/doc.docx"), "/home/me/doc.docx");
}

TEST(UtilsReplaceAll, ReplacesEveryOccurrence)
{
    std::wstring s = L"a-b-c";
    Utils::replaceAll(s, L"-", L"+");
    EXPECT_EQ(s, L"a+b+c");
}

TEST(UtilsReplaceAll, ReplacementContainingTheSearchDoesNotLoop)
{
    std::wstring s = L"aa";
    Utils::replaceAll(s, L"a", L"aa");
    EXPECT_EQ(s, L"aaaa");
}

TEST(UtilsGetPortalName, StripsSchemeAndTrailingSlash)
{
    EXPECT_EQ(Utils::getPortalName("https://cloud.example.com/"), "cloud.example.com");
    EXPECT_EQ(Utils::getPortalName("http://cloud.example.com"), "cloud.example.com");
}

TEST(UtilsGetPortalName, KeepsPathAndPort)
{
    EXPECT_EQ(Utils::getPortalName("https://example.com:8443/nextcloud/"), "example.com:8443/nextcloud");
}

TEST(UtilsGetPortalName, ReturnsNonHttpInputUnchanged)
{
    EXPECT_EQ(Utils::getPortalName("cloud.example.com"), "cloud.example.com");
    EXPECT_EQ(Utils::getPortalName(""), "");
}

TEST(UtilsNormalizeAppProtocolUrl, AppProtocolBecomesHttpsWithDesktopPlacement)
{
    EXPECT_EQ(Utils::normalizeAppProtocolUrl(L"oo-office:https://cloud.example.com/doc?id=1"),
              L"https://cloud.example.com/doc?id=1&placement=desktop");
}

TEST(UtilsNormalizeAppProtocolUrl, ExternalAscdesktopUrlBecomesItsPath)
{
    EXPECT_EQ(Utils::normalizeAppProtocolUrl(L"ascdesktop://external/https://example.com/help"),
              L"https://example.com/help");
}

TEST(UtilsNormalizeAppProtocolUrl, OtherUrlsAreUnchanged)
{
    EXPECT_EQ(Utils::normalizeAppProtocolUrl(L"https://example.com/a?b=c"), L"https://example.com/a?b=c");
    EXPECT_EQ(Utils::normalizeAppProtocolUrl(L"ascdesktop://other/x"), L"ascdesktop://other/x");
}

TEST(UtilsUniqFileName, ReturnsPathWhenItDoesNotExist)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const QString path = dir.filePath("new.docx");
    EXPECT_EQ(Utils::uniqFileName(path), path);
}

TEST(UtilsUniqFileName, AppendsTheFirstFreeNumber)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(touch(dir.filePath("doc.docx")));
    ASSERT_TRUE(touch(dir.filePath("doc1.docx")));

    EXPECT_EQ(Utils::uniqFileName(dir.filePath("doc.docx")),
              QDir(dir.path()).absoluteFilePath("doc2.docx"));
}

TEST(Scaling, FactorAndScalingConvertBothWays)
{
    const QStringList scalings = {"100", "125", "150", "175", "200", "225", "250",
                                  "275", "300", "350", "400", "450", "500"};
    for (const QString& scaling : scalings) {
        const std::wstring factor = Scaling::scalingToFactor(scaling);
        EXPECT_NE(factor, L"0") << scaling.toStdString();
        EXPECT_EQ(Scaling::factorToScaling(factor), scaling);
    }
}

TEST(Scaling, UnknownValuesMapToZero)
{
    EXPECT_EQ(Scaling::scalingToFactor("110"), L"0");
    EXPECT_EQ(Scaling::scalingToFactor("abc"), L"0");
    EXPECT_EQ(Scaling::factorToScaling(L"1.1"), "0");
}

// InputArgs keeps the arguments in a global list and init() appends to it, so
// all checks share one set of arguments.
class InputArgsTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        static char arg0[] = "DesktopEditors";
        static char arg1[] = "--lang=de-DE";
        static char arg2[] = "--help-url=https://example.com/help";
        static char arg3[] = "--single-window-app";
        static char* argv[] = {arg0, arg1, arg2, arg3};
        InputArgs::init(4, argv);
    }
};

TEST_F(InputArgsTest, SkipsTheProgramName)
{
    for (const auto& arg : InputArgs::arguments())
        EXPECT_NE(arg, L"DesktopEditors");
}

TEST_F(InputArgsTest, ContainsMatchesWholeOptionNames)
{
    EXPECT_TRUE(InputArgs::contains(L"--lang"));
    EXPECT_TRUE(InputArgs::contains(L"--single-window-app"));
    EXPECT_TRUE(InputArgs::contains(L"--help-url"));

    // a prefix of an option is not the option
    EXPECT_FALSE(InputArgs::contains(L"--help"));
    EXPECT_FALSE(InputArgs::contains(L"--single"));
    EXPECT_FALSE(InputArgs::contains(L"--missing"));
}

TEST_F(InputArgsTest, ArgumentValueIsTheTextAfterTheSeparator)
{
    EXPECT_EQ(InputArgs::argument_value(L"--lang"), L"de-DE");
    EXPECT_EQ(InputArgs::argument_value(L"--help-url"), L"https://example.com/help");
    EXPECT_EQ(InputArgs::argument_value(L"--missing"), L"");
}
