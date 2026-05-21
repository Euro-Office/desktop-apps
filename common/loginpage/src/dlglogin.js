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

window.LoginDlg = function(opts) {
    "use strict";

    !opts && (opts = {});

    let panels = {
        panelPortalName: {
            title: utils.Lang.loginTitleStart,
            template: () => {
                return `<section><div id="panel-portal" class="sl-panel">
                          <div class="error-box">
                            <p id="auth-error" class="msg-error">error</p>
                            <input id="auth-portal" type="text" name="" spellcheck="false" class="tbox dlg--option" placeholder="${utils.Lang.pshPortal}" value="">
                          </div>
                          <div style="height:10px;"></div>
                          <div id="box-btn-next" class="lr-flex">
                            <!--<a id="link-create" class="text-sub link newportal" target="popup" href="javascript:void(0)">${utils.Lang.linkCreatePortal}</a>-->
                            <span />
                            <div>
                              <img class="img-loader">
                              <button id="btn-next" class="btn primary">${utils.Lang.btnNext}</button>
                            </div>
                          </div>
                        </div></section>`;
            }
        },
        panelUserName: {
            title: utils.Lang.loginTitleConnectTo,
            template: () => {
                return `<section><div id="panel-login" class="sl-panel">
                          <section id="box-btn-login-sso" class="next">
                            <button id="btn-login-sso" class="btn">Single Sign-on</button>
                            <div class="separator"></div>
                            <span class="separator-label">or</span>
                          </section>
                          <div class="error-box">
                              <p id="auth-error" class="msg-error">asd</p>
                              <input id="auth-email" type="text" class="tbox dlg--option" name="" spellcheck="false" placeholder="${utils.Lang.pshEmail}" maxlenght="255" value="">
                          </div>
                          <input id="auth-pass" type="password" name="" spellcheck="false" class="tbox dlg--option last" placeholder="${utils.Lang.pshPass}" maxlenght="64" value="">
                          <div id="box-btn-login" class="lr-flex">
                            <a id="link-restore" class="text-sub link" target="popup" href="javascript:void(0)">${utils.Lang.linkForgotPass}</a>
                            <span />
                            <div>
                              <img class="img-loader">
                              <button id="btn-login" class="btn primary">${utils.Lang.btnLogin}</button>
                            </div>
                          </div>
                        </div></section>`;
            }
        },
        panelAssignPhone: {
            title: utils.Lang.loginTitleAssignPhone,
            template: () => {
                return `<section><div id="panel-assign-phone" class="sl-panel">
                          <p class="notice text-sub">${utils.Lang.loginNoteAssignPhone}</p>
                          <div class="lr-flex error-box">
                            <p id="auth-error" class="msg-error">error</p>
                            <input id="auth-phone" type="text" class="tbox dlg--option combined" name="" spellcheck="false" placeholder="${utils.Lang.pshPhone}" maxlenght="255" value="">
                            <div>
                              <button id="btn-assign-phone" class="btn primary">${utils.Lang.btnNext}</button>
                            </div>
                          </div>
                       </div></section>`;
            }
        },
        panelApplyAuthCode: {
            title: utils.Lang.loginTitleApplyCode,
            template: (code) => {
                return `<section><div id="panel-apply-code" class="sl-panel">
                    <p class="notice text-sub">${utils.Lang.loginNoteApplyCode.replace(/\$1/, code)}</p>
                    <div class="lr-flex error-box">
                      <p id="auth-error" class="msg-error">error</p>
                      <input id="auth-code" type="text" class="tbox dlg--option combined" name="" spellcheck="false" placeholder="${utils.Lang.pshCode}" maxlenght="255" value="">
                      <div>
                        <button id="btn-apply-code" class="btn primary">${utils.Lang.btnNext}</button>
                      </div>
                    </div>
                    <div class="lr-flex bt-align">
                      <div>
                        <a id="link-repeat" class="text-sub link" target="popup" href="javascript:void(0)">${utils.Lang.linkResend}</a>
                        <span id="repeat-timer" class="timer text-sub">0:00</span>
                      </div>
                      <a id="link-change-phone" class="text-sub link" target="popup" href="javascript:void(0)">${utils.Lang.linkChangePhone}</a>
                    </div>
                </div></section>`;
            }
        }
    };

    var $el;
    var _tpl = `<dialog class="dlg dlg-login">
                  <div class="title">
                    <label class="caption">Title</label>
                    <span class="tool close"></span>
                  </div>
                  <div class="body"></div>
                </dialog>`;

    var protocol = 'https://',
        protarr = ['https://', 'http://'],
        startmodule = '/products/files/?desktop=true';
    var portal = opts.portal,
        provider = opts.provider || 'asc',
        ssoservice = undefined,
        user = opts.user;
    var events = {
        success: opts.success,
        close: opts.close
    };
    var STATUS_EXIST = 1;
    var STATUS_NOT_EXIST = 0;
    var STATUS_UNKNOWN = -1;
    var STATUS_NO_CONNECTION = -255;
    var PROD_ID = 4;
    var TIMER_REPEAT_SMS_CODE = 30;
    var idtimer;

    function bookUser(token) {
        Promise.all([
            clientCheckin(protocol + portal, token),
                getUserInfo(protocol + portal, token) ])
            .then( results => {
                let info = results[1];
                window.sdk.setCookie(protocol + portal, portal, "/", "asc_auth_key", token);
                window.on_set_cookie =
                        () => {
                            if ( !!events.success ) {
                                let auth_info = {
                                    portal: protocol + portal,
                                    user: info.displayName,
                                    email: info.email
                                };

                                events.success({type:'user', data:auth_info});
                            }

                            window.on_set_cookie = undefined;
                            doClose(1);
                        }
                },
                error => showLoginError(utils.Lang.errLoginAuth)
            );
    };

    function recognizeUser(server, data) {
        let _signal = e => {
            if ( e && e.responseText ) {
                var obj = JSON.parse(e.responseText);
                if (obj.statusCode == 402) {
                    showLoginError(obj.error.message);
                } else
                if (obj.statusCode == 500) {
                    showLoginError(utils.Lang.errLogin);
                } else
                if (obj.statusCode != 201) {
                    console.log('server error: ' + obj.statusCode);
                    showLoginError(utils.Lang.errLoginServer);
                } else {
                    if (obj.response.sms) {
                        user = data;
                        if ( !obj.response.phoneNoise )
                            performSmsAuth(); else
                            sendSmsCode( obj.response.phoneNoise );
                    } else
                    if (!obj.response.sms) {
                        bookUser(obj.response.token);
                    }
                }
            } else {
                showLoginError(utils.Lang.errLoginServer);
            }
        };

        postRequest(server, data).
            then( args => { _signal(args.e); },
                args => {
                    console.log('server error: ' + args.status + ', ' + args.error);
                    _signal(args.e);
                });
    };

    function onCloseClick(e) {
        doClose(0);
    };

    function doClose(code) {
        if ( !!idtimer ) clearInterval(idtimer);

        $el[0].hasAttribute('open') && $el[0].close();
        $el.remove();

        if (events.close) {
            events.close(code);
        }
    };

    function onLoginClick(e) {
        hideLoginError();

        let email = $el.find('#auth-email').val().trim();

        var re_wrong_symb = /[\s\\]/;
        if (!email.length || re_wrong_symb.test(email)) {
            showLoginError(utils.Lang.errLoginEmail, '#auth-email');
            return;
        }

        var pass = $el.find('#auth-pass').val();
        if (!pass || pass.length < 0) {
            showLoginError(utils.Lang.errLoginPass, '#auth-pass');
            return;
        }

        var url         = `${portal}/api/2.0/authentication.json`;
        var check_url   = `${portal}/api/2.0/people/@self.json`;

        disableDialog(true);
        recognizeUser(protocol+url, {userName: email, password: pass});
    };

    function showLoginError(error, focusel) {
        !error && (error = 'connection internal error');
        let $lbl = $el.find('#auth-error');

        $lbl.text(error);
        $lbl.fadeIn(100);

        !!focusel && $el.find(focusel).addClass('error').focus();
        disableDialog(false);
    };

    function hideLoginError() {
        let $lbl = $el.find('#auth-error');
        if ( $lbl.is(':visible') ) {
            $lbl.fadeOut(100);
            $el.find('.error').removeClass('error');
        }
    };

    function getUserInfo(url, token) {
        return new Promise ((resolve, reject)=>{
            var _url = url + "/api/2.0/people/@self.json";

            var opts = {
                url: _url,
                crossOrigin: true,
                crossDomain: true,
                headers: {'Authorization': token},
                beforeSend: function (xhr) {
                    // xhr.setRequestHeader ("Access-Control-Allow-Origin", "*");
                },
                complete: function(e, status) {
                    if (status == 'success') {
                        var obj = JSON.parse(e.responseText);
                        if (obj.statusCode == 200) {
                            resolve(obj.response);
                        } else {
                            console.log('authentication error: ' + obj.statusCode);
                            reject(obj.statusCode);
                        }
                    } else {
                        console.log('authentication error: ' + status);
                        reject(status);
                    }
                },
                error: function(e, status, error) {
                    console.log('server error: ' + status + ', ' + error);
                    reject(status);
                }
            };

            $.ajax(opts);
        });
    };

    function clientCheckin(url, token) {
        return new Promise ((resolve, reject) => {
            $.ajax({
                url: url + "/api/2.0/portal/mobile/registration.json",
                method: 'post',
                headers: {'Authorization': token},
                data: {type: PROD_ID}
                ,complete: function(e, status) {}
                ,error: function(e, status, error) {}
            });

            resolve();
        });
    };

    function remindPass(portal, email) {
        let _url_   = `${portal}/api/2.0/people/password.json`;
        var opts = {
            url:  _url_,
            type: 'post',
            crossOrigin: true,
            crossDomain: true,
            data: {email: email},
            complete: function(e, status) {
                console.log('complete: ' + e + ', ' + status);
            },
            error: function(e, status, error) {
                console.log('server error: ' + status + ', ' + error);
            }
        };

        $.ajax(opts);
    };

    function bindEvents() {
        $el.find('.body').on('keypress', '.tbox', 
            function(e) {
                if (e.which == 13) {
                    if (/auth-portal/.test(e.target.id)) 
                        onNextClick(); else
                    if (/auth-email/.test(e.target.id)) 
                        $el.find('#auth-pass').focus(); else
                    if (/auth-pass/.test(e.target.id)) {
                        $el.find('#btn-login').focus().click();
                    }
                }
        });

        $el.on('keyup',
            function(e) {
                if (e.which == 27) {
                    onCloseClick();
                }
        });
    };

    function disableDialog(disable) {
        $el.find('.tbox, #btn-login').prop('disabled', disable);
        $el.find('.img-loader')[disable?'show':'hide']();
    };

    function onRestorePass() {
        window.open(utils.defines.links.restorepass);
    };

    function onNextClick() {
        hideLoginError();
        portal = $el.find('#auth-portal').val().trim().toLowerCase();

        var re_wrong_symb = /[\s\\]/;
        if (!portal.length || re_wrong_symb.test(portal)) {
            showLoginError(utils.Lang.errLoginPortal, '#auth-portal');
            return;
        }

        portal = /^(https?:\/{2})?([^\/]+)/i.exec(portal);
        if (!!portal && portal[2].length) {
            portal[1] && (protocol = portal[1]);
            portal = portal[2];
        } else {
            showLoginError(utils.Lang.errLoginPortal, '#auth-portal');
            return;
        }

        disableDialog(true);

        let _callback = (obj) => {
            if ( obj ) {
                if ( obj.status == 'ok' ) {
                    disableDialog(false);
                    let params = {portal: portal};

                    if ( !!obj.response.ssoUrl && obj.response.ssoUrl.length ) {
                        params.authservice = {url: obj.response.ssoUrl};

                        if ( obj.response.ssoLabel && obj.response.ssoLabel.length )
                            params.authservice.label = obj.response.ssoLabel;
                    }

                    firstConnect(params);
                } else {
                    let _mess;
                    if ( obj.status == 'error' && obj.response.status == 404 )
                        _mess = utils.Lang.errLoginPortal;
                    else
                    if ( obj.response && !!obj.response.statusText )
                        _mess = obj.response.statusText;
                    else _mess = 'error: portal is unvailable';

                    showLoginError(_mess, '#auth-portal');
                }
            }
        };

        requirePortalInfo(protocol + portal).then(
            _callback,
            obj => {
                if ( obj.status == 'error' && obj.response.status == 404 ) {
                    protocol = protocol == "https://" ? "http://" : "https://";
                    return requirePortalInfo(protocol + portal);
                } else _callback(obj);
                return obj;
            }
        ).then( _callback, _callback );
    };

    function requirePortalInfo(portal, vendor) {
        return new Promise ((resolve, reject)=>{
            !vendor && (vendor = 'asc');
            let _info = config.portals.checklist.find(i => i.id == vendor);
            var _url = portal + _info.check.url;

            $.ajax({
                url: _url,
                crossOrigin: true,
                crossDomain: true,
                timeout: 10000,
                complete: function(e, status) {
                    if (status == 'success') {
                        if ( vendor == 'asc' ) {
                            var obj = JSON.parse(e.responseText);
                            if (obj.statusCode == 200) {
                                resolve({status:'ok', response:obj.response});
                            } else {
                                reject({status:'error', response: e});
                            }
                        } else {
                            resolve({status:'ok', response:e});
                        }
                    } else {
                        reject({status:status, response:e});
                    }
                },
                error: function(e, status, error) {
                    reject({status:status, response:e});
                }
            });
        });
    };

    function startDialog() {
        let _panel = panels.panelPortalName;

        setTitle( _panel.title );
        $el.find('.body').html( _panel.template() );
        $el.find('#btn-next').click(onNextClick);

        let $portal = $el.find('#auth-portal');
        if ( !!portal ) $portal.val(portal);

        setTimeout(()=>{$portal.focus();}, 50);
    };

    function firstConnect(params) {
        let _panel = panels.panelUserName;
        setTitle( _panel.title.replace(/\$1/, params.portal) );
        setBody( _panel.template() );

        let $email = $el.find('#auth-email');
        if ( !!user ) $email.val(user);

        if ( !!params.authservice ) {
            $el.find('#box-btn-login-sso').show();

            let _sso_btn = $el.find('#btn-login-sso');
            _sso_btn.click( e => {
                    events.success({type:'sso', portal:protocol+portal, provider:params.authservice.url});
                    doClose(0);
                });

            if ( !!params.authservice.label )
                _sso_btn.html( params.authservice.label );
        }

        $el.find('#link-restore').click(onRestorePass);
        $el.find('#btn-login').click(onLoginClick);

        setTimeout(() => {
            if ( !!$email.val() )
                $el.find('#auth-pass').focus(); else
                $email.focus();
        }, 50);
    };

    function performSmsAuth(phone) {
        disableDialog(false);

        setTitle(panels.panelAssignPhone.title);
        setBody(panels.panelAssignPhone.template());

        let $phone = $el.find('#auth-phone');
        if ( !!phone ) {
            $phone.val(phone);
        }

        // TODO: process new element 'Enter' press event and focusing in one place
        $phone.on('keypress', e => {
            if ( e.which == 13 ) {
                $el.find('#btn-assign-phone').click();
            }
        });

        $el.find('#btn-assign-phone').click(e => {
            let _phone_num = $phone.val();
            if ( !/^\+[\d\s]{1,30}$/.test(_phone_num) ) {
                $phone.on('input', e => {
                    hideLoginError();
                    $phone.off('input');
                });

                showLoginError(utils.Lang.errLoginWrongPhone, '#auth-phone');
                return;
            }

            let url  = `${protocol+portal}/api/2.0/authentication/setphone.json`;
            let data = {
                mobilePhone: _phone_num,
                userName: user.userName,
                password: user.password
            };

            postRequest(url, data).then(
                args => {
                    if (args.e.status == 201) {
                        let jsonObj = JSON.parse(args.e.responseText);

                        if ( !!jsonObj.response.phoneNoise ) {
                            $phone.off();
                            sendSmsCode( jsonObj.response.phoneNoise );
                        }
                    }
                    console.log('request succeed: ' + args.e.responseText);
                },
                args => {
                    console.log('request error: ' + args.error);
                });
        });

        setTimeout(() => {$phone.focus()}, 50);
    };

    function sendSmsCode(phone) {
        setTitle(panels.panelApplyAuthCode.title);
        setBody(panels.panelApplyAuthCode.template(phone));

        let $timer = $el.find('#repeat-timer');
        let $repeat = $el.find('#link-repeat');
        $repeat.click(e => {
            resubmitSmsCode();
            lockResubmit();
        });

        let $code = $el.find('#auth-code');
        $code.on('keypress', e => {
            if ( e.which == 13 ) {
                $el.find('#btn-apply-code').click();
            }
        });

        // show button to change phone number if number isn't hidden
        if ( /\*+/.test(phone) ) {
            $el.find('#link-change-phone').hide();
        } else {
            $el.find('#link-change-phone').click(e => {
                clearInterval(idtimer);
                performSmsAuth(phone);
                $code.off();
            });
        }

        $el.find('#btn-apply-code').click(e => {
            let code = $code.val();
            let url  = `${protocol+portal}/api/2.0/authentication/${code}.json`;
            let data = {
                userName: user.userName,
                password: user.password
            };

            postRequest(url, data)
                .then(
                    args => {
                        if (args.e.status == 201) {
                            let obj = JSON.parse(args.e.responseText);
                            bookUser(obj.response.token);

                            $code.off();
                        }
                        console.log('request succeed: ' + args.e.responseText);
                    },
                    args => {
                        console.log('request error: ' + args.error);
                    });
        });
        setTimeout(() => {$code.focus()}, 50);

        function lockResubmit() {
            if ( $timer.length ) {
                $timer.show();
                $repeat.attr('disabled', 'disabled');

                idtimer && clearInterval(idtimer);
                let count = 0;
                $timer.text(utils.Lang.textThrough + ' 0:' + TIMER_REPEAT_SMS_CODE);
                idtimer = setInterval(()=>{
                    if ( ++count < TIMER_REPEAT_SMS_CODE ) {
                        let str_elaps = ' 0:' + (TIMER_REPEAT_SMS_CODE - count);
                        $timer.text(utils.Lang.textThrough + str_elaps.replace(/\:(\d)$/, ':0$1'));
                    } else {
                        clearInterval(idtimer);
                        $timer.hide();
                        $repeat.removeAttr('disabled');
                    }
                }, 1000);
            }
        };

        lockResubmit();
    };

    function resubmitSmsCode() {
        let url  = `${protocol+portal}/api/2.0/authentication/sendsms.json`;
        let data = {
            userName: user.userName,
            password: user.password
        };

        postRequest(url, data)
            .then(
                args => {
                    if (args.e.status == 201) {
                        let obj = JSON.parse(args.e.responseText);
                    }
                    console.log('request succeed: ' + args.e.responseText);
                },
                args => {
                    console.log('request error: ' + args.error);
                });
    };

    function setTitle(title) {
        $el.find('.title .caption').text(title);
    };

    function setBody(html) {
        $el.height('auto');
        let before = $el.height();
        $el.find('.body').html(html);
        let after = $el.height();

        // for using animation on dialog resize
        $el.height(before);
        $el.height(after);
    };

    function postRequest(url, data) {
        return new Promise((resolve, reject) => {
            var opts = {
                url:  url,
                type: 'post',
                crossOrigin: true,
                crossDomain: true,
                data: data,
                complete: (e, status) => { resolve({e:e, status:status}); },
                error: (e, status, error) => { reject({e:e, status:status, error:error}); }
            };

            $.ajax(opts);
        });
    };

    return {
        show: (params) => {
            !params && (params = {});

            $el = $('#placeholder').append(_tpl).find('.dlg-login');

            if ( !!params.portal ) {
                portal = utils.skipUrlProtocol(params.portal);

                let sp = utils.getUrlProtocol(params.portal);
                !!sp && (protocol = sp);
            }
            !!params.email && (user = params.email);
            if ( !!params.provider &&
                    config.portals.checklist.find(i => i.id == params.provider) )
            {
                provider = params.provider;
            }

            // $el.width(450).height(470);
            // set height without logo
            // $el.width(450).height(210);
            $el.width(450);

            $el.find('.tool.close').bind('click', onCloseClick);

            bindEvents();
            $el.get(0).showModal();
            $el.addClass('scaled');
            $el.on('close', doClose);

            startDialog();

            if ( params.portal && params.forceportal ) {
                firstConnect(params);
            }
        },
        close: function(){
            doClose(0);
        },
        portalavailable: requirePortalInfo
    };  
};

function doLogin() {

}
