/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QFile>
#include <QTemporaryDir>
#include <QWidget>

// Checks the test setup itself rather than application code: GoogleTest runs,
// Qt links and resolves at run time, and widgets work headless. If this suite
// fails, every other desktop-apps suite would fail for the same reason.

TEST(TestSetup, RunsOnOffscreenPlatform)
{
    EXPECT_EQ(QGuiApplication::platformName(), QStringLiteral("offscreen"));
}

TEST(TestSetup, WidgetsCanBeShown)
{
    QWidget widget;
    widget.resize(200, 100);
    widget.show();

    EXPECT_TRUE(widget.isVisible());
    EXPECT_EQ(widget.size(), QSize(200, 100));
}

TEST(TestSetup, TemporaryDirectoryIsWritable)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    QFile file(dir.filePath("probe.txt"));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    EXPECT_EQ(file.write("ok"), 2);
}
