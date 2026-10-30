/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "fakesdk.h"
#include "fakesdk_p.h"

namespace FakeSdk
{
    State& state()
    {
        static State s;
        return s;
    }

    std::vector<std::wstring> calls(const std::string& name)
    {
        std::vector<std::wstring> args;
        for (const auto& call : state().calls) {
            if (call.first == name)
                args.push_back(call.second);
        }
        return args;
    }

    void clearCalls()
    {
        state().calls.clear();
    }

    void sendEvent(NSEditorApi::CAscCefMenuEvent* event)
    {
        if (state().listener)
            state().listener->OnEvent(event);
        else if (event)
            event->Release();
    }

    CCefView* view(int id)
    {
        auto it = state().views.find(id);
        return it == state().views.end() ? nullptr : it->second;
    }

    void setFileFormat(const std::wstring& path, int format)
    {
        state().fileFormats[path] = format;
    }

    void reset()
    {
        state().calls.clear();
        state().fileFormats.clear();
        state().userSettings.clear();
    }

    void record(const std::string& name, const std::wstring& arg)
    {
        state().calls.emplace_back(name, arg);
    }
}
