/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

/*
    "connect to portal" page
    controller + view
*/

+function(){ 'use strict'
    window.relpath = !/mac os/i.test(navigator.userAgent) ? '.' : '..';
    window.config = { portals: {
        update: function() {
            config.portals.checklist = utils.fn.sortProviders(sdk.externalClouds());

            if ( config.portals.checklist ) {
                let _providers = {};
                config.portals.checklist.forEach(item => {
                    _providers[item.provider] = item;
                });

                config.portals.providers = _providers;
                config.portals.providers.find = name => config.portals.providers[name];
            }
        }
    }};

    config.portals.update();
    sdk.command("provider:list", JSON.stringify(config.portals.checklist));

    var ControllerPortals = function(args) {
        args.caption = 'Connect to portal';
        args.action = 
        this.action = "connect";

        this.view = new ViewPortals(args);
    };

    ControllerPortals.prototype = Object.create(baseController.prototype);
    ControllerPortals.prototype.constructor = ControllerPortals;

    var ViewPortals = function(args) {
        var _lang = utils.Lang;

        args.id&&(args.id=`id=${args.id}`)||(args.id='');


        this.tpl_sidebar =
                    `<div class="sidebar-block-title">
                        <span l10n>${_lang.actClouds}</span>
                        <button class="btn-quick login">
                            ${isSvgIcons? `<svg class = "icon"><use xlink:href="#plus"></use></svg>` : ''}
                                <i class="icon tool-icon__20 plus" />
                        </button>
                    </div>
                    <div class="sidebar-block-content flexbox scrollable">
                        <div class="table-box flex-fill"><table class="table-files list"></table></div>
                    </div>`;

        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 2;
        args.tplItem = 'nomenuitem';

        baseView.prototype.constructor.call(this, args);
    };
    var isSvgIcons = window.devicePixelRatio >=2 || window.devicePixelRatio == 1;

    ViewPortals.prototype = Object.create(baseView.prototype);
    ViewPortals.prototype.constructor = ViewPortals;

    utils.fn.extend(ViewPortals.prototype, {
        render: function() {

            const $placeholder = $(this.opts.placeholder);
            $placeholder.html(this.tpl_sidebar);

            this.$sidebarPortalList = $(this.opts.placeholder);
        },
        portaltemplate: function(info, edit) {
            let _row = `<td class="row-cell cicon">
                            <div class="icon-box">
                                <svg class='icon'><use href='#${info.iconid}'></svg>
                            </div>
                        </td>
                        <td class="row-cell info">
                            <div class="info-content">
                                <p class="cportal primary" title="${info.portal}">${utils.skipUrlProtocol(info.portal)}</p>
                                <p class="cuser minor">${info.user}${info.email.length && (' (' + info.email + ')') || ''}</p>
                            <div>
                        </td>
                        <td class="cell-tools">
                            <div class="hlayout">
                                <button class="btn-quick more">
                                    <svg class = "icon" data-precls="tool-icon" data-iconname="more">
                                        <use xlink:href="#more"></use>
                                    </svg>
                                    ${!isSvgIcons ? `<i class="icon tool-icon more" />` : ''}
                                </button>
                            </span>
                        </td>`;

            if ( !!info.themeicons ) {
                const icon_el = `<img class='icon icon__light' src='${info.themeicons.light}'></img>
                                    <img class='icon icon__dark' src='${info.themeicons.dark}'></img>`;
                _row = _row.replace(/<svg.+<\/svg>/, icon_el);
            } else
            if ( !!info.iconsrc ) {
                const icon_el = `<img class='icon icon_light' src='${info.iconsrc}'></img>`;
                _row = _row.replace(/<svg.+<\/svg>/, icon_el);
            }

            return edit===true ? _row : `<tr id=${info.elid}>${_row}</tr>`;
        },
        portalsemptypage: function() {
            const _lang = utils.Lang;
            const _page_tpl = `<div id="box-empty-portals" class="empty flex-center">
                                <section id='connect-empty-var-2'>
                                    <!--h3 class="empty-title" style="margin:0;" l10n>${_lang.portalEmptyTitle}</h3-->
                                    <h4 class='text-description' l10n='portalEmptyDescr'>${_lang.portalEmptyDescr}</h4>
                                    <section class='tools-connect2 connect'>
                                        <div id='box-providers-premium-button' />
                                        <div id="box-providers-buttons" style='font-size:0;' />
                                    </section>
                                    <h4 class='text-description separate-top' l10n='portalEmptyAdv1'>${_lang.portalEmptyAdv1}</h4>
                                    <div class="tools-connect">
                                        <button class="btn btn--landing newportal" l10n>${_lang.btnCreatePortal}</button>
                                        <section class="link-connect">
                                            <label l10n>${_lang.textHavePortal}</label><a class="login link" href="#" l10n>${_lang.btnConnect}</a>
                                        </section>
                                    </div>
                                </section>
                            </div>`;

            const provider_button_template = (provider, name, icons) => {
                                                const icon_light = icons ? icons.themeLight.buttonLogo : '',
                                                        icon_dark = icons ? icons.themeDark.buttonLogo : '';
                                                const button_el = `<img class='icon icon__light' src='${relpath}/providers/${provider}/${icon_light}'></img>
                                                                    <img class='icon icon__dark' src='${relpath}/providers/${provider}/${icon_dark}'></img>`;
                                                return `<button class="btn btn--big btn--svg login" data-cprov='${provider}'>
                                                            ${!!icons ? button_el : name}
                                                        </button>`;
                                            }

            const html_empty_panel = $(_page_tpl);
            let $box = $('<div />');
            config.portals.checklist.forEach(item => {
                if ( !!item.icons && !!item.icons.themeLight ) {
                    const btn = provider_button_template(item.provider, item.name, item.icons);

                    item.provider != 'onlyoffice' ? $box.append(btn) :
                            html_empty_panel.find('#box-providers-premium-button').append(btn);
                }
            });

            html_empty_panel.find('#box-providers-buttons').append($box.children());

            return html_empty_panel;
        },
        onscale: function (pasteSvg) {
            if (pasteSvg) {
                $('.btn-quick.login').each(function () {
                    if (!$(this).find('svg.icon').length) {
                        $(this).prepend('<svg class="icon"><use xlink:href="#plus"></use></svg>');
                    }
                });
            }
            // if ( !pasteSvg ) {
            //     $('button .icon.more',this.$panelPortalList).each(function (){
            //         let elm = $(this);
            //         if( !elm.find('i.icon').length )
            //             elm.append($('<i class="icon tool-icon more" />'));
            //     });
            // }
        }
    });

    window.ControllerPortals = ControllerPortals;
    utils.fn.extend(ControllerPortals.prototype, (function() {
        let collection,
            ppmenu;

        function _on_context_menu(menu, action, data) {
            var model = data;
            if (/\:open/.test(action)) {
                const _provider_cfg = config.portals.providers[model.provider];
                const _entrypage = !_provider_cfg ? '/' : _provider_cfg.startPage;
                sdk.command("portal:open", JSON.stringify({
                        portal: !_provider_cfg || !_provider_cfg.entryPage ? model.path : _provider_cfg.entryPage,
                        provider: model.provider,
                        entrypage: _entrypage}));
            } else
            if (/\:logout/.test(action)) {
                _do_logout.call(this, model);
            } else
            if (/\:forget/.test(action)) {
                model.removed = true;
                _do_logout.call(this, model);
            }
        };

        var _do_connect = function(model) {
            let _dialog = new DialogConnect({
                portal: model.path,
                provider: model.provider,
                onclose: opts => {
                    if ( opts ) {
                        opts.type = 'outer';

                        const _pm = config.portals.checklist.find(e => e.provider == opts.provider);
                        opts.entrypage = !_pm ? '/' : _pm.startPage;

                        sdk.execCommand("auth:outer", JSON.stringify(opts));
                    }

                    _dialog = undefined;
                }
            });

            _dialog.show();
        };

        function _do_logout(model) {

            let info = {domain:model.path};
            const _provider = config.portals.providers.find(model.provider);
            if ( _provider ) {
                if ( !!_provider.entryPage ) {
                    if ( !_provider.extraLogout )
                        _provider.extraLogout = [];

                    if ( !_provider.extraLogout.includes(_provider.entryPage) )
                        _provider.extraLogout.push(_provider.entryPage);
                }

                info.extra = _provider.extraLogout;
            };

            window.sdk.execCommand('portal:logout', JSON.stringify(info));
        };

        function _update_portals() {
            collection.empty();

            /* fill portals list */
            var portals = PortalsStore.portals();

            if (portals.length) {
                let auth_arr = {};
                for (let rec of portals) {
                    var pm = new PortalModel(rec);

                    auth_arr[pm.name] = '';
                    collection.add(pm);
                }

                window.sdk && window.sdk.checkAuth && window.sdk.checkAuth(auth_arr);

            } 
        };

        var _init_collection = function() {
                collection = new Collection({
                    view: this.view.$sidebarPortalList,
                    list: '.table-files.list'
                });

                function _create_icon_id(provider) {
                    switch ( provider ) {
                    case 'onlyoffice':
                    case 'asc': return 'icon__asc';
                    case 'owncloud': return 'icon__ownc';
                    case 'nextcloud': return 'icon__nextc';
                    default: return 'icon__common';
                    }
                };

                function _get_icon_scr(provider) {
                    const _model = config.portals.checklist.find(e => (e.provider == provider))
                    return !!_model && !!_model.icons && !!_model.icons.themeLight.connectionsList ?
                                `${relpath}/providers/${_model.provider}/${_model.icons.themeLight.connectionsList}` : undefined;
                };

                function _get_theme_icons(provider) {
                    const _model = config.portals.checklist.find(e => (e.provider == provider))
                    return !!_model && !!_model.icons ?
                                { light: `${relpath}/providers/${_model.provider}/${_model.icons.themeLight.connectionsList}`,
                                    dark: `${relpath}/providers/${_model.provider}/${_model.icons.themeDark.connectionsList}` } : 
                                undefined;
                };

                collection.events.changed.attach((collection, model, value) => {
                    if ( !!value ) {
                        if ( value.logged != undefined ) {
                            // this.view.$sidebarPortalList.find('#' + model.uid)[model.logged?'addClass':'removeClass']('logged');
                        }
                        else
                        if ( value.user ) {
                            let el = this.view.$sidebarPortalList.find('#' + model.uid);
                            el.html(
                                $(this.view.portaltemplate({
                                    portal: model.path,
                                    iconid: _create_icon_id(model.provider),
                                    iconsrc: _get_icon_scr(model.provider),
                                    themeicons: _get_theme_icons(model.provider),
                                    user: model.user,
                                    email: model.email}, true)));
                        } else
                        if ( value.removed != undefined ) {
                            value.removed ? $('#' + model.uid, this.view.$sidebarPortalList).addClass('lost') :
                                    $('#' + model.uid, this.view.$sidebarPortalList).removeClass('lost');
                        }
                    }
                });

                collection.events.inserted.attach((collection, model) => {
                    let $listPortals = collection.view.find('.table-files.list');
                    let $item = $(this.view.portaltemplate({
                        portal: model.path,
                        iconid: _create_icon_id(model.provider),
                        iconsrc: _get_icon_scr(model.provider),
                        themeicons: _get_theme_icons(model.provider),
                        user: model.user,
                        email: model.email,
                        elid: model.uid
                    }));
                    

                    $item.find('.more').click(model, e => {
                        if ( ppmenu.contextdata ) {
                            const m = ppmenu.contextdata;
                            if ( m.uid != model.uid )
                                Menu.closeAll();
                        }

                        if ( !Menu.opened ) {
                            ppmenu.disableItem('portal:logout', !model.logged);
                            ppmenu.showUnderElem(e.currentTarget, model);
                        } else {
                            Menu.closeAll();
                        }

                        e.stopPropagation && e.stopPropagation();
                        return false;
                    });
                    
                    $listPortals.append($item);
                });

                collection.events.click.attach((collection, model)=>{
                    _on_context_menu(undefined, 'portal:open', model);

                    // TODO: doubful variant to check portal availability on click instead of on launch
                    if ( model.get('exists') === undefined )
                        (new DialogConnect).portalexists(model.path, model.provider)
                                .then(data => {
                                    model.set('exists', true)
                                    // data.status == 'success' && _is_logged && model.set('logged', true); 
                                }, error => {
                                    $('#' + model.uid, this.view.$sidebarPortalList).toggleClass('unavail', true);
                                    model.set('logged', false)
                                    model.set('exists', false)
                });
                });

                collection.events.contextmenu.attach((collection, model, e)=>{
                    ppmenu.disableItem('portal:logout', !model.logged);
                    ppmenu.show({left: e.clientX, top: e.clientY}, model);
                });
        };

        var _init_ppmenu = function() {
            ppmenu = new Menu({
                id: 'pp-menu-portals',
                items: [{
                    caption: utils.Lang.menuFileOpen,
                    action: 'portal:open'
                },{
                    caption: utils.Lang.menuLogout,
                    action: 'portal:logout'
                },{
                    caption: utils.Lang.menuRemoveModel,
                    action: 'portal:forget'
                }]
            });

            ppmenu.init('#placeholder');
            ppmenu.events.itemclick.attach( _on_context_menu.bind(this) );
        };

        var _apply_auth = function(obj) {
            for (let i in obj) {
                let model = collection.find('name', i);

                if (model) {
                    // TODO: doubful variant to check portal availability on click instead of on launch
                    model.set('logged', obj[i].length > 0)

                    // model.set('logged', false)
                    // const _is_logged = obj[i].length > 0;
                    // (new DialogConnect).portalexists(model.path, model.provider)
                    //         .then(data => {
                    //             data.status == 'success' && _is_logged && model.set('logged', true); 
                    //         }, error => {
                    //             $('#' + model.uid, this.view.$sidebarPortalList).toggleClass('unavail', true);
                    //         });
                }
            };
        };

        var _on_create_portal = function() {
            window.sdk.execCommand('portal:create', '');
        };

        var _on_login_message = function(info) {
            let _write_portal_cookie = portal => {
                let _re = /^(https?:\/{2})?([^\<\>\/\s]+)(\/[^\s]+)?/i.exec(portal);
                let _domain = _re[2];

                sdk.setCookie(portal, _domain, _re[3] || '/', "asc_auth_key", utils.fn.uuid());
            };

            let obj = JSON.parse(info);
            if ( obj && !!obj.displayName ) {
                var model = collection.find('name', utils.skipUrlProtocol(obj.domain));
                if ( model ) {
                    !obj.email && (obj.email = '');
                    if ( model.email == obj.email ) {
                        if ( !model.get('logged') ) {
                            model.set('logged', true);
                            if (model.provider != 'onlyoffice')
                                _write_portal_cookie(obj.domain);

                            if ( model.get('removed') ) {
                                model.set('removed', false);
                                PortalsStore.keep({
                                    portal: model.path,
                                    provider: model.provider,
                                    user: model.user,
                                    email: model.email
                                });
                            }

                            if ( model.get('user') != obj.displayName ) {
                                model.set('user', obj.displayName);

                                let _m = PortalsStore.get({portal:model.path});
                                if ( !!_m ) {
                                    _m['user'] = obj.displayName;
                                    PortalsStore.keep(_m);
                                }
                            }
                        }
                        return;
                    } else {
                        PortalsStore.forget(obj.domain);
                    }
                }


                !obj.provider && (obj.provider = 'onlyoffice');
                if ( !config.portals.checklist.find(i => i.provider == obj.provider) ) {
                    let _p = config.portals.checklist.find(i => i.name.toLowerCase() == obj.provider.toLowerCase());
                    if ( _p )
                        obj.provider = _p.provider;
                    else {
                        console.warn(`login: "${obj.provider}" is unknown provider. please, check provider id.`);
                        return;
                    }
                }

                let info = {
                    portal: obj.domain,
                    provider: obj.provider,
                    user: obj.displayName,
                    email: obj.email
                };

                info.portal.endsWith('/') &&
                        (info.portal = info.portal.slice(0,-1));

                PortalsStore.keep(info);
                if ( obj.provider != 'onlyoffice' ) {
                    // sdk.setCookie(info.portal, utils.skipUrlProtocol(info.portal), "/", "asc_auth_key", utils.fn.uuid());
                    _write_portal_cookie(info.portal);

                    window.on_set_cookie = () => {
                        window.on_set_cookie = undefined;
                        _update_portals.call(this);
                    }
                } else {
                    _update_portals.call(this);
                }

                if ( $('.action-panel').filter('.welcome').is(':visible') ) {
                    window.selectAction('connect');
                }
            }
        };

        var _on_settings = function(cmd, params) {
            if (/init$/.test(cmd)) {
                if ( params.includes('\"portals\"\:') ) {
                    let opts;
                    try {
                        opts = JSON.parse(params);
                    } catch (e) { /*delete opts;*/ }
                } else
                if (params.includes('\"uitheme\"\:')) {
                    // let opts = JSON.parse(params);

                    // if ( !!opts.uitheme ) {
                    //     opts.uitheme == 'canuse' && (opts.uitheme = 'theme-light');

                    //     // if ( localStorage.getItem('ui-theme') != opts.uitheme )
                    //         _on_theme_changed(opts.uitheme);
                    // }
                }
            }
        };

        let carousel = {};
        function _scrollCarousel(direction) {
            function __check_limits(v, max) {
                if ( v < 0 ) return max;
                else if ( v > max ) return 0;
                else return v;
            };

            let _activeindex = carousel.$items.filter('.active').index();
            direction == 'next' ? ++_activeindex : --_activeindex;

            _activeindex = __check_limits(_activeindex, carousel.$items.length - 1);

            let _pre_index = _activeindex - 1,
                _pro_index = _activeindex + 1;

            _pre_index = __check_limits(_pre_index, carousel.$items.length - 1);
            _pro_index = __check_limits(_pro_index, carousel.$items.length - 1);

            carousel.$items.eq(_activeindex).addClass('migrate');
            if ( direction == 'next' ) {
                carousel.$items.filter('.pre-active').removeClass('pre-active').addClass('migrate');
                carousel.$items.eq(_pre_index).removeClass('migrate pre-active active pro-active').addClass('pre-active');
            } else {
                carousel.$items.filter('.pro-active').removeClass('pro-active').addClass('migrate');
                carousel.$items.eq(_pro_index).removeClass('migrate pre-active active pro-active').addClass('pro-active');
            }

            carousel.$items.eq(_activeindex).removeClass('migrate pre-active pro-active').addClass('active');

            if ( direction == 'next' )
                carousel.$items.eq(_pro_index).removeClass('migrate pre-active active pro-active').addClass('pro-active');
            else carousel.$items.eq(_pre_index).removeClass('migrate pre-active active pro-active').addClass('pre-active');
        };


        function _on_lang_changed(ol,nl) {
        };

        function _on_theme_changed(name, type) {
            if ( !!type )
                $('.carousel__slide__img > use').each((i, el) => {
                    const src = el.getAttribute('data-src');
                    if ( type == 'dark' )
                        el.setAttribute('xlink:href', `#${src}-dark`);
                    else el.setAttribute('xlink:href', `#${src}-light`);
                });
        }

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();

                window.sdk.on('on_check_auth', _apply_auth.bind(this));
                window.sdk.on('on_native_message', (cmd, param)=>{
                    let res = /portal:logout(\:cancel)?/.exec(cmd);
                    if (!!res && !!res[0]) {
                        var short_name = utils.skipUrlProtocol(param);
                        var model = collection.find('name', short_name);

                        if (!!model) {
                            if (!res[1]) {
                                model.set('logged', false);
                                if ( model.removed ) {
                                    model.set('removed', true);
                                    PortalsStore.forget(param);
                                }
                            } else
                                delete model.removed;
                        }
                    } else
                    if (/portal:login/.test(cmd)) {
                        _on_login_message.call(this, param);
                    } else
                    if (/^settings\:/.test(cmd)) {
                        _on_settings.call(this, cmd, param);
                    } else
                    if (cmd == 'uitheme:changed') {
                        _on_theme_changed(param);
                    }
                });

                _init_collection.call(this);
                _update_portals.call(this);
                _init_ppmenu.call(this);
                // _initCarousel.call(this);

                $('body').on('click', '.login', e=>{
                    var portals = PortalsStore.portals();
                    if (portals.length) {
                        let _data = $(e.currentTarget).data();
                        !_data ? _do_connect.call(this) : _do_connect.call(this, {provider:_data.cprov});
                    } else { 
                        new DialogProviders({
                            bodyTemplate: this.view.portalsemptypage(),
                            onConnect: (e, provider) => {
                                _do_connect.call(this, provider ? {provider} : {});
                            }
                        }).show();
                    }
                });

                window.CommonEvents.on('portal:create', _on_create_portal);
                window.CommonEvents.on('lang:changed', _on_lang_changed);
                window.CommonEvents.on('theme:changed', _on_theme_changed);
                window.CommonEvents.on("icons:svg", this.view.onscale);

                return this;
            },
            isConnected: function(portal) {
                var model = collection.find('name', utils.skipUrlProtocol(portal));
                return model && model.logged;
            },
            collection: function() {
                return collection;
            },
        };
    })());
}();

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerPortals({});
//     p.init();

//     !window.app && (window.app = {controller:{}});
//     !window.app.controller && (window.app.controller = {});
//     window.app.controller.portals = p;
// });
