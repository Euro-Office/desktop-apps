/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include "cproviders.h"

// CProviders (src/cproviders.cpp) reads the cloud providers' config and
// decides whether an editor opened from a provider gets a frame
// ("editorFrameSize": "finite") or fills the tab.

// CProviders is a singleton and init() appends, so the suite configures it
// once for all checks.
class ProvidersTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        CProviders::instance().init(R"([
            {"provider": "Nextcloud", "editorFrameSize": "finite",
             "editorPage": "/apps/onlyoffice/"},
            {"provider": "seafile", "editorFrameSize": "finite",
             "editorPage": "regex:^https://[^/]+/lib/[^/]+/file/"},
            {"provider": "owncloud", "editorFrameSize": "full"}
        ])");
    }
};

TEST_F(ProvidersTest, KnownProviderUsesItsFrameSetting)
{
    EXPECT_TRUE(CProviders::instance().editorsHasFrame("https://example.com/x", "nextcloud"));
    EXPECT_FALSE(CProviders::instance().editorsHasFrame("https://example.com/x", "owncloud"));
}

TEST_F(ProvidersTest, ProviderNamesAreReadCaseInsensitively)
{
    // "Nextcloud" in the config matches the lower-case cloud name
    EXPECT_TRUE(CProviders::instance().editorsHasFrame("https://example.com/x", "nextcloud"));
}

TEST_F(ProvidersTest, EditorPageMatchesAsSubstring)
{
    EXPECT_TRUE(CProviders::instance().editorsHasFrame(
        "https://cloud.example.com/index.php/apps/onlyoffice/42", ""));
}

TEST_F(ProvidersTest, EditorPageCanBeACaseInsensitiveRegex)
{
    EXPECT_TRUE(CProviders::instance().editorsHasFrame(
        "https://sea.example.com/lib/abc-123/file/report.docx", ""));
    EXPECT_TRUE(CProviders::instance().editorsHasFrame(
        "HTTPS://SEA.EXAMPLE.COM/LIB/ABC-123/FILE/report.docx", ""));
    EXPECT_FALSE(CProviders::instance().editorsHasFrame(
        "https://sea.example.com/other/abc-123/file/report.docx", ""));
}

TEST_F(ProvidersTest, UnknownProviderAndPageHaveNoFrame)
{
    EXPECT_FALSE(CProviders::instance().editorsHasFrame("https://unknown.example.com/doc", "unknown"));
}

TEST_F(ProvidersTest, InvalidJsonIsIgnored)
{
    CProviders::instance().init("this is not json");

    EXPECT_TRUE(CProviders::instance().editorsHasFrame("https://example.com/x", "nextcloud"));
    EXPECT_FALSE(CProviders::instance().editorsHasFrame("https://unknown.example.com/doc", "unknown"));
}
