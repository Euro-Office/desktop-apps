#ifndef GATEWAY_GATEWAYCOMMANDRUNNER_H
#define GATEWAY_GATEWAYCOMMANDRUNNER_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <functional>

#include "gatewaytypes.h"

class CAscApplicationManager;

namespace Gateway
{
    // The one call both the CLI and the external wire protocol are thin shells around
    // (see cdp-gateway-cli-plan.md "Scope of this document" in gateway-test-case-designs.md
    // and §1/§2 of the plan). Owns: allowlist lookup, scope-schema validation, target
    // resolution via the app's existing view map, and driving CDP's Runtime.evaluate.
    class GatewayCommandRunner : public QObject
    {
        Q_OBJECT
    public:
        explicit GatewayCommandRunner(CAscApplicationManager* manager, QObject* parent = nullptr);

        // Synchronous from the caller's point of view — internally drives the CDP
        // WebSocket round-trip via a nested event loop, matching the plan's per-command
        // test design (§A of gateway-test-case-designs.md expects Execute() to return
        // a Result, not take a callback). `targetViewId` selects which open document to
        // run against, using CAscApplicationManager's own view map (m_mapViews) —
        // never CDP target URL/title matching, per plan §0.
        Result Execute(const QString& command, const QJsonObject& scope, int targetViewId);

    private:
        // Resolves `targetViewId` to that view's CDP WebSocket debugger URL.
        //
        // KNOWN GAP (flagged, not silently assumed solved): CEF exposes one
        // --remote-debugging-port for the whole process, listing all open browser
        // targets on http://127.0.0.1:<port>/json. Correlating a specific CDP target
        // in that list back to our own `m_mapViews` entry needs a stable, non-URL,
        // non-title key. Candidates to verify against the vendored CEF sources before
        // the Word §B1 build/deploy gate (cdp-gateway-cli-plan.md §6): whether CEF's
        // per-target "id" in the /json listing is derivable from CefBrowser::GetIdentifier(),
        // or whether CefBrowserHost's own DevTools message API
        // (SendDevToolsMessage/AddDevToolsMessageObserver) is a better in-process fit
        // than round-tripping through the external port at all. Not resolved here —
        // ResolveTargetWebSocketUrl currently returns Result::Failure(TargetNotFound)
        // unconditionally so this gap fails loudly (test case A5) rather than silently
        // matching the wrong document.
        QString ResolveTargetWebSocketUrl(int targetViewId, Error& outErrorIfNotFound);

        CAscApplicationManager* m_manager;
    };
}

#endif // GATEWAY_GATEWAYCOMMANDRUNNER_H
