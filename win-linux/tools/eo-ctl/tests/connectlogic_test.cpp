// Unit tests for EoCtl::ConnectAndResolveViewId (connectlogic.h/.cpp) -- pure logic,
// no live gateway/DesktopEditors process needed; every dependency is injected as a
// fake. See connectlogic.h's header comment for the rationale behind each branch.

#include "../src/connectlogic.h"

#include <cstdio>
#include <string>
#include <vector>
#include <functional>

namespace
{
    bool Test_AlreadyOpen_SocketExists_ResolvesImmediately_NoLaunch()
    {
        int launchCalls = 0;
        int resolveCalls = 0;

        const int viewId = EoCtl::ConnectAndResolveViewId(
            /*socketAlreadyExists=*/true,
            /*ensureSocketRunning=*/[]() -> bool { return true; }, // must not be called
            /*resolveViewId=*/[&resolveCalls]() -> int { ++resolveCalls; return 7; },
            /*launchForFileOpen=*/[&launchCalls]() { ++launchCalls; },
            /*sleepMs=*/[](int) {});

        return viewId == 7 && resolveCalls == 1 && launchCalls == 0;
    }

    bool Test_ColdStart_NoSocket_LaunchesAndResolves()
    {
        bool ensureCalled = false;
        int launchForFileOpenCalls = 0;

        const int viewId = EoCtl::ConnectAndResolveViewId(
            /*socketAlreadyExists=*/false,
            /*ensureSocketRunning=*/[&ensureCalled]() -> bool { ensureCalled = true; return true; },
            /*resolveViewId=*/[]() -> int { return 3; },
            /*launchForFileOpen=*/[&launchForFileOpenCalls]() { ++launchForFileOpenCalls; },
            /*sleepMs=*/[](int) {});

        // Cold start: the just-launched instance already opened the file itself --
        // launchForFileOpen (the "forward to running instance" path) must NOT also fire.
        return viewId == 3 && ensureCalled && launchForFileOpenCalls == 0;
    }

    bool Test_ColdStart_EnsureSocketRunningFails_ReturnsMinusOne()
    {
        const int viewId = EoCtl::ConnectAndResolveViewId(
            /*socketAlreadyExists=*/false,
            /*ensureSocketRunning=*/[]() -> bool { return false; },
            /*resolveViewId=*/[]() -> int { return 5; }, // must not be reached
            /*launchForFileOpen=*/[]() {},
            /*sleepMs=*/[](int) {});

        return viewId == -1;
    }

    bool Test_SocketExists_FileNotOpenYet_LaunchesForFileOpen_ThenPolls()
    {
        int launchCalls = 0;
        int resolveCalls = 0;

        const int viewId = EoCtl::ConnectAndResolveViewId(
            /*socketAlreadyExists=*/true,
            /*ensureSocketRunning=*/[]() -> bool { return true; },
            /*resolveViewId=*/[&resolveCalls]() -> int {
                ++resolveCalls;
                return resolveCalls < 3 ? -1 : 9; // resolves on the 3rd attempt
            },
            /*launchForFileOpen=*/[&launchCalls]() { ++launchCalls; },
            /*sleepMs=*/[](int) {},
            /*maxWaitMs=*/10000,
            /*pollIntervalMs=*/100);

        return viewId == 9 && launchCalls == 1 && resolveCalls == 3;
    }

    bool Test_NeverResolves_TimesOut_ReturnsMinusOne()
    {
        int sleepCalls = 0;

        const int viewId = EoCtl::ConnectAndResolveViewId(
            /*socketAlreadyExists=*/true,
            /*ensureSocketRunning=*/[]() -> bool { return true; },
            /*resolveViewId=*/[]() -> int { return -1; },
            /*launchForFileOpen=*/[]() {},
            /*sleepMs=*/[&sleepCalls](int) { ++sleepCalls; },
            /*maxWaitMs=*/1000,
            /*pollIntervalMs=*/200);

        // 1000/200 = 5 poll iterations, each preceded by one sleep.
        return viewId == -1 && sleepCalls == 5;
    }

    bool Test_SleepMs_ReceivesThePollInterval()
    {
        std::vector<int> sleptFor;

        EoCtl::ConnectAndResolveViewId(
            /*socketAlreadyExists=*/true,
            /*ensureSocketRunning=*/[]() -> bool { return true; },
            /*resolveViewId=*/[]() -> int { return -1; },
            /*launchForFileOpen=*/[]() {},
            /*sleepMs=*/[&sleptFor](int ms) { sleptFor.push_back(ms); },
            /*maxWaitMs=*/600,
            /*pollIntervalMs=*/150);

        for (int ms : sleptFor)
            if (ms != 150) return false;
        return sleptFor.size() == 4; // 600/150
    }
}

int main()
{
    const std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"AlreadyOpen_SocketExists_ResolvesImmediately_NoLaunch", Test_AlreadyOpen_SocketExists_ResolvesImmediately_NoLaunch},
        {"ColdStart_NoSocket_LaunchesAndResolves", Test_ColdStart_NoSocket_LaunchesAndResolves},
        {"ColdStart_EnsureSocketRunningFails_ReturnsMinusOne", Test_ColdStart_EnsureSocketRunningFails_ReturnsMinusOne},
        {"SocketExists_FileNotOpenYet_LaunchesForFileOpen_ThenPolls", Test_SocketExists_FileNotOpenYet_LaunchesForFileOpen_ThenPolls},
        {"NeverResolves_TimesOut_ReturnsMinusOne", Test_NeverResolves_TimesOut_ReturnsMinusOne},
        {"SleepMs_ReceivesThePollInterval", Test_SleepMs_ReceivesThePollInterval},
    };

    int failures = 0;
    for (const auto& test : tests)
    {
        const bool passed = test.second();
        std::printf("[%s] %s\n", passed ? "PASS" : "FAIL", test.first.c_str());
        if (!passed) ++failures;
    }

    std::printf("%zu tests, %d failed\n", tests.size(), failures);
    return failures == 0 ? 0 : 1;
}
