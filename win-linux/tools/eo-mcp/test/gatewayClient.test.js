// Automated tests for gatewayClient.js. Two kinds:
//  - sendRequest/callCommand/listCommands: a real net.Server in-process, speaking the
//    exact one-shot protocol GatewayServer implements, so these are genuine
//    integration tests of the wire format -- not mocks of it.
//  - connectAndResolveViewId: pure logic, every dependency injected as a fake,
//    mirroring win-linux/tools/eo-ctl/tests/connectlogic_test.cpp's cases exactly
//    (same algorithm, ported to JS -- see gatewayClient.js's header comment on why).

import { test } from 'node:test';
import assert from 'node:assert/strict';
import net from 'node:net';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';

import { sendRequest, callCommand, listCommands, connectAndResolveViewId } from '../src/gatewayClient.js';

// --- fake gateway server helper -------------------------------------------------

async function withFakeGateway(handler, testFn) {
    const socketPath = path.join(os.tmpdir(), `eo-mcp-test-${process.pid}-${Date.now()}-${Math.random().toString(36).slice(2)}.sock`);
    const server = net.createServer((socket) => {
        let buffer = '';
        socket.on('data', (chunk) => {
            buffer += chunk.toString('utf8');
            const newlineIndex = buffer.indexOf('\n');
            if (newlineIndex === -1) return;
            const request = JSON.parse(buffer.slice(0, newlineIndex));
            const response = handler(request);
            socket.write(JSON.stringify(response) + '\n');
        });
    });

    await new Promise((resolve) => server.listen(socketPath, resolve));
    try {
        await testFn(socketPath);
    } finally {
        server.close();
        fs.rmSync(socketPath, { force: true });
    }
}

// --- sendRequest / callCommand / listCommands -----------------------------------

test('sendRequest: frames the request and returns the parsed response', async () => {
    let receivedRequest;
    await withFakeGateway(
        (request) => {
            receivedRequest = request;
            return { id: request.id, ok: true, result: 42 };
        },
        async (socketPath) => {
            const response = await sendRequest('word.getTitle', { a: 1 }, 3, {
                socketPathOverride: socketPath,
                tokenOverride: 'test-token',
            });
            assert.equal(response.ok, true);
            assert.equal(response.result, 42);
            assert.equal(receivedRequest.command, 'word.getTitle');
            assert.deepEqual(receivedRequest.scope, { a: 1 });
            assert.equal(receivedRequest.targetViewId, 3);
            assert.equal(receivedRequest.auth, 'test-token');
        },
    );
});

test('callCommand: resolves with result on ok:true', async () => {
    await withFakeGateway(
        () => ({ id: 'x', ok: true, result: { title: 'Q3 Report' } }),
        async (socketPath) => {
            const result = await callCommand('word.getTitle', {}, 1, {
                socketPathOverride: socketPath,
                tokenOverride: 't',
            });
            assert.deepEqual(result, { title: 'Q3 Report' });
        },
    );
});

test('callCommand: throws with .code set on ok:false', async () => {
    await withFakeGateway(
        () => ({ id: 'x', ok: false, error: { code: 'SCHEMA_INVALID', message: 'bad scope' } }),
        async (socketPath) => {
            await assert.rejects(
                () => callCommand('word.setTitle', {}, 1, { socketPathOverride: socketPath, tokenOverride: 't' }),
                (err) => {
                    assert.equal(err.code, 'SCHEMA_INVALID');
                    assert.equal(err.message, 'bad scope');
                    return true;
                },
            );
        },
    );
});

test('listCommands: calls gateway.listCommands with an empty scope', async () => {
    let receivedRequest;
    await withFakeGateway(
        (request) => {
            receivedRequest = request;
            return { id: request.id, ok: true, result: ['word.getTitle', 'pdf.getAllFields'] };
        },
        async (socketPath) => {
            const names = await listCommands({ socketPathOverride: socketPath, tokenOverride: 't' });
            assert.deepEqual(names, ['word.getTitle', 'pdf.getAllFields']);
            assert.equal(receivedRequest.command, 'gateway.listCommands');
            assert.deepEqual(receivedRequest.scope, {});
        },
    );
});

test('sendRequest: rejects on timeout when the server never responds', async () => {
    await withFakeGateway(
        () => null, // handler return value is ignored -- server below never writes back
        async () => {
            const socketPath = path.join(os.tmpdir(), `eo-mcp-test-noresponse-${Date.now()}.sock`);
            const server = net.createServer(() => {}); // accepts, never writes anything back
            await new Promise((resolve) => server.listen(socketPath, resolve));
            try {
                await assert.rejects(
                    () => sendRequest('word.getTitle', {}, 1, {
                        socketPathOverride: socketPath,
                        tokenOverride: 't',
                        timeoutMs: 50,
                    }),
                    /timed out/,
                );
            } finally {
                server.close();
                fs.rmSync(socketPath, { force: true });
            }
        },
    );
});

// --- connectAndResolveViewId (pure logic, ported 1:1 from connectlogic_test.cpp) --

test('connectAndResolveViewId: already open, socket exists -> resolves immediately, no launch', async () => {
    let launchCalls = 0;
    let resolveCalls = 0;

    const viewId = await connectAndResolveViewId({
        socketAlreadyExists: true,
        ensureSocketRunning: async () => true, // must not be called
        resolveViewId: async () => { resolveCalls++; return 7; },
        launchForFileOpen: () => { launchCalls++; },
        sleepMs: async () => {},
    });

    assert.equal(viewId, 7);
    assert.equal(resolveCalls, 1);
    assert.equal(launchCalls, 0);
});

test('connectAndResolveViewId: cold start, no socket -> launches and resolves, no forward-launch', async () => {
    let ensureCalled = false;
    let launchForFileOpenCalls = 0;

    const viewId = await connectAndResolveViewId({
        socketAlreadyExists: false,
        ensureSocketRunning: async () => { ensureCalled = true; return true; },
        resolveViewId: async () => 3,
        launchForFileOpen: () => { launchForFileOpenCalls++; },
        sleepMs: async () => {},
    });

    assert.equal(viewId, 3);
    assert.equal(ensureCalled, true);
    assert.equal(launchForFileOpenCalls, 0);
});

test('connectAndResolveViewId: cold start, ensureSocketRunning fails -> -1', async () => {
    const viewId = await connectAndResolveViewId({
        socketAlreadyExists: false,
        ensureSocketRunning: async () => false,
        resolveViewId: async () => 5, // must not be reached
        launchForFileOpen: () => {},
        sleepMs: async () => {},
    });

    assert.equal(viewId, -1);
});

test('connectAndResolveViewId: socket exists, file not open yet -> launches for file open, then polls', async () => {
    let launchCalls = 0;
    let resolveCalls = 0;

    const viewId = await connectAndResolveViewId({
        socketAlreadyExists: true,
        ensureSocketRunning: async () => true,
        resolveViewId: async () => { resolveCalls++; return resolveCalls < 3 ? -1 : 9; },
        launchForFileOpen: () => { launchCalls++; },
        sleepMs: async () => {},
        maxWaitMs: 10000,
        pollIntervalMs: 100,
    });

    assert.equal(viewId, 9);
    assert.equal(launchCalls, 1);
    assert.equal(resolveCalls, 3);
});

test('connectAndResolveViewId: never resolves -> times out, -1', async () => {
    let sleepCalls = 0;

    const viewId = await connectAndResolveViewId({
        socketAlreadyExists: true,
        ensureSocketRunning: async () => true,
        resolveViewId: async () => -1,
        launchForFileOpen: () => {},
        sleepMs: async () => { sleepCalls++; },
        maxWaitMs: 1000,
        pollIntervalMs: 200,
    });

    assert.equal(viewId, -1);
    assert.equal(sleepCalls, 5); // 1000/200
});

test('connectAndResolveViewId: sleepMs receives the poll interval', async () => {
    const sleptFor = [];

    await connectAndResolveViewId({
        socketAlreadyExists: true,
        ensureSocketRunning: async () => true,
        resolveViewId: async () => -1,
        launchForFileOpen: () => {},
        sleepMs: async (ms) => { sleptFor.push(ms); },
        maxWaitMs: 600,
        pollIntervalMs: 150,
    });

    assert.equal(sleptFor.length, 4); // 600/150
    for (const ms of sleptFor) assert.equal(ms, 150);
});
