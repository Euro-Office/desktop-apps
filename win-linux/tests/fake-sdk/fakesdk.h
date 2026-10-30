/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef FAKESDK_H
#define FAKESDK_H

#include <string>
#include <vector>
#include "applicationmanager_events.h"

class CCefView;

// Test controls of the fake desktop-sdk the unit test build links instead of
// the real one. The fake records what the app asks the SDK to do and lets a
// test send SDK events to the app; everything else does nothing and returns
// neutral values. Like the real SDK, functions that take an event release it.
namespace FakeSdk
{
    // Arguments of the calls the app made to the SDK function `name`, oldest
    // first, e.g. calls("CCefView::load") -> {L"https://..."}. Events are
    // recorded with their type, e.g. calls("CAscApplicationManager::Apply")
    // -> {L"42"}.
    std::vector<std::wstring> calls(const std::string& name);
    void clearCalls();

    // Sends an event to the listener the app registered with
    // SetEventListener(), as the SDK does; the listener releases it.
    void sendEvent(NSEditorApi::CAscCefMenuEvent* event);

    // A view the app created through the SDK, nullptr if there is none.
    CCefView* view(int id);

    // What CCefViewEditor::GetFileFormat() returns for `path`
    // (AVS_OFFICESTUDIO_FILE_*); 0, unknown, by default.
    void setFileFormat(const std::wstring& path, int format);

    // Forgets recorded calls, file formats and user settings.
    void reset();

    // Used by the fake itself
    void record(const std::string& name, const std::wstring& arg = L"");
}

#endif // FAKESDK_H
