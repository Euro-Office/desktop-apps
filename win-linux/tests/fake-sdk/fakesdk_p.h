/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef FAKESDK_P_H
#define FAKESDK_P_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "fakesdk.h"

// State shared by the fake SDK classes
namespace FakeSdk
{
    struct State
    {
        std::vector<std::pair<std::string, std::wstring>> calls;
        NSEditorApi::CAscCefMenuEventListener* listener = nullptr;
        std::map<int, CCefView*> views;
        int lastViewId = 0;
        std::map<std::wstring, int> fileFormats;
        std::map<std::wstring, std::wstring> userSettings;
    };

    State& state();

    // Records an event the app passed to the SDK and releases it, as the SDK does.
    // Commands to the pages are also recorded as "<name>:command" -> "<command> <param>".
    inline void recordEvent(const std::string& name, NSEditorApi::CAscMenuEvent* event)
    {
        record(name, event ? std::to_wstring(event->m_nType) : L"");
        if (event && event->m_nType == ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS) {
            auto* data = static_cast<NSEditorApi::CAscExecCommandJS*>(event->m_pData);
            record(name + ":command", data->get_Command() + L" " + data->get_Param());
        }
        if (event)
            event->Release();
    }
}

#endif // FAKESDK_P_H
