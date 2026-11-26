// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

/*
*   Nextcloud client of the start page: browses, searches, uploads and creates files
*   of a cloud the user logged in to with the system browser (BrowserAuth).
*
*   Requests go through AscSimpleRequest; the native side adds the cloud's
*   Authorization header, so the app password never reaches this page. All methods
*   return promises. Errors are {status, responseStatus}: status 'auth' when the
*   cloud rejects the credentials (log in again), 'offline' when it can't be
*   reached, 'conflict' when a file exists already, 'error' otherwise.
*/
window.NextcloudClient = (function() {
    'use strict';

    const NS_DAV = 'DAV:',
          NS_OC = 'http://owncloud.org/ns',
          NS_NC = 'http://nextcloud.org/ns';

    const PROPS = `<d:prop>
            <d:displayname/><d:getcontenttype/><d:getlastmodified/><d:getcontentlength/>
            <d:getetag/><d:resourcetype/><oc:fileid/><oc:permissions/><oc:size/>
            <nc:lock/><nc:lock-owner-displayname/>
        </d:prop>`;

    const RECENT_DAYS = 90,
          RECENT_LIMIT = 50;

    function _encode_path(path) {
        return path.split('/').map(encodeURIComponent).join('/');
    }

    function _join(dir, name) {
        return (dir.endsWith('/') ? dir : dir + '/') + name;
    }

    function _xml_escape(s) {
        return String(s).replace(/[<>&'"]/g, c => ({'<':'&lt;', '>':'&gt;', '&':'&amp;', "'":'&apos;', '"':'&quot;'})[c]);
    }

    function _json(text) {
        try { return JSON.parse(text); } catch (e) { return undefined; }
    }

    // "00000123ocabcdef" (OC-FileId header) -> 123
    function _numeric_id(id) {
        const n = parseInt(String(id || ''), 10);
        return isNaN(n) ? undefined : n;
    }

    var Client = function(portal) {
        this.portal = portal.replace(/\/+$/, '');
        this.user = undefined;      // {id, displayName}
        this.office = undefined;    // eurooffice capability, null without the app
    };

    Client.prototype.request = function(opts) {
        return new Promise((resolve, reject) => {
            if ( !window.AscSimpleRequest || !window.AscSimpleRequest.createRequest ) {
                reject({status: 'error'});
                return;
            }

            const headers = Object.assign({'OCS-APIRequest': 'true'}, opts.headers);
            const req = {
                url: this.portal + opts.path,
                method: opts.method || 'GET',
                headers: headers,
                // the native side adds the cloud's Authorization header
                portalAuth: true,
                timeout: opts.timeout || 30000,
                complete: e => {
                    const status = e.responseStatus;
                    if ( status >= 200 && status < 300 )
                        resolve({status: status, text: e.responseText || '', headers: e.responseHeaders || {}});
                    else
                    if ( status == 401 )
                        reject({status: 'auth', responseStatus: status});
                    else
                    if ( status == 412 || status == 409 && opts.conflict )
                        reject({status: 'conflict', responseStatus: status});
                    else reject({status: 'error', responseStatus: status});
                },
                // no response: unreachable, timed out or the request was refused
                error: e => reject({status: 'offline', responseStatus: e && e.responseStatus}),
            };
            if ( opts.body !== undefined ) req.body = opts.body;
            if ( opts.file !== undefined ) req.file = opts.file;

            window.AscSimpleRequest.createRequest(req);
        });
    };

    Client.prototype.ocs = function(path, opts = {}) {
        const sep = path.includes('?') ? '&' : '?';
        return this.request(Object.assign({}, opts, {path: path + sep + 'format=json'}))
            .then(res => {
                const obj = _json(res.text);
                if ( !obj || !obj.ocs ) throw {status: 'error'};
                return obj.ocs.data;
            });
    };

    // the user id, which can differ from the login name, and what the server offers
    Client.prototype.connect = function() {
        if ( this.user ) return Promise.resolve(this);

        return Promise.all([
            this.ocs('/ocs/v2.php/cloud/user'),
            this.ocs('/ocs/v2.php/cloud/capabilities'),
        ]).then(([user, caps]) => {
            if ( !user || !user.id ) throw {status: 'auth'};

            this.user = {id: user.id, displayName: user['display-name'] || user.displayname || user.id};
            this.office = caps && caps.capabilities && caps.capabilities.eurooffice || null;
            return this;
        });
    };

    Client.prototype.davRoot = function() {
        return '/remote.php/dav/files/' + encodeURIComponent(this.user.id);
    };

    Client.prototype._parse = function(text, basepath) {
        const doc = new DOMParser().parseFromString(text, 'application/xml');
        const items = [];
        const root = this.davRoot();

        for (const response of doc.getElementsByTagNameNS(NS_DAV, 'response')) {
            const href = response.getElementsByTagNameNS(NS_DAV, 'href')[0];
            if ( !href ) continue;

            // "/remote.php/dav/files/alice/Documents/a%20b.docx" -> "/Documents/a b.docx"
            let path = decodeURIComponent(href.textContent.replace(/^https?:\/\/[^\/]+/, ''));
            const at = path.indexOf(decodeURIComponent(root));
            if ( at < 0 ) continue;
            path = path.substring(at + decodeURIComponent(root).length) || '/';

            let prop;
            for (const propstat of response.getElementsByTagNameNS(NS_DAV, 'propstat')) {
                const status = propstat.getElementsByTagNameNS(NS_DAV, 'status')[0];
                if ( !status || / 200 /.test(status.textContent) ) {
                    prop = propstat.getElementsByTagNameNS(NS_DAV, 'prop')[0];
                    break;
                }
            }
            if ( !prop ) continue;

            const value = (ns, name) => {
                const el = prop.getElementsByTagNameNS(ns, name)[0];
                return el ? el.textContent : '';
            };

            const isdir = prop.getElementsByTagNameNS(NS_DAV, 'collection').length > 0;
            const trimmed = path.replace(/\/+$/, '');
            const item = {
                path: isdir ? trimmed + '/' : trimmed,
                name: trimmed.substring(trimmed.lastIndexOf('/') + 1),
                isdir: isdir,
                fileid: _numeric_id(value(NS_OC, 'fileid')),
                etag: value(NS_DAV, 'getetag'),
                mime: value(NS_DAV, 'getcontenttype'),
                size: parseInt(value(NS_OC, 'size') || value(NS_DAV, 'getcontentlength') || '0', 10),
                modified: Date.parse(value(NS_DAV, 'getlastmodified')) || 0,
                permissions: value(NS_OC, 'permissions'),
                locked: value(NS_NC, 'lock') == '1',
                lockowner: value(NS_NC, 'lock-owner-displayname'),
            };

            if ( basepath !== undefined && item.path == basepath ) continue;    // the folder itself
            items.push(item);
        }
        return items;
    };

    // one folder level, folders first
    Client.prototype.list = function(path) {
        const dir = path.endsWith('/') ? path : path + '/';
        return this.connect().then(() => this.request({
            path: this.davRoot() + _encode_path(dir),
            method: 'PROPFIND',
            headers: {'Depth': '1', 'Content-Type': 'application/xml; charset=utf-8'},
            body: `<?xml version="1.0"?>
                <d:propfind xmlns:d="DAV:" xmlns:oc="${NS_OC}" xmlns:nc="${NS_NC}">${PROPS}</d:propfind>`,
        })).then(res => this._parse(res.text, dir).sort((a, b) => {
            if ( a.isdir != b.isdir ) return a.isdir ? -1 : 1;
            return a.name.localeCompare(b.name, undefined, {numeric: true, sensitivity: 'base'});
        }));
    };

    // files changed lately, newest first, without walking the folder tree
    Client.prototype.recent = function() {
        const since = Math.floor(Date.now() / 1000) - RECENT_DAYS * 24 * 3600;
        return this.connect().then(() => this.request({
            path: '/remote.php/dav/',
            method: 'SEARCH',
            headers: {'Content-Type': 'text/xml; charset=utf-8'},
            body: `<?xml version="1.0" encoding="UTF-8"?>
                <d:searchrequest xmlns:d="DAV:" xmlns:oc="${NS_OC}" xmlns:nc="${NS_NC}">
                <d:basicsearch>
                    <d:select>${PROPS}</d:select>
                    <d:from><d:scope>
                        <d:href>/files/${_xml_escape(this.user.id)}/</d:href><d:depth>infinity</d:depth>
                    </d:scope></d:from>
                    <d:where><d:and>
                        <d:not><d:eq><d:prop><d:getcontenttype/></d:prop><d:literal>httpd/unix-directory</d:literal></d:eq></d:not>
                        <d:gt><d:prop><d:getlastmodified/></d:prop><d:literal>${since}</d:literal></d:gt>
                    </d:and></d:where>
                    <d:orderby><d:order><d:prop><d:getlastmodified/></d:prop><d:descending/></d:order></d:orderby>
                    <d:limit><d:nresults>${RECENT_LIMIT}</d:nresults></d:limit>
                </d:basicsearch>
                </d:searchrequest>`,
        })).then(res => this._parse(res.text).filter(i => !i.isdir));
    };

    // the server's own file search (unified search)
    Client.prototype.search = function(term) {
        return this.connect().then(() =>
            this.ocs('/ocs/v2.php/search/providers/files/search?limit=30&term=' + encodeURIComponent(term))
        ).then(data => (data && data.entries || []).map(e => {
            const attrs = e.attributes || {};
            const path = attrs.path || '';
            const name = path ? path.substring(path.lastIndexOf('/') + 1) : e.title;
            return {
                path: path,
                name: name,
                isdir: /directory/.test(e.icon || '') || attrs.type == 'folder',
                fileid: _numeric_id(attrs.fileId),
                descr: e.subline || '',
            };
        }).filter(i => i.fileid !== undefined));
    };

    // uploads a local file into dir without replacing a file of the same name
    Client.prototype.upload = function(localpath, dir) {
        const name = localpath.substring(Math.max(localpath.lastIndexOf('/'), localpath.lastIndexOf('\\')) + 1);
        const path = _join(dir, name);
        return this.connect().then(() => this.request({
            path: this.davRoot() + _encode_path(path),
            method: 'PUT',
            headers: {'If-None-Match': '*'},
            file: localpath,
            timeout: 10 * 60 * 1000,
            conflict: true,
        })).then(res => {
            let fileid = _numeric_id(res.headers['oc-fileid']);
            if ( fileid !== undefined ) return {path: path, name: name, fileid: fileid};

            // older servers: ask for it
            return this.request({
                path: this.davRoot() + _encode_path(path),
                method: 'PROPFIND',
                headers: {'Depth': '0', 'Content-Type': 'application/xml; charset=utf-8'},
                body: `<?xml version="1.0"?><d:propfind xmlns:d="DAV:" xmlns:oc="${NS_OC}" xmlns:nc="${NS_NC}">${PROPS}</d:propfind>`,
            }).then(res => {
                const item = this._parse(res.text)[0];
                return {path: path, name: name, fileid: item && item.fileid};
            });
        });
    };

    // a new empty document from the server's blank template, via the Files templates API
    Client.prototype.create = function(dir, name) {
        const path = _join(dir, name);
        return this.connect().then(() => this.ocs('/ocs/v2.php/apps/files/api/v1/templates/create', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'filePath=' + encodeURIComponent(path) + '&templatePath=&templateType=user',
        })).then(data => {
            const fileid = data && _numeric_id(data.fileid);
            if ( fileid === undefined ) throw {status: 'error'};
            return {path: path, name: data.basename || name, fileid: fileid};
        });
    };

    // the page of the live editing session for a file
    Client.prototype.editorUrl = function(fileid) {
        let url = this.office && this.office.editor ?
                        this.office.editor.replace('{fileId}', fileid) :
                        `${this.portal}/index.php/apps/eurooffice/${fileid}`;
        return url + (url.includes('?') ? '&' : '?') + 'placement=desktop';
    };

    // whether the file opens in Nextcloud Office; without the capability (older app
    // versions) the common office formats do
    Client.prototype.canOpen = function(name) {
        const ext = name.substring(name.lastIndexOf('.') + 1).toLowerCase();
        if ( this.office && this.office.formats )
            return !!this.office.formats[ext];
        return /^(docx?|xlsx?|pptx?|odt|ods|odp|rtf|txt|csv|pdf)$/.test(ext);
    };

    // "HTTPS://Cloud.example.com/nc/" -> "https://cloud.example.com/nc"
    function _key(portal) {
        return portal.replace(/\/+$/, '').replace(/^[^:]+:\/\/[^\/]+/, m => m.toLowerCase());
    }

    const clients = {};
    return {
        // one client per cloud, so the user and capabilities are asked once
        get: function(portal) {
            const key = _key(portal);
            return clients[key] || (clients[key] = new Client(portal));
        },
        forget: function(portal) {
            delete clients[_key(portal)];
        },
        Client: Client,
    };
})();
