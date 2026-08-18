# eo-mcp

Thin MCP server wrapping the DesktopEditors gateway (see
[`../../src/gateway/`](../../src/gateway/), [`gateway-api-reference.md`](../../../../gateway-api-reference.md),
and the design doc at `~/repos/eo-mcp-service-plan.md`). Not part of the DesktopEditors
build — a standalone Node package, spawned by an MCP host over stdio.

## Setup

```bash
cd tools/eo-mcp
npm install
```

## Running

```bash
npm start
# or, once installed globally / linked:
eo-mcp
```

Configure your MCP host to spawn `node <path-to-this-dir>/src/index.js` (or the `eo-mcp`
bin) over stdio. No configuration/env vars needed — it reads the gateway's auth token
from `$XDG_RUNTIME_DIR/eo-gateway-<uid>.token`, same as `eo-ctl`.

## Tools

- **`gateway_connect(file)`** → `{targetViewId}`. Resolves `file` to a stable id,
  opening it (launching DesktopEditors, or opening a new tab in an already-running
  instance) if not already open. Idempotent — call again for a file already opened
  earlier in the conversation to get the same id back. Call once per file before
  `gateway_call`.
- **`gateway_call(command, scope, targetViewId)`** → command result, or an MCP tool
  error on failure. See `gateway-api-reference.md` for every command's `scope` shape.
- **`gateway_list_commands()`** → array of every registered command name.

## Testing

```bash
npm test
```

Runs `test/gatewayClient.test.js` via Node's built-in test runner (`node --test`) —
no live gateway or DesktopEditors process required:

- `sendRequest`/`callCommand`/`listCommands` are tested against a real
  in-process `net.Server` speaking the gateway's exact one-shot wire protocol (genuine
  integration coverage of the wire format, not a mock of it).
- `connectAndResolveViewId` is pure logic with every dependency injected as a fake,
  covering the same branches as `tools/eo-ctl/tests/connectlogic_test.cpp` (cold start,
  already-open, forward-and-poll, timeout) — see `gatewayClient.js`'s header comment
  for why the two are the same algorithm in two languages.
