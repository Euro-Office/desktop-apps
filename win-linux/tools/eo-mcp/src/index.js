#!/usr/bin/env node
// eo-mcp -- thin MCP wrapper around the DesktopEditors gateway. Three tools, mirroring
// eo-ctl exactly (see ~/repos/eo-mcp-service-plan.md §2/§4 for why this is the
// "thin/generic" shape rather than one MCP tool per gateway command): gateway_connect,
// gateway_call, gateway_list_commands. No business logic lives here -- see
// gatewayClient.js for the actual protocol/polling logic, which is what's unit-tested.

import { McpServer } from '@modelcontextprotocol/sdk/server/mcp.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import { z } from 'zod';
import { callCommand, listCommands, connectFile } from './gatewayClient.js';

const server = new McpServer({
    name: 'eo-mcp',
    version: '0.1.0',
});

server.tool(
    'gateway_connect',
    'Resolve a file path to a stable targetViewId for the DesktopEditors gateway, ' +
    'opening the file (launching DesktopEditors, or opening a new tab in an already-' +
    'running instance) if it is not already open. Idempotent -- safe to call again ' +
    'for a file already opened earlier in the conversation; returns the same id. ' +
    'Call this once per file before gateway_call.',
    {
        file: z.string().describe('Path to the document to open/resolve.'),
    },
    async ({ file }) => {
        const targetViewId = await connectFile(file);
        if (targetViewId === -1) {
            return {
                isError: true,
                content: [{ type: 'text', text: `timed out resolving a view for ${file}` }],
            };
        }
        return {
            content: [{ type: 'text', text: JSON.stringify({ targetViewId }) }],
        };
    },
);

server.tool(
    'gateway_call',
    'Run one allowlisted DesktopEditors gateway command against an already-open ' +
    'document (obtained via gateway_connect). See gateway-api-reference.md for every ' +
    'command\'s scope fields, return shape, and examples.',
    {
        command: z.string().describe('Gateway command name, e.g. "word.setTitle".'),
        scope: z.record(z.any()).default({}).describe('Command-specific parameters.'),
        targetViewId: z.number().int().describe('The id returned by gateway_connect.'),
    },
    async ({ command, scope, targetViewId }) => {
        try {
            const result = await callCommand(command, scope, targetViewId);
            return {
                content: [{ type: 'text', text: JSON.stringify(result ?? null) }],
            };
        } catch (err) {
            return {
                isError: true,
                content: [{ type: 'text', text: `${err.code ?? 'ERROR'}: ${err.message}` }],
            };
        }
    },
);

server.tool(
    'gateway_list_commands',
    'List every currently-registered gateway command name.',
    {},
    async () => {
        const names = await listCommands();
        return {
            content: [{ type: 'text', text: JSON.stringify(names) }],
        };
    },
);

const transport = new StdioServerTransport();
await server.connect(transport);
