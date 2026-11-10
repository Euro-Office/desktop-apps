/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "cwinsparkle.h"
#include "cascapplicationmanagerwrapper.h"
#include <QCoreApplication>
#include <QMetaObject>
#include <winsparkle.h>

namespace {
bool started = false;

// Both run on WinSparkle's thread
int __cdecl canShutdown()
{
    return 1;   // closing the windows asks about unsaved documents
}

void __cdecl shutdownRequest()
{
    // The installer is about to run; quit so it can replace the files
    QMetaObject::invokeMethod(qApp, [] {
        AscAppManager::closeAppWindows();
    }, Qt::QueuedConnection);
}
}

void WinSparkle::init()
{
    // Name and registry key come from editors.exe's VERSIONINFO
    win_sparkle_set_appcast_url(EO_UPDATE_FEED_URL);
    // e.g. 9.3.8rc.1; FileVersion has no room for release candidates
    if (EO_UPDATE_VERSION[0])
        win_sparkle_set_app_build_version(EO_UPDATE_VERSION);
    if (!win_sparkle_set_eddsa_public_key(EO_UPDATE_PUBLIC_KEY))
        return; // invalid key: never install unverified updates
    win_sparkle_set_can_shutdown_callback(&canShutdown);
    win_sparkle_set_shutdown_request_callback(&shutdownRequest);
    win_sparkle_init();
    started = true;
}

void WinSparkle::checkWithUi()
{
    if (started)
        win_sparkle_check_update_with_ui();
}

void WinSparkle::cleanup()
{
    if (started)
        win_sparkle_cleanup();
}
