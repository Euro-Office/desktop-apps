// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

// The start page scripts are plain browser scripts that index.html loads one
// after another and that share globals (jQuery, window.utils, ...). This runs
// them unchanged in the test's jsdom window, in the order given.

import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

// (jsdom replaces the global URL, so no new URL(..., import.meta.url) here)
const loginpage = join(dirname(fileURLToPath(import.meta.url)), '..');

export function loadScripts(...paths) {
    for (const path of paths) {
        const code = readFileSync(join(loginpage, path), 'utf8');
        // indirect eval runs the script in the global scope, like a <script>
        (0, eval)(`${code}\n//# sourceURL=${path}`);
    }
}

// jQuery and src/utils.js, which most scripts build on
export function loadUtils() {
    loadScripts('vendor/jquery/jquery.min.js', 'src/utils.js');
    return window.utils;
}
