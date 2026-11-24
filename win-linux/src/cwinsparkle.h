/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef CWINSPARKLE_H
#define CWINSPARKLE_H

// In-app updates through WinSparkle. The appcast URL and EdDSA public key are
// set at build time (EO_UPDATE_FEED_URL, EO_UPDATE_PUBLIC_KEY in CMake).
namespace WinSparkle
{
    void init();            // starts background checks (WinSparkle asks the user first)
    void checkWithUi();     // "Check for updates" from the About panel
    void cleanup();
}

#endif // CWINSPARKLE_H
