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
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

/*
*   Cloud login with the system browser, for providers with "auth": {"type": ...}
*   in their config (loginpage/providers/<provider>/config.json).
*
*   A method's login(portal) returns {promise, cancel}; the promise resolves with
*   {login, authorization} (Authorization header value) or rejects with
*   {status: 'unsupported'|'error'|'canceled', error}. Requests go through
*   AscSimpleRequest (CEF network stack). The native side (CBrowserAuth in win-linux)
*   stores the authorization and sends it with all requests to the cloud.
*/
window.BrowserAuth = (function() {
    "use strict";

    function _request(opts) {
        return new Promise((resolve, reject) => {
            if ( !window.AscSimpleRequest || !window.AscSimpleRequest.createRequest ) {
                reject({status: 'unsupported'});
                return;
            }

            window.AscSimpleRequest.createRequest(Object.assign({
                timeout: 15000,
                complete: e => resolve(e),
                error: e => reject({status: 'error', error: e && e.statusCode}),
            }, opts));
        });
    };

    function _json(text) {
        try { return JSON.parse(text); } catch (e) { return undefined; }
    };

    function _is_http_url(url) {
        return typeof url == 'string' && /^https?:\/\/[^\s\/]+/i.test(url);
    };

    // Nextcloud Login Flow v2: the user grants access in the browser and the
    // app receives an app password
    // https://docs.nextcloud.com/server/latest/developer_manual/client_apis/LoginFlow/
    const loginflow2 = {
        login: function(portal) {
            const POLL_INTERVAL = 2000,
                  FLOW_LIFETIME = 20 * 60 * 1000;     // Login Flow v2 tokens expire after 20 minutes
            let canceled = false, timer;

            const promise = new Promise((resolve, reject) => {
                _request({
                    url: portal + '/index.php/login/v2',
                    method: 'POST',
                    headers: {'OCS-APIRequest': 'true'},
                }).then(e => {
                    if ( canceled ) return;

                    const obj = _json(e.responseText);
                    if ( e.responseStatus != 200 || !obj || !obj.poll || !obj.poll.token ||
                            !_is_http_url(obj.poll.endpoint) || !_is_http_url(obj.login) )
                    {
                        // server without Login Flow v2
                        reject({status: 'unsupported'});
                        return;
                    }

                    window.sdk.execCommand('auth:browser:open', JSON.stringify({url: obj.login}));

                    const started = Date.now();
                    const poll = () => {
                        if ( canceled ) return;
                        if ( Date.now() - started > FLOW_LIFETIME ) {
                            reject({status: 'error', error: 'timeout'});
                            return;
                        }

                        _request({
                            url: obj.poll.endpoint,
                            method: 'POST',
                            body: 'token=' + encodeURIComponent(obj.poll.token),
                            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                        }).then(e => {
                            if ( canceled ) return;

                            // 404 until the user has granted access in the browser
                            const res = e.responseStatus == 200 && _json(e.responseText);
                            if ( !res ) {
                                timer = setTimeout(poll, POLL_INTERVAL);
                            } else
                            if ( !res.loginName || !res.appPassword ) {
                                reject({status: 'error', error: 'invalid response'});
                            } else {
                                resolve({
                                    login: res.loginName,
                                    authorization: 'Basic ' + btoa(unescape(encodeURIComponent(res.loginName + ':' + res.appPassword))),
                                });
                            }
                        }, () => {
                            // network hiccup: keep polling until the flow expires
                            timer = setTimeout(poll, POLL_INTERVAL);
                        });
                    };
                    timer = setTimeout(poll, POLL_INTERVAL);
                }, err => {
                    if ( !canceled ) reject(err);
                });
            });

            return {
                promise: promise,
                cancel: () => { canceled = true; clearTimeout(timer); },
            };
        },

        revoke: function(portal, authorization) {
            _request({
                url: portal + '/ocs/v2.php/core/apppassword',
                method: 'DELETE',
                headers: {'OCS-APIRequest': 'true', 'Authorization': authorization},
            }).catch(() => {});
        },
    };

    const methods = {
        loginflow2: loginflow2,
    };

    // the native side asks to invalidate the credentials on logout
    window.sdk.on('on_native_message', (cmd, param) => {
        if ( cmd == 'auth:browser:revoke' ) {
            const obj = _json(param);
            if ( obj && methods[obj.type] && methods[obj.type].revoke )
                methods[obj.type].revoke(obj.portal, obj.authorization);
        }
    });

    return {
        // TODO: CBrowserAuth is not implemented in the macOS app yet
        method: type => !utils.isMacOS && methods[type] ? methods[type] : undefined,
    };
})();

window.DialogConnect = function(params) {
    "use strict";

    !params && (params = {});
    !params.provider && (params.provider = 'onlyoffice');

    let $el, $title, $body;
    var _events = { close: params.onclose };
    var _template = `<dialog class="dlg dlg-connect-to">
                        <div class="title">
                            <label class="caption">Title</label>
                            <span class="tool close"></span>
                        </div>
                        <div class="body"></div>
                    </dialog>`;

    var _body_template = `<section>
                            <div id="panel-portal" class="sl-panel">
                                <div class='select-field dlg--option'>
                                    <section class='box-cmp-select'>
                                        <select class='combobox' data-size='5'></select>
                                    </section>
                                </div>
                                <div class="error-box">
                                    <p id="auth-error" class="msg-error">asd</p>
                                    <input id="auth-portal" type="text" name="" spellcheck="false" class="tbox dlg--option" placeholder="${utils.Lang.pshPortal}" value="">
                                </div>
                                <div style="height:12px;"></div>
                                <div id="box-connect" class="lr-flex">
                                    <!--<a class="text-sub link newportal" target="popup" href="javascript:void(0)">${utils.Lang.linkCreatePortal}</a>-->
                                    <span />
                                    <div class="lr-flex">
                                        <img class="img-loader">
                                        <button id="btn-next" l10n class="btn btn--landing">${utils.Lang.btnConnect}</button>
                                    </div>
                                </div>
                                <div id="box-browser-login" class="lr-flex" style="display:none;">
                                    <label class="text-sub" l10n>${utils.Lang.textBrowserLogin}</label>
                                    <button id="btn-cancel-login" l10n class="btn btn--landing">${utils.Lang.btnCancelLogin}</button>
                                </div>
                            </div>
                        </section>`;

    function _set_title(title) {
        $title.find('.caption').text(title);
    };

    function _set_error(error, focusel) {
        let $label = $el.find('#auth-error');

        if ( !!error ) {
            $label.text(error).fadeIn(100);
            !!focusel && $body.find(focusel).addClass('error').focus();
        } else {
            $label.fadeOut(100);
            $body.find('.error').removeClass('error');
        }
    };

    let _clear_error = _set_error.bind(this,'');

    function _on_close_click(e) {
        _close();
    };

    function _close(opts) {
        // $el[0].hasAttribute('open') && $el[0].close();
        _stop_browser_login();
        $el.remove();

        if ( _events.close ) {
            _events.close(opts);
        }
    };

    function _on_click_connect() {
        _clear_error();

        let provider = $body.find('select').val(),
            portal = $body.find('#auth-portal').val(),
            protocol = 'https://';

        const model = config.portals.providers.find(provider);
        if ( !!model && model.entryPage ) {
            _close({
                portal: model.entryPage,
                provider:provider
            });
            return
        }

        if (/^\s|\s$/.test(portal)) {
            portal = portal.trim();
            $body.find('#auth-portal').val(portal);
        }

        var re_wrong_symb = /([\s\r\n\t\\]|%(?!\d{2}))/;
        if (!portal.length || re_wrong_symb.test(portal) ||
                !/^(https?:\/\/)?([^@\/\s]{1,63})\/[^\s]*/.test(portal+'/') )
        {
            _set_error(utils.Lang.errLoginPortal, '#auth-portal');
            return;
        }

        portal = /^(https?:\/{2})?([^\s\?\&]+)/i.exec(portal);
        if (!!portal && portal[2].length) {
            portal[1] && (protocol = portal[1]);
            portal = portal[2];
        } else {
            _set_error(utils.Lang.errLoginPortal, '#auth-portal');
            return;
        }

        portal = portal.replace(/\/+$/i, '');

        /* skip odd url parts for owncloud */       
        if ( provider == 'owncloud' || provider == 'nextcloud' ) {
            portal.endsWith('/index.php/login') && (portal = portal.slice(0,-16));
        }

        _disable_dialog(true);

        // portal += config.portals.checklist[farm];

        let _callback = (obj) => {
            if ( obj ) {
                if ( obj.status == 'success' || obj.status == 'skipped' ) {
                    if ( obj.status == 'skipped' )
                        console.log(`get portal info skipped, ${obj.response.statusText}`);

                    if ( _browser_auth_type(provider) )
                        _start_browser_login(protocol+portal, provider);
                    else
                    _close({
                        portal:protocol+portal,
                        provider:provider
                    });
                } else {
                    // if ( obj.response.status == 404 || obj.status == 'timeout' ) {
                        _set_error(utils.Lang.errLoginPortal, '#auth-portal');
                    // } else _set_error((obj.response && obj.response.statusText) || obj.status, '#auth-portal');
                    console.log(`get portal info status: ${obj.status}, ${!!obj.response ? obj.response.statusText:'no response'}`);
                }
            }

            _disable_dialog(false);
        };

        _require_portal_info(protocol + portal, provider).then(
            _callback,
            obj => {
                if ( obj.status == 'error' && (obj.response.status == 404 || obj.response.statusCode == 404) ) {
                    protocol = protocol == "https://" ? "http://" : "https://";
                    return _require_portal_info(protocol + portal, provider);
                } else _callback(obj);
                return obj;
            }
        ).then( _callback, _callback );
    };

    // providers with "auth": {"type": ...} in their config log in with the system browser
    let _browser_login;

    function _browser_auth_type(provider) {
        const model = config.portals.providers.find(provider);
        const type = !!model && !!model.auth ? model.auth.type : undefined;
        return BrowserAuth.method(type) ? type : undefined;
    };

    function _show_browser_login(show) {
        $body.find('#box-browser-login')[show ? 'show' : 'hide']();
        $body.find('#box-connect').toggle(!show);
        $body.find('.tbox, select').prop('disabled', show);
    };

    function _start_browser_login(portal, provider) {
        _stop_browser_login();

        const type = _browser_auth_type(provider);
        const flow = BrowserAuth.method(type).login(portal);
        _browser_login = flow;
        _show_browser_login(true);

        flow.promise.then(res => {
            if ( _browser_login !== flow ) return;
            _browser_login = undefined;

            // the native side stores the credentials and opens the portal,
            // which then reports "portal:login"
            window.sdk.execCommand('auth:browser:store', JSON.stringify({
                portal: portal, provider: provider, type: type,
                login: res.login, authorization: res.authorization}));
            _close();
        }, err => {
            if ( _browser_login !== flow ) return;
            _stop_browser_login();

            if ( err.status == 'unsupported' ) {
                // fall back to the in-app login
                _close({portal: portal, provider: provider});
            } else
            if ( err.status == 'error' ) {
                _set_error(err.error == 'timeout' ? utils.Lang.errBrowserLoginTimeout : utils.Lang.errBrowserLogin);
            }
        });
    };

    function _stop_browser_login() {
        if ( !_browser_login ) return;

        _browser_login.cancel();
        _browser_login = undefined;
        _show_browser_login(false);
    };

    function _on_combo_provider_change(e) {
        const item = config.portals.providers.find(e.target.value);
        if ( !!item ) {
            const $portal = $body.find('#auth-portal');
            if ( item.entryPage ) {
                if ( !$portal.data("autoportal") )
                    $portal.data("userportal", $portal.val());
                $portal.data("autoportal", true);

                $portal[0].disabled = true;
                $portal.val(item.entryPage)
            } else {
                if ( $portal.data("autoportal") ) {
                    $portal.val($portal.data("userportal"));
                    $portal.data("autoportal", false);
                }

                $portal[0].disabled = false;
            }
        }
    }

    function _bind_events() {
        $body.on('keypress', '.tbox', 
            function(e) {
                if (e.which == 13) {
                    if (/auth-portal/.test(e.target.id)) 
                        _on_click_connect(); 
                    // else
                    // if (/auth-email/.test(e.target.id)) 
                    //     $el.find('#auth-pass').focus(); else
                    // if (/auth-pass/.test(e.target.id)) {
                    //     $el.find('#btn-login').focus().click();
                    // }
                }
        });

        // $el.on('keyup', e => {
        //     if (e.which == 27) _close();
        // });
    };

    function _disable_dialog(disable) {
        $body.find('.tbox, #btn-login').prop('disabled', disable || !!_browser_login);
        $body.find('.img-loader')[disable?'show':'hide']();
    };

    function _require_portal_info(portal, provider) {
        !provider && (provider = 'onlyoffice');
        const _model = config.portals.checklist.find(i => i.provider == provider);
        let _url;
        if ( _model )
            if ( !!_model.check.url )
                _url = portal + _model.check.url;
            else _url = `no check url for ${provider} found`;
        else _url = `no config for ${provider} found`

        return new Promise((resolve, reject) => {
            if ( !_url.startsWith('http') )
                resolve({status:'skipped', response: {statusText: _url}});
            else {
    
                let fetchFuntion = $.ajax;
                if (window.AscSimpleRequest && window.AscSimpleRequest.createRequest)
                    fetchFuntion = window.AscSimpleRequest.createRequest;
                
                fetchFuntion({
                    url: _url,
                    crossOrigin: true,
                    crossDomain: true,
                    timeout: 10000,
                    headers: _model.check.headers,
                    complete: function(e, status) {
                        if ( status == 'success' ) {
                            try {
                                // skip checking response for tests
                                // if ( !_model.entryPage )
                                    // JSON.parse(e.responseText)

                                resolve({status:status, response:e});
                            } catch (err) {
                                e.status = 404;
                                reject({status:'error', response:e});
                            }
                        } else {
                            reject({status:status, response:e});
                        }
                    },
                    error: function(e, status, error) {
// AscSimpleRequest
// include/base/internal/cef_net_error_list.h
// A connection attempt was refused.
// NET_ERROR(CONNECTION_REFUSED, -102)

                        if ( e.responseStatus == 302)
                            resolve({status:'success', response:e});
                        else {
                            if ( e.statusCode == -102 ) e.statusCode = 404;
                            reject({status:status, response:e});
                        }
                    }
                });
            }
        });
    };

    return {
        show: function() {
            $el = $('#placeholder').append(_template).find('.dlg-connect-to');
            $title = $el.find('.title');
            $body = $el.find('.body');

            $el.width(480);
            $title.find('.tool.close').bind('click', _on_close_click);

            _set_title( utils.Lang.loginTitleStart );
            $body.html( _body_template );

            let $combo = $el.find('select');
            let _clouds = config.portals.checklist;
            if ( _clouds.length == 1 && _clouds[0].provider == 'onlyoffice' ) {
                $combo.append(`<option value='onlyoffice'>onlyoffice</option>`);
                $combo.parents('.select-field').hide();
            } else {
                for (let c of _clouds) {
                    const icon_light = c.icons ? c.icons.themeLight.connectionsList : '',
                          icon_dark  = c.icons ? c.icons.themeDark.connectionsList : '';

                    $combo.append(`
                        <option value='${c.provider}' data-content='
                            ${`<img class="icon icon__light" src="${relpath}/providers/${c.provider}/${icon_light}" />`}
                            ${`<img class="icon icon__dark"  src="${relpath}/providers/${c.provider}/${icon_dark}" />`}
                            ${c.name}
                        '></option>
                    `);
                }
                $combo.val(params.provider);

                let $newportal = $el.find('.newportal').disable(!(params.provider=='onlyoffice'));
                $combo.on('change', e => {
                    $newportal.disable(!(e.target.value=='onlyoffice'));
                    _clear_error();
                });

                $combo.selectpicker();
                $combo.on('change', _on_combo_provider_change.bind(this));
                $combo.trigger('change');
            }
            
            let $portal = $body.find('#auth-portal');
            if ( !!params.portal ) $portal.val(utils.skipUrlProtocol(params.portal));
            $body.find('#btn-next').on('click', _on_click_connect);
            $body.find('#btn-cancel-login').on('click', e => _stop_browser_login());

            _bind_events();
            $el.get(0).showModal();
            $el.addClass('scaled');
            $el.on('close', _on_close_click);
            $body.find('.newportal').one('click', e => {
                setTimeout(t =>_close(), 0);
            });

            setTimeout(()=>{$portal.focus();}, 50);
        },
        portalexists: _require_portal_info
    };
};
