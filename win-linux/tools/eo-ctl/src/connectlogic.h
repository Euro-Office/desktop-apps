#ifndef EOCTL_CONNECTLOGIC_H
#define EOCTL_CONNECTLOGIC_H

#include <functional>

namespace EoCtl
{
    // Pure polling logic behind `eo-ctl connect <file>`, extracted out of main.cpp so
    // it's unit-testable against fakes rather than a live gateway/DesktopEditors
    // process -- same "no business logic in main, dependency-inject for testability"
    // shape as GatewayCommandRunner::Execute.
    //
    // The gateway's `gateway.connect` meta command (gatewayserver.cpp) is a pure
    // resolver: it never opens anything itself, so ANY view resolution has to be
    // driven from here. The one thing that actually opens a file is launching
    // `DesktopEditors <file>` as a subprocess -- SingleApplication (main.cpp,
    // cascapplicationmanagerwrapper.cpp) makes that work identically whether or not an
    // instance is already running: cold start opens `file` directly as the initial
    // document; a second launch forwards the path via sendMessage and the existing
    // instance opens it as a new tab via handleInputCmd. So this function never needs
    // to distinguish those cases beyond "was a launch already implied by getting the
    // socket up" -- it just launches once if resolution comes back empty, then polls.
    //
    // `socketAlreadyExists`: whether the gateway socket existed before this call
    //     (skips the redundant "ensure socket running" launch below if so).
    // `ensureSocketRunning`: launches DesktopEditors and blocks (bounded) until the
    //     socket exists; returns false on failure/timeout. Only called when
    //     `socketAlreadyExists` is false.
    // `resolveViewId`: calls gateway.connect{path: file} and returns the resulting
    //     targetViewId, or -1 if the file isn't open yet (per gatewayserver.cpp's
    //     contract -- not an error).
    // `launchForFileOpen`: launches `DesktopEditors <file>` again to trigger
    //     SingleApplication's forward-and-open-a-new-tab path. Only called when
    //     `socketAlreadyExists` is true and the first resolveViewId() came back
    //     unresolved.
    // `sleepMs`: injected so tests don't actually sleep.
    //
    // Returns the resolved targetViewId, or -1 if it never resolved within maxWaitMs.
    int ConnectAndResolveViewId(
        bool socketAlreadyExists,
        const std::function<bool()>& ensureSocketRunning,
        const std::function<int()>& resolveViewId,
        const std::function<void()>& launchForFileOpen,
        const std::function<void(int)>& sleepMs,
        int maxWaitMs = 30000,
        int pollIntervalMs = 200);
}

#endif // EOCTL_CONNECTLOGIC_H
