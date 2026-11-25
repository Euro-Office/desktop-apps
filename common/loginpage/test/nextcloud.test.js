// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

// src/nextcloud.js: the start page's requests to a Nextcloud server and how it
// reads the answers.

import { beforeAll, beforeEach, describe, expect, it } from 'vitest';
import { loadScripts } from './load.js';

const PORTAL = 'https://cloud.example.com/nc';

let requests, server;

const multistatus = responses => `<?xml version="1.0"?>
<d:multistatus xmlns:d="DAV:" xmlns:oc="http://owncloud.org/ns" xmlns:nc="http://nextcloud.org/ns">
${responses.join('\n')}
</d:multistatus>`;

const davItem = ({href, dir = false, id, lock, owner, modified = 'Tue, 24 Nov 2026 10:00:00 GMT'}) => `
<d:response>
  <d:href>${href}</d:href>
  <d:propstat>
    <d:prop>
      <d:getlastmodified>${modified}</d:getlastmodified>
      <d:getetag>"etag${id}"</d:getetag>
      <d:resourcetype>${dir ? '<d:collection/>' : ''}</d:resourcetype>
      <oc:fileid>${id}</oc:fileid>
      <oc:permissions>RGDNVW</oc:permissions>
      <oc:size>1234</oc:size>
      ${lock ? `<nc:lock>1</nc:lock><nc:lock-owner-displayname>${owner}</nc:lock-owner-displayname>` : ''}
    </d:prop>
    <d:status>HTTP/1.1 200 OK</d:status>
  </d:propstat>
  <d:propstat>
    <d:prop><d:getcontenttype/></d:prop>
    <d:status>HTTP/1.1 404 Not Found</d:status>
  </d:propstat>
</d:response>`;

const ocs = data => JSON.stringify({ocs: {meta: {status: 'ok'}, data}});

function fakeServer() {
    return {
        user: {id: 'alice.id', 'display-name': 'Alice'},
        capabilities: {eurooffice: {
            editor: PORTAL + '/index.php/apps/eurooffice/{fileId}',
            formats: {docx: {type: 'word', edit: true}, odt: {type: 'word', edit: false}},
        }},
        answer: undefined,      // (opts) => response, for the test's request
    };
}

function fakeRequest(opts) {
    requests.push(opts);
    const path = opts.url.substring(PORTAL.length);
    let res;
    if ( server.answer ) res = server.answer(opts, path);
    if ( !res && path.startsWith('/ocs/v2.php/cloud/user?') ) res = {responseStatus: 200, responseText: ocs(server.user)};
    if ( !res && path.startsWith('/ocs/v2.php/cloud/capabilities?') ) res = {responseStatus: 200, responseText: ocs({capabilities: server.capabilities})};
    if ( !res ) res = {responseStatus: 404, responseText: ''};

    if ( res.offline ) opts.error({status: 'error', statusCode: -105});
    else opts.complete(Object.assign({status: 'success', responseHeaders: {}}, res), 'success');
}

beforeAll(() => {
    window.AscSimpleRequest = {createRequest: opts => fakeRequest(opts)};
    loadScripts('src/nextcloud.js');
});

beforeEach(() => {
    requests = [];
    server = fakeServer();
});

const client = () => new window.NextcloudClient.Client(PORTAL + '/');

describe('connect', () => {
    it('asks for the user id and the Nextcloud Office capability', async () => {
        const c = await client().connect();
        expect(c.user).toEqual({id: 'alice.id', displayName: 'Alice'});
        expect(c.office.editor).toBe(PORTAL + '/index.php/apps/eurooffice/{fileId}');
        expect(requests.every(r => r.headers['OCS-APIRequest'] == 'true')).toBe(true);
        // the page never sends credentials, it asks the native side to add them
        expect(requests.some(r => r.headers.Authorization)).toBe(false);
        expect(requests.every(r => r.portalAuth === true)).toBe(true);
    });

    it('works without the eurooffice app', async () => {
        server.capabilities = {};
        const c = await client().connect();
        expect(c.office).toBe(null);
        expect(c.editorUrl(42)).toBe(PORTAL + '/index.php/apps/eurooffice/42?placement=desktop');
        expect(c.canOpen('Report.DOCX')).toBe(true);
        expect(c.canOpen('photo.jpg')).toBe(false);
    });

    it('reports rejected credentials', async () => {
        server.answer = () => ({responseStatus: 401});
        await expect(client().connect()).rejects.toEqual({status: 'auth', responseStatus: 401});
    });

    it('reports an unreachable server', async () => {
        server.answer = () => ({offline: true});
        await expect(client().connect()).rejects.toMatchObject({status: 'offline'});
    });
});

describe('list', () => {
    it('reads one folder level, folders first, without the folder itself', async () => {
        const root = '/nc/remote.php/dav/files/alice.id';
        server.answer = (opts, path) => path.startsWith('/remote.php/dav/files/') && {
            responseStatus: 207,
            responseText: multistatus([
                davItem({href: `${root}/Team%20docs/`, dir: true, id: '00000010ocinst'}),
                davItem({href: `${root}/Team%20docs/z.docx`, id: '00000012ocinst', lock: true, owner: 'Bob'}),
                davItem({href: `${root}/Team%20docs/Plans/`, dir: true, id: '00000013ocinst'}),
                davItem({href: `${root}/Team%20docs/a%20b.odt`, id: '00000011ocinst'}),
            ]),
        };

        const items = await client().list('/Team docs');

        const propfind = requests.find(r => r.method == 'PROPFIND');
        expect(propfind.url).toBe(PORTAL + '/remote.php/dav/files/alice.id/Team%20docs/');
        expect(propfind.headers.Depth).toBe('1');
        expect(propfind.body).toContain('<oc:fileid/>');

        expect(items.map(i => i.name)).toEqual(['Plans', 'a b.odt', 'z.docx']);
        expect(items[0]).toMatchObject({path: '/Team docs/Plans/', isdir: true, fileid: 13});
        expect(items[1]).toMatchObject({path: '/Team docs/a b.odt', isdir: false, fileid: 11, size: 1234, locked: false});
        expect(items[2]).toMatchObject({locked: true, lockowner: 'Bob'});
        expect(items[1].modified).toBe(Date.parse('Tue, 24 Nov 2026 10:00:00 GMT'));
    });
});

describe('recent and search', () => {
    it('asks the server for recently changed files, newest first', async () => {
        server.answer = (opts, path) => path == '/remote.php/dav/' && {
            responseStatus: 207,
            responseText: multistatus([davItem({href: '/nc/remote.php/dav/files/alice.id/x.docx', id: '5'})]),
        };

        const items = await client().recent();

        const search = requests.find(r => r.method == 'SEARCH');
        expect(search.body).toContain('<d:href>/files/alice.id/</d:href>');
        expect(search.body).toContain('<d:descending/>');
        expect(search.body).toContain('<d:nresults>50</d:nresults>');
        expect(items).toMatchObject([{name: 'x.docx', path: '/x.docx', fileid: 5}]);
    });

    it('uses the unified file search', async () => {
        server.answer = (opts, path) => path.startsWith('/ocs/v2.php/search/providers/files/search') && {
            responseStatus: 200,
            responseText: ocs({entries: [
                {title: 'Budget.xlsx', subline: 'in Finance', icon: 'icon-xlsx', attributes: {fileId: '77', path: '/Finance/Budget.xlsx'}},
                {title: 'no id', attributes: {}},
            ]}),
        };

        const items = await client().search('bud get');

        expect(requests.at(-1).url).toContain('term=bud%20get');
        expect(items).toEqual([{path: '/Finance/Budget.xlsx', name: 'Budget.xlsx', isdir: false, fileid: 77, descr: 'in Finance'}]);
    });
});

describe('upload and create', () => {
    it('uploads a local file without replacing an existing one', async () => {
        server.answer = (opts) => opts.method == 'PUT' && {responseStatus: 201, responseHeaders: {'oc-fileid': '00000099ocinst'}};

        const file = await client().upload('/home/alice/Report 1.docx', '/Team/');

        const put = requests.find(r => r.method == 'PUT');
        expect(put.url).toBe(PORTAL + '/remote.php/dav/files/alice.id/Team/Report%201.docx');
        expect(put.file).toBe('/home/alice/Report 1.docx');
        expect(put.body).toBeUndefined();
        expect(put.headers['If-None-Match']).toBe('*');
        expect(file).toEqual({path: '/Team/Report 1.docx', name: 'Report 1.docx', fileid: 99});
    });

    it('takes Windows paths', async () => {
        server.answer = (opts) => opts.method == 'PUT' && {responseStatus: 201, responseHeaders: {'oc-fileid': '7'}};
        const file = await client().upload('C:\\Users\\alice\\a.xlsx', '/');
        expect(file.name).toBe('a.xlsx');
    });

    it('reports a file that exists already', async () => {
        server.answer = (opts) => opts.method == 'PUT' && {responseStatus: 412};
        await expect(client().upload('/tmp/a.docx', '/')).rejects.toMatchObject({status: 'conflict'});
    });

    it('creates a document with the Files templates API', async () => {
        server.answer = (opts, path) => path.startsWith('/ocs/v2.php/apps/files/api/v1/templates/create') && {
            responseStatus: 200, responseText: ocs({fileid: 123, basename: 'New document.docx'}),
        };

        const file = await client().create('/Team/', 'New document.docx');

        const post = requests.at(-1);
        expect(post.method).toBe('POST');
        expect(post.body).toBe('filePath=%2FTeam%2FNew%20document.docx&templatePath=&templateType=user');
        expect(file).toEqual({path: '/Team/New document.docx', name: 'New document.docx', fileid: 123});
    });
});

describe('editor', () => {
    it('opens the files the server edits in Nextcloud Office', async () => {
        const c = await client().connect();
        expect(c.editorUrl(42)).toBe(PORTAL + '/index.php/apps/eurooffice/42?placement=desktop');
        expect(c.canOpen('a.odt')).toBe(true);
        expect(c.canOpen('a.pptx')).toBe(false);
    });

    it('keeps one client per cloud', () => {
        const a = window.NextcloudClient.get(PORTAL);
        expect(window.NextcloudClient.get('HTTPS://Cloud.Example.com/nc/')).toBe(a);
        window.NextcloudClient.forget(PORTAL);
        expect(window.NextcloudClient.get(PORTAL)).not.toBe(a);
    });
});
