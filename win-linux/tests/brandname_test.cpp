/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

// The brand builds pass their name as ABOUT_PAGE_APP_NAME. Build this suite
// with a name of its own, so it checks that the names users see follow the
// brand while the application name, which data paths depend on, does not.
#undef ABOUT_PAGE_APP_NAME
#define ABOUT_PAGE_APP_NAME "Test Brand"

#include <gtest/gtest.h>
#include "defines.h"
#include "version.h"

TEST(BrandName, DisplayNameFollowsTheBrand)
{
    EXPECT_STREQ(APP_DISPLAY_NAME, "Test Brand");
}

TEST(BrandName, ApplicationNameStaysTheSame)
{
    EXPECT_STREQ(WINDOW_NAME, "Euro-Office");
}

TEST(BrandName, FilePropertiesFollowTheBrand)
{
    EXPECT_STREQ(VER_PRODUCTNAME_STR, "Test Brand");
    EXPECT_STREQ(VER_FILEDESCRIPTION_STR, "Test Brand");
}
