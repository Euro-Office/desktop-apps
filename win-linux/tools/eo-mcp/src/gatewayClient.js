// Thin client for the DesktopEditors gateway (see cdp-gateway-cli-plan.md and
// gateway-api-reference.md). Talks directly to the Unix socket -- does not shell out
// to eo-ctl -- using the exact one-shot protocol GatewayServer implements
// (win-linux/src/gateway/gatewayserver.cpp): connect, write one newline-terminated
// JSON request, read one newline-terminated JSON response, disconnect.

import net from 'node:net';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawn } from 'node:child_process';

export function socketPath(uid = process.getuid()) {
    const runtimeDir = process.env.XDG_RUNTIME_DIR || os.tmpdir();
    return path.join(runtimeDir, `eo-gateway-${uid}.sock`);
}

export function tokenPath(uid = process.getuid()) {
    const runtimeDir = process.env.XDG_RUNTIME_DIR || os.tmpdir();
    return path.join(runtimeDir, `eo-gateway-${uid}.token`);
}

export function socketExists(uid = process.getuid()) {
    return fs.existsSync(socketPath(uid));
}

function readToken(uid = process.getuid()) {
    return fs.readFileSync(tokenPath(uid), 'utf8').trim();
}

// Low-level one-shot request/response. `socketPathOverride`/`tokenOverride` exist
// purely so tests can point this at a fake in-process server instead of a real
// gateway -- see test/gatewayClient.test.js.
export function sendRequest(command, scope, targetViewId, options = {}) {
    const {
        uid,
        timeoutMs = 10000,
        socketPathOverride,
        tokenOverride,
    } = options;

    return new Promise((resolve, reject) => {
        let target;
        let token;
        try {
            target = socketPathOverride ?? socketPath(uid);
            token = tokenOverride ?? readToken(uid);
        } catch (err) {
            reject(err);
            return;
        }

        const socket = net.createConnection(target);
        let buffer = '';
        let settled = false;

        const timer = setTimeout(() => {
            if (settled) return;
            settled = true;
            socket.destroy();
            reject(new Error('gateway: no response (timed out)'));
        }, timeoutMs);

        socket.on('connect', () => {
            const request = {
                id: 'eo-mcp-1',
                command,
                scope: scope ?? {},
                targetViewId: targetViewId ?? -1,
                auth: token,
            };
            socket.write(JSON.stringify(request) + '\n');
        });

        socket.on('data', (chunk) => {
            if (settled) return;
            buffer += chunk.toString('utf8');
            const newlineIndex = buffer.indexOf('\n');
            if (newlineIndex === -1) return;

            settled = true;
            clearTimeout(timer);
            const line = buffer.slice(0, newlineIndex);
            socket.end();
            try {
                resolve(JSON.parse(line));
            } catch (err) {
                reject(new Error(`gateway: malformed response: ${err.message}`));
            }
        });

        socket.on('error', (err) => {
            if (settled) return;
            settled = true;
            clearTimeout(timer);
            reject(err);
        });
    });
}

// Throws on ok:false, mapping the gateway's {code, message} error onto a JS Error
// (with .code set) rather than returning it as a value -- matches the tool-error
// convention MCP tool handlers are expected to use.
export async function callCommand(command, scope, targetViewId, options = {}) {
    const response = await sendRequest(command, scope, targetViewId, options);
    if (!response.ok) {
        const error = new Error(response.error?.message ?? 'gateway command failed');
        error.code = response.error?.code ?? 'UNKNOWN';
        throw error;
    }
    return response.result;
}

export function listCommands(options = {}) {
    return callCommand('gateway.listCommands', {}, -1, options);
}

// Pure polling algorithm behind connectFile() below -- every dependency is injected
// so this is unit-testable without a real socket/process. Deliberately the same
// shape as EoCtl::ConnectAndResolveViewId (win-linux/tools/eo-ctl/src/connectlogic.h)
// for the same reason documented there: gateway.connect never opens anything itself
// (see gatewayserver.cpp), so launching DesktopEditors <file> is what actually opens
// the document -- SingleApplication makes that work identically whether it's a cold
// start or a forward to an already-running instance.
export async function connectAndResolveViewId(deps) {
    const {
        socketAlreadyExists,
        ensureSocketRunning,
        resolveViewId,
        launchForFileOpen,
        sleepMs,
        maxWaitMs = 30000,
        pollIntervalMs = 200,
    } = deps;

    if (!socketAlreadyExists) {
        const ok = await ensureSocketRunning();
        if (!ok) return -1;
        // Cold start: the instance we just launched opened the file itself as its
        // initial document -- resolve below rather than launching again.
    }

    let viewId = await resolveViewId();
    if (viewId !== -1) return viewId;

    if (socketAlreadyExists) {
        await launchForFileOpen();
    }

    for (let waited = 0; waited < maxWaitMs; waited += pollIntervalMs) {
        await sleepMs(pollIntervalMs);
        viewId = await resolveViewId();
        if (viewId !== -1) return viewId;
    }

    return -1;
}

// Real-world wiring for connectAndResolveViewId: spawns DesktopEditors and polls the
// real socket. This function itself is intentionally thin (just wiring real IO) --
// the interesting branch logic it delegates to is what's unit-tested.
export async function connectFile(file, options = {}) {
    const { uid } = options;
    const absoluteFile = path.resolve(file);

    const spawnEditor = () => {
        const child = spawn('DesktopEditors', [absoluteFile], { detached: true, stdio: 'ignore' });
        child.unref();
    };

    const resolveViewId = async () => {
        const result = await callCommand('gateway.connect', { path: absoluteFile }, -1, options);
        return result.targetViewId;
    };

    const ensureSocketRunning = async () => {
        spawnEditor();
        const deadline = Date.now() + 30000;
        while (Date.now() < deadline) {
            if (socketExists(uid)) return true;
            await new Promise((resolve) => setTimeout(resolve, 200));
        }
        return false;
    };

    return connectAndResolveViewId({
        socketAlreadyExists: socketExists(uid),
        ensureSocketRunning,
        resolveViewId,
        launchForFileOpen: spawnEditor,
        sleepMs: (ms) => new Promise((resolve) => setTimeout(resolve, ms)),
    });
}
