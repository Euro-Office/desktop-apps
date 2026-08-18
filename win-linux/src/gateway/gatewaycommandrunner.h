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

        // Backs the gateway.connect meta-command (gatewayserver.cpp) -- a pure
        // resolver, not an "open" operation: matches CAscApplicationManager::GetViewByUrl
        // (which itself checks a view's GetUrl()/GetOriginalUrl()/GetUrlAsLocal()) after
        // normalizing `path` the same way CAscApplicationManagerWrapper::handleInputCmd
        // does before a view's local-file URL is ever set, so an already-open or
        // freshly-opened view for this file is actually found. Returns -1 if no view
        // matches -- the caller (a client that itself launched
        // `DesktopEditors <path>`, relying on SingleApplication to either cold-start or
        // forward-and-open-a-new-tab in the already-running instance either way) is
        // expected to poll this in a bounded loop rather than this method opening
        // anything itself, so gateway.connect stays a single CDP-free, side-effect-free
        // call like gateway.listCommands, not a second way to open documents.
        int ResolveViewIdByPath(const QString& path) const;

    private:
        CAscApplicationManager* m_manager;
        int m_nextMessageId = 1;
    };
}

#endif // GATEWAY_GATEWAYCOMMANDRUNNER_H
