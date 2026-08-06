#ifndef GATEWAY_GATEWAYCOMMANDRUNNER_H
#define GATEWAY_GATEWAYCOMMANDRUNNER_H

#include <QObject>
#include <QJsonObject>
#include <QString>

#include "gatewaytypes.h"

class CAscApplicationManager;

namespace Gateway
{
    // The one call both the CLI and the external wire protocol are thin shells around
    // (see cdp-gateway-cli-plan.md and gateway-test-case-designs.md's "Scope of this
    // document"). Owns: allowlist lookup, scope-schema validation, target resolution
    // via the app's existing view map (CAscApplicationManager::GetViewById), and
    // driving CDP in-process through CCefView::SendGatewayDevToolsMessage (see
    // desktop-sdk/ChromiumBasedEditors/lib/include/cefview.h) -- no external
    // --remote-debugging-port, no websocket; we already hold the exact CefBrowser via
    // the resolved view, so there is no CDP-target-correlation problem to solve.
    class GatewayCommandRunner : public QObject
    {
        Q_OBJECT
    public:
        explicit GatewayCommandRunner(CAscApplicationManager* manager, QObject* parent = nullptr);

        // Synchronous from the caller's point of view -- internally drives one
        // SendGatewayDevToolsMessage round trip via a nested event loop, matching the
        // plan's per-command test design (gateway-test-case-designs.md §A expects
        // Execute() to return a Result, not take a callback). `targetViewId` selects
        // which open document to run against, via CAscApplicationManager::GetViewById
        // -- never CDP target URL/title matching, per plan §0.
        Result Execute(const QString& command, const QJsonObject& scope, int targetViewId);

    private:
        CAscApplicationManager* m_manager;
        int m_nextMessageId = 1;
    };
}

#endif // GATEWAY_GATEWAYCOMMANDRUNNER_H
