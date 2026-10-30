/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include <QApplication>

// GoogleTest entry point for suites that need a QApplication (widgets, event
// loop, QSettings). Defaults to the offscreen platform so the tests run
// without a display; set QT_QPA_PLATFORM to override it when debugging.
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    return RUN_ALL_TESTS();
}
