// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

// src/panelcloud.js + src/panelconnect.js: a Nextcloud account in the Clouds list
// shows its files in the start page, and its documents open in editor tabs.

import { beforeAll, beforeEach, describe, expect, it, vi } from 'vitest';
import { loadScripts } from './load.js';

const PORTAL = 'https://cloud.example.com';

let commands, requests, server, portals, cloud;

const ocs = data => JSON.stringify({ocs: {meta: {status: 'ok'}, data}});

const davItem = (href, id, dir, lockowner) => `<d:response><d:href>/remote.php/dav/files/alice${href}</d:href>
    <d:propstat><d:prop><d:resourcetype>${dir ? '<d:collection/>' : ''}</d:resourcetype><oc:fileid>${id}</oc:fileid>
    ${lockowner ? `<nc:lock>1</nc:lock><nc:lock-owner-displayname>${lockowner}</nc:lock-owner-displayname>` : ''}</d:prop>
    <d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response>`;

const folder = items => `<?xml version="1.0"?><d:multistatus xmlns:d="DAV:" xmlns:oc="http://owncloud.org/ns" xmlns:nc="http://nextcloud.org/ns">${items.join('')}</d:multistatus>`;

function fakeDesktopEditor() {
    return {
        execCommand: (cmd, param) => commands.push([cmd, param]),
        GetExternalClouds: () => [{
            provider: 'nextcloud', name: 'Nextcloud', editorPage: '/apps/eurooffice/',
            auth: {type: 'loginflow2'}, files: 'nextcloud', check: {url: 'status.php'},
            icons: {themeLight: {connectionsList: './assets/listicon.svg'}, themeDark: {connectionsList: './assets/listicon_dark.svg'}},
        }, {
            provider: 'owncloud', name: 'ownCloud', check: {url: 'status.php'},
        }, {
            // logs in with the system browser, but has no Nextcloud file APIs
            provider: 'othercloud', name: 'Other cloud', auth: {type: 'loginflow2'}, check: {url: 'status.php'},
        }],
        GetCryptoMode: () => 0,
        GetSupportCryptoModes: () => [],
        setCookie: () => setTimeout(() => window.on_set_cookie && window.on_set_cookie()),
        checkAuth: () => {},
        OpenFilenameDialog: (filter, multi, cb) => cb(server.pickedFile),
    };
}

function fakeRequest(opts) {
    requests.push(opts);
    const path = opts.url.substring(PORTAL.length);
    const answer = server.answer && server.answer(opts, path);
    if ( answer && answer.offline ) { opts.error({status: 'error'}); return; }

    let res = answer;
    if ( !res && path.startsWith('/ocs/v2.php/cloud/user?') ) res = {responseStatus: 200, responseText: ocs({id: 'alice', 'display-name': 'Alice A.'})};
    if ( !res && path.startsWith('/ocs/v2.php/cloud/capabilities?') ) res = {responseStatus: 200, responseText: ocs({capabilities: {eurooffice: {
        editor: PORTAL + '/apps/eurooffice/{fileId}', formats: {docx: {type: 'word', edit: true}}}}})};
    if ( !res && opts.method == 'PROPFIND' && path == '/remote.php/dav/files/alice/' )
        res = {responseStatus: 207, responseText: folder([davItem('/', 1, true), davItem('/Docs/', 2, true),
                                                          davItem('/Report.docx', 3), davItem('/photo%3Cb%3E.jpg', 4)])};
    if ( !res && opts.method == 'PROPFIND' && path == '/remote.php/dav/files/alice/Docs/' )
        res = {responseStatus: 207, responseText: folder([davItem('/Docs/', 2, true), davItem('/Docs/Plan.docx', 5, false, 'Bob B.')])};
    if ( !res ) res = {responseStatus: 404};
    opts.complete(Object.assign({status: 'success', responseHeaders: {}}, res), 'success');
}

const flush = () => new Promise(r => setTimeout(r, 20));
const $panel = () => $('.action-panel.cloud');
const rows = () => $panel().find('.file-list-body .row .name').map((i, el) => el.textContent).get();
const sent = cmd => commands.filter(c => c[0] == cmd).map(c => JSON.parse(c[1]));

beforeAll(() => {
    commands = [];
    window.AscDesktopEditor = fakeDesktopEditor();
    window.AscSimpleRequest = {createRequest: opts => fakeRequest(opts)};
    window.isSvgIcons = false;
    loadScripts('vendor/jquery/jquery.min.js', 'vendor/bootstrap/dropdown.js',
                'vendor/bootstrap-select/js/bootstrap-select.js', 'src/utils.js', 'src/locale.js', 'src/events.js',
                'src/sdk.js', 'src/model.js', 'src/menu.js', 'src/base.js', 'src/dialogbase.js',
                'src/panelconnect.js', 'src/nextcloud.js', 'src/panelcloud.js', 'src/dialogconnect.js');
    window.sdk = window.AscDesktopEditor;
    HTMLDialogElement.prototype.showModal ||= function() { this.setAttribute('open', ''); };
    window.selectAction = action => { $('.action-panel').hide(); $(`.action-panel.${action}`).show(); };

    $('body').append(`<div class="action-panel"></div><div id="placeholder">
                        <div class="main-column tool-menu"></div><div class="main-column col-center"></div>
                        <div id="idx-sidebar-portals"></div></div>`);
    window.app = {controller: {}};
    window.app.controller.portals = portals = new ControllerPortals({placeholder: '#idx-sidebar-portals'}).init();
    window.app.controller.cloud = cloud = new ControllerCloud().init();
});

beforeEach(() => {
    commands.length = 0;
    requests = [];
    server = {};
    window.NextcloudClient.forget(PORTAL);
});

describe('Clouds list', () => {
    it('shows the files of a Nextcloud account instead of its web page', async () => {
        const open = vi.spyOn(cloud, 'open');
        window.app.controller.portals.collection().events.click.notify(new PortalModel({portal: PORTAL, provider: 'nextcloud', user: 'alice', email: ''}));
        await flush();

        expect(open).toHaveBeenCalledWith({portal: PORTAL, provider: 'nextcloud', user: 'alice'});
        expect(sent('portal:open')).toEqual([]);
        expect($panel().css('display')).not.toBe('none');
        open.mockRestore();
    });

    it('keeps the web page for clouds without the Nextcloud file APIs', async () => {
        // the browser login alone doesn't make the start page browse a cloud
        expect(cloud.handles('nextcloud')).toBe(true);
        expect(cloud.handles('othercloud')).toBe(false);
        expect(cloud.handles('owncloud')).toBe(false);
        expect(cloud.handles('unknown')).toBe(false);

        window.app.controller.portals.collection().events.click.notify(new PortalModel({portal: 'https://other.example.com', provider: 'othercloud', user: 'bob', email: ''}));
        await flush();
        expect(sent('portal:open')).toMatchObject([{portal: 'https://other.example.com', provider: 'othercloud'}]);
    });

    it('keeps the web page for other clouds', async () => {
        window.app.controller.portals.collection().events.click.notify(new PortalModel({portal: 'https://own.example.com', provider: 'owncloud', user: 'bob', email: ''}));
        await flush();

        expect(sent('portal:open')).toMatchObject([{portal: 'https://own.example.com', provider: 'owncloud'}]);
    });

    it('offers the web page in the context menu of browsed clouds', () => {
        const menuItem = () => $('#pp-menu-portals a').filter((i, el) => el.textContent.includes('Open web interface')).closest('li');
        const collection = window.app.controller.portals.collection();

        collection.events.contextmenu.notify(new PortalModel({portal: PORTAL, provider: 'nextcloud', user: 'alice', email: ''}), {clientX: 0, clientY: 0});
        expect(menuItem().css('display')).not.toBe('none');

        collection.events.contextmenu.notify(new PortalModel({portal: 'https://own.example.com', provider: 'owncloud', user: 'bob', email: ''}), {clientX: 0, clientY: 0});
        expect(menuItem().css('display')).toBe('none');
        Menu.closeAll();
    });

    it('logs in again when the cloud rejects the credentials', async () => {
        server.answer = () => ({responseStatus: 401});
        cloud.open({portal: PORTAL, provider: 'nextcloud', user: 'alice'});
        await flush();

        expect($panel().find('.cloud-message').text()).toContain('did not accept the login');
        expect($panel().find('.cloud-login').css('display')).not.toBe('none');

        const reconnect = vi.fn();
        window.CommonEvents.on('portal:reconnect', reconnect);
        $panel().find('.cloud-login').click();
        expect(reconnect).toHaveBeenCalledWith({portal: PORTAL, provider: 'nextcloud', user: 'alice'});
        // the connect dialog for this cloud
        expect($('dialog #auth-portal').val()).toBe('cloud.example.com');
        $('dialog .tool.close').trigger('click');
        await flush();

        // the dialog tries https first, so a plain http cloud keeps its scheme
        window.CommonEvents.fire('portal:reconnect', [{portal: 'http://plain.example.com', provider: 'nextcloud'}]);
        expect($('dialog #auth-portal').val()).toBe('http://plain.example.com');
        $('dialog .tool.close').trigger('click');
        await flush();
    });
});

describe('browsing', () => {
    beforeEach(async () => {
        cloud.open({portal: PORTAL, provider: 'nextcloud', user: 'alice'});
        await flush();
    });

    it('lists the home folder with the account', () => {
        expect(rows()).toEqual(['Docs', 'photo<b>.jpg', 'Report.docx']);
        expect($panel().find('.cloud-title').text()).toBe('cloud.example.com');
        expect($panel().find('.cloud-user').text()).toBe('Signed in as Alice A.');
        // file names are text, not markup
        expect($panel().find('.file-list-body b').length).toBe(0);
    });

    it('opens folders and goes back with the breadcrumbs', async () => {
        $panel().find('.file-list-body .row').eq(0).click();
        await flush();
        expect(rows()).toEqual(['Plan.docx']);
        expect($panel().find('.cloud-crumb').map((i, el) => el.textContent).get()).toEqual(['Home', 'Docs']);

        $panel().find('.cloud-crumb').eq(0).click();
        await flush();
        expect(rows()).toEqual(['Docs', 'photo<b>.jpg', 'Report.docx']);
    });

    it('opens documents in the live editing session', () => {
        $panel().find('.file-list-body .row').eq(2).click();
        expect(sent('cloud:open')).toEqual([{
            url: PORTAL + '/apps/eurooffice/3?placement=desktop', name: 'Report.docx', portal: PORTAL}]);
    });

    it('shows other files on the web page', () => {
        $panel().find('.file-list-body .row').eq(1).click();
        expect(sent('cloud:open')).toEqual([]);
        expect(sent('portal:open')).toEqual([{portal: PORTAL, provider: 'nextcloud', entrypage: '/index.php/f/4'}]);
    });

    it('tells when the cloud is unreachable', async () => {
        server.answer = () => ({offline: true});
        $panel().find('.cloud-tab[data-view=recent]').click();
        await flush();
        expect($panel().find('.cloud-message').text()).toContain("Can't reach cloud.example.com");
        expect($panel().find('.cloud-retry').css('display')).not.toBe('none');
    });

    it('creates documents with a free name and opens them', async () => {
        server.answer = (opts, path) => path.startsWith('/ocs/v2.php/apps/files/api/v1/templates/create') &&
                    {responseStatus: 200, responseText: ocs({fileid: 9, basename: 'New document.docx'})};
        $panel().find('.cloud-new[data-type=docx]').click();
        await flush();

        expect(requests.find(r => r.url.includes('templates/create')).body).toContain('filePath=%2FNew%20document.docx');
        expect(sent('cloud:open')).toMatchObject([{url: PORTAL + '/apps/eurooffice/9?placement=desktop'}]);
    });

    it('uploads a local file and opens it', async () => {
        server.pickedFile = '/home/alice/Draft.docx';
        server.answer = (opts) => opts.method == 'PUT' && {responseStatus: 201, responseHeaders: {'oc-fileid': '00000011oc1'}};
        $panel().find('.cloud-upload').click();
        await flush();

        expect(requests.find(r => r.method == 'PUT')).toMatchObject({
            url: PORTAL + '/remote.php/dav/files/alice/Draft.docx', file: '/home/alice/Draft.docx'});
        expect(sent('cloud:open')).toMatchObject([{url: PORTAL + '/apps/eurooffice/11?placement=desktop', name: 'Draft.docx'}]);
    });

    it('searches the cloud', async () => {
        server.answer = (opts, path) => path.startsWith('/ocs/v2.php/search/providers/files/search') &&
                    {responseStatus: 200, responseText: ocs({entries: [{title: 'Q3.docx', subline: 'in Finance', attributes: {fileId: '21', path: '/Finance/Q3.docx'}}]})};
        $panel().find('.cloud-search').val('Q3').trigger($.Event('keydown', {key: 'Enter'}));
        await flush();

        expect(rows()).toEqual(['Q3.docx']);
        expect($panel().find('.col-location').eq(1).text()).toBe('in Finance');
    });

    it('shows the account with the provider icon and the web page link', () => {
        const $icons = $panel().find('.cloud-account-icon img');
        expect($icons.map((i, el) => el.getAttribute('src')).get()).toEqual([
            './providers/nextcloud/./assets/listicon.svg', './providers/nextcloud/./assets/listicon_dark.svg']);
        expect($panel().find('.cloud-user').text()).toBe('Signed in as Alice A.');

        $panel().find('.cloud-web').click();
        expect(sent('portal:open')).toEqual([{portal: PORTAL, provider: 'nextcloud', entrypage: '/'}]);
    });

    it('offers new documents and uploads only in folders', async () => {
        const visible = sel => $panel().find(sel).css('display') != 'none';
        expect(visible('.cloud-create')).toBe(true);
        expect($panel().find('.cloud-create .cloud-new').map((i, el) => $(el).data('type')).get()).toEqual(['docx', 'xlsx', 'pptx']);
        // no location column in a folder
        expect($panel().find('.cloud-files').hasClass('no-location')).toBe(true);

        server.answer = (opts, path) => path == '/remote.php/dav/' && {responseStatus: 207, responseText: folder([davItem('/Docs/Plan.docx', 5)])};
        $panel().find('.cloud-tab[data-view=recent]').click();
        await flush();
        expect(visible('.cloud-create')).toBe(false);
        expect($panel().find('.cloud-tab.selected').data('view')).toBe('recent');
        expect($panel().find('.cloud-files').hasClass('no-location')).toBe(false);
        expect($panel().find('.file-list-body .col-location').text()).toBe('/Docs/');
    });

    it('marks locked files', async () => {
        $panel().find('.file-list-body .row').eq(0).click();
        await flush();
        const $lock = $panel().find('.file-list-body .row .cloud-lock');
        expect($lock.text()).toBe('Locked');
        expect($lock.attr('title')).toBe('Locked by Bob B.');
    });

    it('shows an empty folder, an empty search and no recent files', async () => {
        const state = () => [$panel().find('.cloud-state-title').text(), $panel().find('.cloud-state').css('display') != 'none',
                             $panel().find('.cloud-files').css('display') != 'none'];

        server.answer = (opts, path) => opts.method == 'PROPFIND' && path == '/remote.php/dav/files/alice/' &&
                    {responseStatus: 207, responseText: folder([davItem('/', 1, true)])};
        $panel().find('.cloud-crumb').eq(0).click();
        await flush();
        expect(state()).toEqual(['This folder is empty', true, false]);

        server.answer = (opts, path) => path == '/remote.php/dav/' && {responseStatus: 207, responseText: folder([])};
        $panel().find('.cloud-tab[data-view=recent]').click();
        await flush();
        expect(state()).toEqual(['No recent files', true, false]);

        server.answer = (opts, path) => path.startsWith('/ocs/v2.php/search/') && {responseStatus: 200, responseText: ocs({entries: []})};
        $panel().find('.cloud-search').val('zzz').trigger($.Event('keydown', {key: 'Enter'}));
        await flush();
        expect(state()).toEqual(['No files found', true, false]);
        expect($panel().find('.cloud-results').text()).toBe('Results for “zzz”');
    });

    it('searches while typing and clears the search', async () => {
        vi.useFakeTimers();
        try {
            server.answer = (opts, path) => path.startsWith('/ocs/v2.php/search/') &&
                        {responseStatus: 200, responseText: ocs({entries: [{title: 'Q3.docx', attributes: {fileId: '21', path: '/Finance/Q3.docx'}}]})};
            // one letter is too short, then a pause after the last key press
            $panel().find('.cloud-search').val('Q').trigger('input');
            await vi.advanceTimersByTimeAsync(500);
            expect(requests.some(r => r.url.includes('/search/'))).toBe(false);

            $panel().find('.cloud-search').val('Q3').trigger('input');
            await vi.advanceTimersByTimeAsync(100);
            $panel().find('.cloud-search').val('Q3 ').trigger('input');
            await vi.advanceTimersByTimeAsync(500);
            expect(requests.filter(r => r.url.includes('/search/')).map(r => r.url.replace(/.*term=/, '').replace(/&.*/, ''))).toEqual(['Q3']);
            expect(rows()).toEqual(['Q3.docx']);
            expect($panel().find('.cloud-search-clear').css('display')).not.toBe('none');

            $panel().find('.cloud-search-clear').click();
            await vi.advanceTimersByTimeAsync(50);
            expect($panel().find('.cloud-search').val()).toBe('');
            expect($panel().find('.cloud-tab.selected').data('view')).toBe('files');
            expect(rows()).toEqual(['Docs', 'photo<b>.jpg', 'Report.docx']);
        } finally {
            vi.useRealTimers();
        }
    });

    it('tells while it creates or uploads, and when the file exists', async () => {
        let finish;
        server.answer = (opts, path) => {
            if ( path.startsWith('/ocs/v2.php/apps/files/api/v1/templates/create') ) {
                finish = () => opts.complete({status: 'success', responseStatus: 200, responseHeaders: {},
                                              responseText: ocs({fileid: 9, basename: 'New spreadsheet.xlsx'})}, 'success');
                return {pending: true};
            }
        };
        const realRequest = window.AscSimpleRequest.createRequest;
        window.AscSimpleRequest.createRequest = opts => {
            const answer = server.answer(opts, opts.url.substring(PORTAL.length));
            if ( answer && answer.pending ) { requests.push(opts); return; }
            realRequest(opts);
        };
        try {
            $panel().find('.cloud-new[data-type=xlsx]').click();
            await flush();
            expect($panel().find('.cloud-progress').text()).toBe('Creating the document…');
            finish();
            await flush();
            expect($panel().find('.cloud-progress').text()).toBe('');
            expect(sent('cloud:open')).toMatchObject([{name: 'New spreadsheet.xlsx'}]);
        } finally {
            window.AscSimpleRequest.createRequest = realRequest;
        }

        server.pickedFile = 'C:\\Users\\alice\\Report.docx';
        server.answer = (opts) => opts.method == 'PUT' && {responseStatus: 412};
        $panel().find('.cloud-upload').click();
        await flush();
        expect($panel().find('.cloud-progress').text()).toBe('A file named Report.docx already exists in this folder.');
        // the files stay
        expect(rows()).toEqual(['Docs', 'photo<b>.jpg', 'Report.docx']);
    });

    it('opens files with the keyboard', () => {
        $panel().find('.file-list-body .row').eq(2).trigger($.Event('keydown', {key: 'Enter'}));
        expect(sent('cloud:open')).toMatchObject([{name: 'Report.docx'}]);
    });
});

describe('browser login', () => {
    it('shows the files of the new account instead of opening its web page', async () => {
        server.answer = (opts, path) => {
            if ( path == '/status.php' ) return {responseStatus: 200, responseText: JSON.stringify({installed: true, productname: 'Nextcloud'})};
            if ( path == '/index.php/login/v2' ) return {responseStatus: 200, responseText: JSON.stringify({
                    poll: {token: 'tok', endpoint: PORTAL + '/login/v2/poll'}, login: PORTAL + '/login/v2/flow/x'})};
            if ( path == '/login/v2/poll' ) return {responseStatus: 200, responseText: JSON.stringify({
                    server: PORTAL, loginName: 'alice', appPassword: 'secret'})};
        };
        const open = vi.spyOn(cloud, 'open');

        vi.useFakeTimers();
        try {
            PortalsStore.keep({portal: 'https://other.example.com', provider: 'nextcloud', user: 'bob', email: ''});
            $('<button class="login">').appendTo('body').trigger('click');
            await vi.advanceTimersByTimeAsync(0);

            const $dlg = $('dialog');
            $dlg.find('select').val('nextcloud');
            $dlg.find('#auth-portal').val('cloud.example.com');
            $dlg.find('#btn-next').trigger('click');
            await vi.advanceTimersByTimeAsync(5000);
        } finally {
            vi.useRealTimers();
        }

        expect(sent('auth:browser:store')).toMatchObject([{portal: PORTAL, provider: 'nextcloud', open: false}]);
        expect(open).toHaveBeenCalledWith({portal: PORTAL, provider: 'nextcloud', user: 'alice'});
        open.mockRestore();
    });
});
