#include "connectlogic.h"

namespace EoCtl
{
    int ConnectAndResolveViewId(
        bool socketAlreadyExists,
        const std::function<bool()>& ensureSocketRunning,
        const std::function<int()>& resolveViewId,
        const std::function<void()>& launchForFileOpen,
        const std::function<void(int)>& sleepMs,
        int maxWaitMs,
        int pollIntervalMs)
    {
        if (!socketAlreadyExists)
        {
            if (!ensureSocketRunning())
                return -1;
            // Cold start: the instance we just launched opened `file` itself as its
            // initial document -- resolve below rather than launching again.
        }

        int viewId = resolveViewId();
        if (viewId != -1)
            return viewId;

        if (socketAlreadyExists)
            launchForFileOpen();

        for (int waited = 0; waited < maxWaitMs; waited += pollIntervalMs)
        {
            sleepMs(pollIntervalMs);
            viewId = resolveViewId();
            if (viewId != -1)
                return viewId;
        }

        return -1;
    }
}
