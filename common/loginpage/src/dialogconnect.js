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
                                <div class="lr-flex">
                                    <a class="text-sub link newportal" target="popup" href="javascript:void(0)">${utils.Lang.linkCreatePortal}</a>
                                    <span />
                                    <div class="lr-flex">
                                        <img class="img-loader">
                                        <button id="btn-next" l10n class="btn btn--landing">${utils.Lang.btnConnect}</button>
                                    </div>
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
        $body.find('.tbox, #btn-login').prop('disabled', disable);
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
