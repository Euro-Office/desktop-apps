// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vitest/config';

export default defineConfig({
    test: {
        environment: 'jsdom',
        include: ['**/*.test.js'],
        exclude: ['node_modules/**'],
        // npx vitest run --coverage. The page scripts are outside this
        // folder, hence allowExternal; the LCOV report names them relative to
        // the repository root, as the CI coverage check expects.
        coverage: {
            provider: 'v8',
            include: ['**/loginpage/src/**'],
            allowExternal: true,
            reporter: ['text-summary', ['lcov', { projectRoot: fileURLToPath(new URL('../../..', import.meta.url)) }]],
        },
    },
});
