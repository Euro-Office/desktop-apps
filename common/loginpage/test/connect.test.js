// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

// src/panelconnect.js + src/dialogconnect.js: a cloud connected with the
// system browser stays in the Clouds list, even when its page never reports
// "portal:login" (Nextcloud without the eurooffice app).

import { beforeAll, beforeEach, describe, expect, it, vi } from 'vitest';
import { loadScripts } from './load.js';

const PORTAL = 'https://cloud.example.com';

let commands, requests, controller;

function fakeDesktopEditor() {
    return {
        execCommand: (cmd, param) => commands.push([cmd, param]),
        GetExternalClouds: () => [{
            provider: 'nextcloud', name: 'Nextcloud', editorPage: '/apps/eurooffice/',
            auth: {type: 'loginflow2'}, check: {url: 'status.php'},
        }],
        GetCryptoMode: () => 0,
        GetSupportCryptoModes: () => [],
        setCookie: () => setTimeout(() => window.on_set_cookie && window.on_set_cookie()),
        checkAuth: () => {},
    };
}

// answers the start page's AscSimpleRequest calls, see BrowserAuth
function fakeRequest(opts) {
    requests.push(opts);
    if ( opts.url.endsWith('/status.php') )
        opts.complete({responseStatus: 200, responseText: JSON.stringify({installed: true, productname: 'Nextcloud'})}, 'success');
    else
    if ( opts.url.endsWith('/index.php/login/v2') )
        opts.complete({responseStatus: 200, responseText: JSON.stringify({
            poll: {token: 'tok', endpoint: PORTAL + '/login/v2/poll'}, login: PORTAL + '/login/v2/flow/x'})});
    else
    if ( opts.url.endsWith('/login/v2/poll') )
        opts.complete({responseStatus: 200, responseText: JSON.stringify({
            server: PORTAL, loginName: 'alice', appPassword: 'secret'})});
}

beforeAll(() => {
    commands = [];
    window.AscDesktopEditor = fakeDesktopEditor();
    window.AscSimpleRequest = {createRequest: opts => fakeRequest(opts)};
    window.isSvgIcons = false;
    loadScripts('vendor/jquery/jquery.min.js', 'vendor/bootstrap/dropdown.js',
                'vendor/bootstrap-select/js/bootstrap-select.js', 'src/utils.js', 'src/locale.js', 'src/events.js',
                'src/sdk.js', 'src/model.js', 'src/menu.js', 'src/base.js', 'src/dialogbase.js',
                'src/panelconnect.js', 'src/dialogconnect.js');
    window.sdk = window.AscDesktopEditor;
    HTMLDialogElement.prototype.showModal ||= function() { this.setAttribute('open', ''); };

    $('body').append('<div class="action-panel"></div><div id="sidebar"></div><div id="placeholder"></div>');
    controller = new ControllerPortals({}).init();
});

beforeEach(() => {
    localStorage.clear();
    commands = [];
    requests = [];
});

describe('connecting a cloud with the system browser', () => {
    it('keeps the cloud in the portal list without "portal:login" from its page', async () => {
        vi.useFakeTimers();
        try {
            // with a cloud in the list, "+" opens the connect dialog directly
            PortalsStore.keep({portal: 'https://other.example.com', provider: 'nextcloud', user: 'bob', email: ''});
            $('<button class="login">').appendTo('body').trigger('click');
            await vi.advanceTimersByTimeAsync(0);

            const $dlg = $('dialog');
            expect($dlg.length).toBe(1);
            $dlg.find('select').val('nextcloud');
            $dlg.find('#auth-portal').val('cloud.example.com');
            $dlg.find('#btn-next').trigger('click');

            // status check, flow start, then a poll that grants access
            await vi.advanceTimersByTimeAsync(5000);

            expect(commands.map(c => c[0])).toContain('auth:browser:store');
            expect(commands.map(c => c[0])).not.toContain('auth:outer');
            expect($('dialog').length).toBe(0);

            expect(PortalsStore.get({portal: PORTAL})).toEqual(
                {portal: PORTAL, provider: 'nextcloud', user: 'alice', email: ''});
            expect(controller.collection().find('name', 'cloud.example.com')).toBeTruthy();
        } finally {
            vi.useRealTimers();
        }
    });
});

describe('the connect dialog', () => {
    it('preselects the only cloud and translates the empty choice', async () => {
        vi.useFakeTimers();
        try {
            // "+" passes no provider; with one cloud it must not show bootstrap-select's "Nothing selected"
            PortalsStore.keep({portal: 'https://other.example.com', provider: 'nextcloud', user: 'bob', email: ''});
            $('<button class="login">').appendTo('body').trigger('click');
            await vi.advanceTimersByTimeAsync(0);

            const $combo = $('dialog select');
            expect($combo.val()).toBe('nextcloud');
            expect($combo.data('selectpicker').options.noneSelectedText).toBe(utils.Lang.txtSelectCloud);
            expect($('dialog').text()).not.toContain('Nothing selected');

            $('dialog .tool.close').trigger('click');
            await vi.advanceTimersByTimeAsync(0);
        } finally {
            vi.useRealTimers();
        }
    });
});
