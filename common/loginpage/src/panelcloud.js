// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

/*
*   Files of a Nextcloud account, browsed in the start page instead of the cloud's web
*   page. Office files open in an editor tab in the cloud's live editing session.
*   See NextcloudClient for the requests.
*/

+function(){ 'use strict'

    const esc = s => String(s === undefined || s === null ? '' : s)
                        .replace(/[&<>"']/g, c => ({'&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;'})[c]);

    const SEARCH_DELAY = 400,       // ms after the last key press
          SEARCH_MIN = 2;           // characters

    // the tiles to create documents, as on the home page
    const NEW_TYPES = [
        {type: 'docx', label: 'DOCX', icon: '#docx-big', title: 'newDoc', start: '#4298C5', end: '#2D84B2', winxp: '#287ca9'},
        {type: 'xlsx', label: 'XLSX', icon: '#xlsx-big', title: 'newXlsx', start: '#5BB514', end: '#318C2B', winxp: '#3aa133'},
        {type: 'pptx', label: 'PPTX', icon: '#pptx-big', title: 'newPptx', start: '#F4893A', end: '#DE7341', winxp: '#f36700'},
    ];

    var ControllerCloud = function(args={}) {
        args.caption = 'Cloud files';
        args.action =
        this.action = 'cloud';
        this.view = new ViewCloud(args);
    };

    ControllerCloud.prototype = Object.create(baseController.prototype);
    ControllerCloud.prototype.constructor = ControllerCloud;

    var ViewCloud = function(args) {
        const _lang = utils.Lang;

        const tiles = NEW_TYPES.map(t => `
                        <button class="document-creation-item cloud-new" data-type="${t.type}">
                            <span class="format-label" style="--format-bg-start: ${t.start}; --format-bg-end: ${t.end}; --format-bg-winxp: ${t.winxp}">
                                <span>${t.label}</span>
                            </span>
                            <svg class="icon"><use xlink:href="${t.icon}"></use></svg>
                            <span class="title" l10n="${t.title}">${esc(_lang[t.title])}</span>
                        </button>`).join('');

        //language=HTML
        args.tplPage = `
            <div class="action-panel ${args.action}">
                <div class="recent-panel-container cloud-panel">
                    <header class="cloud-header">
                        <div class="cloud-account">
                            <div class="cloud-account-icon"></div>
                            <div class="cloud-account-text">
                                <h3 class="cloud-title"></h3>
                                <p class="cloud-subtitle">
                                    <span class="cloud-user"></span>
                                    <span class="cloud-dot">&middot;</span>
                                    <a href="#" class="cloud-web link" l10n>${esc(_lang.cloudOpenWeb)}</a>
                                </p>
                            </div>
                        </div>
                        <div class="search-container cloud-search-box">
                            <div class="icon-box">
                                <svg class="icon search-icon" data-iconname="search" data-precls="tool-icon"><use href="#search"></use></svg>
                            </div>
                            <input type="text" class="cloud-search" spellcheck="false" placeholder="${esc(_lang.cloudSearch)}">
                            <span class="tool close cloud-search-clear"></span>
                        </div>
                    </header>

                    <section class="cloud-create">
                        ${tiles}
                        <button class="document-creation-item cloud-upload">
                            <span class="cloud-upload-icon"><svg class="icon"><use xlink:href="#plus"></use></svg></span>
                            <span class="title" l10n>${esc(_lang.cloudUpload)}</span>
                        </button>
                    </section>

                    <nav class="cloud-nav">
                        <a class="nav-item cloud-tab" data-view="files" data-value="${esc(_lang.cloudAllFiles)}" l10n>${esc(_lang.cloudAllFiles)}</a>
                        <a class="nav-item cloud-tab" data-view="recent" data-value="${esc(_lang.cloudRecent)}" l10n>${esc(_lang.cloudRecent)}</a>
                    </nav>

                    <div class="cloud-location">
                        <div class="cloud-crumbs"></div>
                        <div class="cloud-results"></div>
                        <div class="cloud-progress"></div>
                    </div>

                    <div class="cloud-files">
                        <div class="file-list-head text-normal">
                            <div class="col-name" l10n>${esc(_lang.colFileName)}</div>
                            <div class="col-location" l10n>${esc(_lang.colLocation)}</div>
                            <div class="col-date" l10n>${esc(_lang.cloudModified)}</div>
                        </div>
                        <div class="file-list-body"></div>
                    </div>

                    <div class="cloud-loading"><div class="cloud-spinner"></div></div>

                    <div class="cloud-state">
                        <svg class="icon cloud-state-icon nothing-found">
                            <use class="light" href="#nothing-found-light"></use>
                            <use class="dark" href="#nothing-found-dark"></use>
                        </svg>
                        <svg class="icon cloud-state-icon connection-error"><use href="#connection_error"></use></svg>
                        <p class="cloud-state-title"></p>
                        <p class="cloud-message"></p>
                        <button class="btn btn--primary cloud-retry" l10n>${esc(_lang.cloudRetry)}</button>
                        <button class="btn btn--primary cloud-login" l10n>${esc(_lang.cloudLogin)}</button>
                    </div>
                </div>
            </div>`;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 3;
        args.tplItem = 'nomenuitem';

        baseView.prototype.constructor.call(this, args);
    };

    ViewCloud.prototype = Object.create(baseView.prototype);
    ViewCloud.prototype.constructor = ViewCloud;

    utils.fn.extend(ViewCloud.prototype, {
        itemtemplate: function(item, location) {
            const dot = item.isdir ? -1 : item.name.lastIndexOf('.');
            const base = dot > 0 ? item.name.substring(0, dot) : item.name,
                  ext = dot > 0 ? item.name.substring(dot) : '';
            const format = item.isdir ? 'folder' : utils.parseFileFormat(utils.fileExtensionToFileFormat(ext || '.'));
            const icon = item.isdir ? 'folder-small' : format;
            const date = item.modified ? new Date(item.modified).toLocaleString(undefined, {dateStyle: 'short', timeStyle: 'short'}) : '';
            const lock = item.locked ?
                    `<span class="cloud-lock" title="${esc(utils.Lang.cloudLockedBy.replace('%1', item.lockowner || ''))}">${esc(utils.Lang.cloudLocked)}</span>` : '';

            //language=HTML
            return `<div class="row text-normal${item.isdir ? ' folder' : ''}" tabindex="0">
                        <div class="col-name" title="${esc(item.name)}">
                            <div class="icon">
                                <svg class="icon" data-iconname="${esc(format)}" data-precls="tool-icon">
                                    <use xlink:href="#${esc(icon)}"></use>
                                </svg>
                                <i class="icon ${item.isdir ? 'img-el folder' : `img-format ${esc(format)}`}"></i>
                            </div>
                            <p class="name">${esc(base)}<span class="ext">${esc(ext)}</span></p>
                            ${lock}
                        </div>
                        <div class="col-location" title="${esc(location)}">${esc(location)}</div>
                        <div class="col-date">${esc(date)}</div>
                    </div>`;
        },
    });

    window.ControllerCloud = ControllerCloud;

    utils.fn.extend(ControllerCloud.prototype, (function() {
        let account,        // {portal, provider, user}
            client,
            view = 'files', // files, recent, search
            folder = '/',
            searched = '',
            searchTimer,
            loading = 0;    // drops the answers of requests a newer one replaced

        function _parent_dir(path) {
            const p = path.replace(/\/+$/, '');
            return p.substring(0, p.lastIndexOf('/') + 1) || '/';
        }

        // the files, the spinner or a message with an illustration
        function _show(what, opts={}) {
            const $panel = this.view.$panel;
            $panel.find('.cloud-files').toggle(what == 'files');
            $panel.find('.cloud-loading').toggle(what == 'loading');

            const $state = $panel.find('.cloud-state').toggle(what == 'state');
            if ( what == 'state' ) {
                $state.find('.nothing-found').toggle(opts.icon == 'empty');
                $state.find('.connection-error').toggle(opts.icon == 'error');
                $state.find('.cloud-state-title').text(opts.title || '');
                $state.find('.cloud-message').text(opts.text || '');
                $state.find('.cloud-retry').toggle(!!opts.retry);
                $state.find('.cloud-login').toggle(!!opts.login);
            }
        }

        function _progress(text) {
            this.view.$panel.find('.cloud-progress').text(text || '').toggle(!!text);
        }

        function _show_error(err) {
            const _lang = utils.Lang;
            const server = utils.skipUrlProtocol(account.portal);
            if ( err && err.status == 'auth' )
                _show.call(this, 'state', {icon: 'error', title: _lang.cloudErrorAuthTitle, text: _lang.cloudErrorAuth.replace('%1', server), login: true});
            else
            if ( err && err.status == 'offline' )
                _show.call(this, 'state', {icon: 'error', title: _lang.cloudErrorOfflineTitle, text: _lang.cloudErrorOffline.replace('%1', server), retry: true});
            else _show.call(this, 'state', {icon: 'error', title: _lang.cloudErrorGenericTitle, text: _lang.cloudErrorGeneric.replace('%1', server), retry: true});
        }

        function _render_location() {
            const $panel = this.view.$panel;
            const $crumbs = $panel.find('.cloud-crumbs').empty().toggle(view == 'files');
            $panel.find('.cloud-results').text(view == 'search' ? utils.Lang.cloudResultsFor.replace('%1', searched) : '')
                    .toggle(view == 'search');
            if ( view != 'files' ) return;

            const parts = folder.split('/').filter(p => p.length);
            $crumbs.append($(`<a href="#" class="cloud-crumb">${esc(utils.Lang.cloudHome)}</a>`).data('path', '/'));

            let path = '/';
            for (const part of parts) {
                path += part + '/';
                $crumbs.append('<span class="cloud-crumb-sep">&rsaquo;</span>');
                $crumbs.append($(`<a href="#" class="cloud-crumb">${esc(part)}</a>`).data('path', path));
            }
        }

        function _render_items(items) {
            const _lang = utils.Lang;
            const $panel = this.view.$panel;
            const $list = $panel.find('.file-list-body').empty();

            // the folder of each file, except in a folder
            $panel.find('.cloud-files').toggleClass('no-location', view == 'files');

            for (const item of items) {
                const location = view == 'files' ? '' : (item.descr ? item.descr : _parent_dir(item.path));
                const $item = $(this.view.itemtemplate(item, location));
                $item.on('click', () => _open_item.call(this, item));
                $item.on('keydown', e => { if ( e.key == 'Enter' ) _open_item.call(this, item); });
                $list.append($item);
            }

            if ( items.length )
                _show.call(this, 'files');
            else
            if ( view == 'search' )
                _show.call(this, 'state', {icon: 'empty', title: _lang.cloudNoResults, text: _lang.cloudNoResultsText});
            else
            if ( view == 'recent' )
                _show.call(this, 'state', {icon: 'empty', title: _lang.cloudRecentEmpty, text: _lang.cloudRecentEmptyText});
            else _show.call(this, 'state', {icon: 'empty', title: _lang.cloudEmpty, text: _lang.cloudEmptyText});
        }

        function _load() {
            if ( !client ) return;

            const $panel = this.view.$panel;
            const ticket = ++loading;
            $panel.find('.cloud-tab').removeClass('selected')
                    .filter(`[data-view=${view}]`).addClass('selected');
            $panel.find('.cloud-create').toggle(view == 'files');
            $panel.find('.cloud-search-clear').toggle(view == 'search');
            _render_location.call(this);
            _show.call(this, 'loading');

            let request;
            if ( view == 'recent' ) request = client.recent();
            else if ( view == 'search' ) request = client.search(searched);
            else request = client.list(folder);

            request.then(items => {
                if ( ticket != loading ) return;
                _render_header.call(this);
                _render_items.call(this, items);
            }, err => {
                if ( ticket != loading ) return;
                _show_error.call(this, err);
            });
        }

        function _render_header() {
            const $panel = this.view.$panel;
            $panel.find('.cloud-title').text(utils.skipUrlProtocol(account.portal));
            const user = client && client.user ? client.user.displayName : account.user;
            $panel.find('.cloud-user').text(user ? utils.Lang.cloudAccount.replace('%1', user) : '');
            $panel.find('.cloud-dot').toggle(!!user);

            // the provider's icon, as in the list of clouds
            const cfg = config.portals.providers && config.portals.providers.find(account.provider);
            const icons = cfg && cfg.icons;
            $panel.find('.cloud-account-icon').html(icons && icons.themeLight && icons.themeLight.connectionsList ?
                    `<img class="icon icon__light" src="${relpath}/providers/${esc(cfg.provider)}/${esc(icons.themeLight.connectionsList)}">
                     <img class="icon icon__dark" src="${relpath}/providers/${esc(cfg.provider)}/${esc((icons.themeDark || icons.themeLight).connectionsList)}">` : '');
        }

        function _search(term) {
            clearTimeout(searchTimer);
            term = term.trim();
            if ( term.length >= SEARCH_MIN ) {
                searched = term;
                view = 'search';
            } else
            if ( view == 'search' ) {
                view = 'files';
            } else return;
            _load.call(this);
        }

        function _open_editor(item) {
            window.sdk.command('cloud:open', JSON.stringify({
                url: client.editorUrl(item.fileid),
                name: item.name,
                portal: account.portal,
            }));
        }

        function _open_item(item) {
            if ( item.isdir ) {
                view = 'files';
                folder = item.path.endsWith('/') ? item.path : item.path + '/';
                this.view.$panel.find('.cloud-search').val('');
                _load.call(this);
            } else
            if ( item.fileid !== undefined && client.canOpen(item.name) ) {
                _open_editor.call(this, item);
            } else {
                // not a document: the cloud's own page shows it
                _open_web.call(this, item.fileid !== undefined ? `/index.php/f/${item.fileid}` : '/');
            }
        }

        function _open_web(entrypage) {
            window.sdk.command('portal:open', JSON.stringify({
                portal: account.portal,
                provider: account.provider,
                entrypage: entrypage,
            }));
        }

        function _create(ext) {
            const _lang = utils.Lang;
            const base = {docx: _lang.cloudNewDocument, xlsx: _lang.cloudNewSpreadsheet, pptx: _lang.cloudNewPresentation}[ext];

            // "New document.docx", or "New document (2).docx" when that exists
            _progress.call(this, _lang.cloudCreating);
            client.list(folder).then(items => {
                const names = new Set(items.map(i => i.name.toLowerCase()));
                let name = `${base}.${ext}`;
                for (let n = 2; names.has(name.toLowerCase()); ++n)
                    name = `${base} (${n}).${ext}`;
                return client.create(folder, name);
            }).then(file => {
                _progress.call(this);
                _open_editor.call(this, file);
                _load.call(this);
            }, err => {
                _progress.call(this);
                _show_error.call(this, err);
            });
        }

        function _upload() {
            const _lang = utils.Lang;
            window.sdk.OpenFilenameDialog('any', false, files => {
                const path = Array.isArray(files) ? files[0] : files;
                if ( !path ) return;

                const name = path.substring(Math.max(path.lastIndexOf('/'), path.lastIndexOf('\\')) + 1);
                _progress.call(this, _lang.cloudUploading.replace('%1', name));
                client.upload(path, folder).then(file => {
                    _progress.call(this);
                    _load.call(this);
                    if ( file.fileid !== undefined && client.canOpen(file.name) )
                        _open_editor.call(this, file);
                }, err => {
                    _progress.call(this);
                    if ( err && err.status == 'conflict' )
                        _progress.call(this, _lang.cloudErrorExists.replace('%1', name));
                    else _show_error.call(this, err);
                });
            });
        }

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);
                this.view.render();

                const $panel = this.view.$panel;
                _show.call(this, 'none');
                _progress.call(this);

                $panel.on('click', '.cloud-tab', e => {
                    view = $(e.currentTarget).data('view');
                    $panel.find('.cloud-search').val('');
                    _load.call(this);
                    return false;
                });
                $panel.on('click', '.cloud-crumb', e => {
                    view = 'files';
                    folder = $(e.currentTarget).data('path');
                    _load.call(this);
                    return false;
                });
                $panel.find('.cloud-search').on('keydown', e => {
                    if ( e.key == 'Enter' ) {
                        _search.call(this, $(e.currentTarget).val());
                    } else
                    if ( e.key == 'Escape' ) {
                        $(e.currentTarget).val('');
                        _search.call(this, '');
                    }
                }).on('input', e => {
                    clearTimeout(searchTimer);
                    const term = $(e.currentTarget).val();
                    searchTimer = setTimeout(() => _search.call(this, term), SEARCH_DELAY);
                });
                $panel.find('.cloud-search-clear').on('click', () => {
                    $panel.find('.cloud-search').val('');
                    _search.call(this, '');
                });
                $panel.on('click', '.cloud-new', e => _create.call(this, $(e.currentTarget).data('type')));
                $panel.find('.cloud-upload').on('click', () => _upload.call(this));
                $panel.find('.cloud-web').on('click', () => { _open_web.call(this, '/'); return false; });
                $panel.find('.cloud-retry').on('click', () => _load.call(this));
                $panel.find('.cloud-login').on('click', () => {
                    window.CommonEvents.fire('portal:reconnect', [account]);
                });

                window.CommonEvents.on('portal:loggedout', portal => {
                    if ( account && portal && utils.skipUrlProtocol(portal) == utils.skipUrlProtocol(account.portal) ) {
                        window.NextcloudClient.forget(account.portal);
                        client = undefined;
                    }
                });

                return this;
            },

            // shows the files of a cloud account: {portal, provider, user}
            open: function(model) {
                const changed = !account || account.portal != model.portal;
                account = {portal: model.portal, provider: model.provider, user: model.user};
                client = window.NextcloudClient.get(account.portal);

                // the files of the account, in the folder it showed last
                view = 'files';
                this.view.$panel.find('.cloud-search').val('');
                if ( changed ) folder = '/';

                _render_header.call(this);
                selectAction(this.action);
                _load.call(this);
            },

            // whether the start page browses this provider's clouds itself: providers
            // with "files": "nextcloud" in their config.json, which speak the Nextcloud
            // APIs, see NextcloudClient. They also log in with the system browser,
            // which is what lets the start page make requests for them
            handles: function(provider) {
                const cfg = config.portals.providers && config.portals.providers.find(provider);
                return !!cfg && cfg.files == 'nextcloud' && !!cfg.auth && !!cfg.auth.type && !!window.NextcloudClient;
            },

            account: () => account,
        };
    })());
}();
