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
    'external' panel
    controller + view
*/

+function(){ 'use strict'
    var ControllerExternalPanel = function(args) {
        args.caption = 'Activate panel';
        args.action =
        this.action = "encrypt";

        // this.view = new ViewActivate(args);
    };

    ControllerExternalPanel.prototype = Object.create(baseController.prototype);
    ControllerExternalPanel.prototype.constructor = ControllerExternalPanel;

    var ViewCustomPanel = function(args) {
        var _lang = utils.Lang;
        const isSvgIcons = window.devicePixelRatio >= 2 || window.devicePixelRatio === 1;

        let _html = `<div class="action-panel style--free ${args.action}">
                      <div class="flexbox">
                        <iframe name="${args.id}" id="${args.id}" src="${args.url}" allow="clipboard-read; clipboard-write"></iframe>
                      </div>
                    </div>`;

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        // args.itemindex = 3;
        // args.itemtext = args.itemtext;
        args.tplItem = `
              <li class="menu-item">
                <a action="${args.action}">
                    <div class="icon-box">
                        <svg class="icon" data-iconname="plugin" data-precls="tool-icon">
                            <use href="#plugin"></use>
                        </svg>
                        ${!isSvgIcons ? '<i class="icon tool-icon plugin"></i>' : ''}
                    </div>
                    <span class="text" l10n>${args.itemtext}</span>
                </a>
              </li>
        `;

        baseView.prototype.constructor.call(this, args);
    };

    ViewCustomPanel.prototype = Object.create(baseView.prototype);
    ViewCustomPanel.prototype.constructor = ViewCustomPanel;

    window.ControllerExternalPanel = ControllerExternalPanel;

    utils.fn.extend(ControllerExternalPanel.prototype, (()=>{
        var panels = [];
        function _add_custom_panel(opts) {
            let item_name = opts.name,
                panel_url = opts.url,
                panel_id = opts.id;

            let _panel = new ViewCustomPanel({
                                itemtext: item_name,
                                l10n: {itemtext:opts.nameLocale},
                                action: 'external-panel-' + panels.length,
                                id: panel_id,
                                url: panel_url
                            });

            _panel.render();
            _panel.$panel.find('iframe').css({'height':'100%','border':'0 none'});

            let iframe = _panel.$panel.find('iframe');

            if (!panel_url.startsWith("onlyoffice://")) {
                iframe.load( e => {
                    // var script = e.target.contentWindow.document.createElement("script");
                    // script.type = "text/javascript";
                    // script.innerHTML = `console.log('script loaded: ' + ONLYONET)`;
                    // e.target.contentWindow.document.body.appendChild(script);

                    if ( !!e.target.contentWindow.ONLYONET ) {
                        let _funcKeepPhrase = e.target.contentWindow.ONLYONET.storeSeedPhraseToFile;
                        e.target.contentWindow.ONLYONET.storeSeedPhraseToFile = function(seedPhrase, password, callback){
                            let _c = 0;
                            while ( !!localStorage['seedphrase' + _c] ) { ++_c; }
                            localStorage.setItem('seedphrase' + _c, seedPhrase);

                            return _funcKeepPhrase(seedPhrase, password, callback);
                        };
                    }
                });
            }

            if ( utils.Lang.id != 'en' )
                _translatePanel(_panel, 'en', utils.Lang.id);

            panels.push(_panel);

            /**/
            if ( panel_id.includes('\{B17BDC61\-') ) {
                _encrype_workaround(_panel);
            } else
            if ( panel_id.includes('F2402876-659F-47FB-A646-67B49F2B5AAA') ||
                    panel_id.includes('9DC93CDB-B576-4F0C-B55E-FCC9C48DD777') )
            {
                const $svgicon = _panel.$menuitem.find('svg.icon');
                if ( $svgicon.length ) {
                    $svgicon.data('iconname', 'aichat');
                    $('use', $svgicon).attr('href', '#aichat');
                }
                _panel.$menuitem.find('i.icon').removeClass('plugin').addClass('aichat');
            }
            /**/
        };

        function _encrype_workaround(view) {
            let _ison = (localStorage.getItem('encrypt') || 'false') == 'true';
            let _label = utils.Lang.settShowEncryptOpts;

            let tpl = `<div class='settings-field hbox'>
                            <div class='onoffswitch' id='sett-encrypt-switch'>
                                <input type="checkbox" name="onoffswitch" class="onoffswitch__checkbox" id="sett-checkbox-encrypt">
                                <label class="onoffswitch__label" for="sett-checkbox-encrypt"></label>
                            </div>
                            <label class='sett__caption' l10n>${_label}</label>
                        </div>`;

            $('.action-panel.settings .settings-items').append(tpl);
            $('.action-panel.settings #sett-encrypt-switch').parent().show();
            let checkbox = $('.action-panel.settings #sett-checkbox-encrypt');
            checkbox.prop('checked', _ison);

            if ( !_ison ) view.$menuitem.hide();
            checkbox.on('change', e => {
                e.target.checked ? view.$menuitem.show() : view.$menuitem.hide();
                localStorage.setItem('encrypt', e.target.checked);
            });

            sdk.on('onChangeCryptoMode', e => {
                checkbox.disable(e!=sdk.encrypt.ENCRYPT_MODE_NONE);
                checkbox.parents('.settings-field').find('.sett__caption').disable(e!=sdk.encrypt.ENCRYPT_MODE_NONE);
            });

            let _f_handle = (cmd, param) => {
                if (/retrive\:localoptions/.test(cmd)) {
                    setTimeout(e =>{
                        sdk.command('app:localoptions', JSON.stringify(localStorage,null,'\t'));
                        sdk.remove('on_native_message', _f_handle);
                    }, 0);
                }
            };
            sdk.on('on_native_message', _f_handle);
        };

        function _translatePanel(panel, langprev, langnext) {
            let predictTr = (arr, l) => {
                if ( !arr[l] ) {
                    for (let i in arr) {
                        if ( i.replace('-','_') == l )
                            return arr[i];
                        else
                        if ( i.substring(0,2) == l.substring(0,2) )
                            return arr[i];
                    }
                }

                return arr[l];
            };

            if ( panel.opts.l10n && panel.opts.l10n.itemtext ) {
                let nameprev = predictTr(panel.opts.l10n.itemtext, langprev) || panel.opts.itemtext,
                    namenext = predictTr(panel.opts.l10n.itemtext, langnext) || panel.opts.itemtext;

                if ( nameprev != namenext ) {
                    $('[l10n]',panel.$menuitem).html(namenext);
                }
            }

            let message = {event:'uiLangChanged', data: {new:langnext, old:langprev}};
            let iframe = panel.$panel.find('iframe')[0].contentWindow;
            iframe.postMessage(JSON.stringify(message), '*');
        };

        CommonEvents.on('lang:changed', (prev,next) => {
            for (let p of panels) {
                _translatePanel(p, prev, next);
            }
        });

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                sdk.on('on_native_message', (cmd, param) => {
                    if (/panel\:external/.test(cmd)) {
                        let opts = JSON.parse( $('<div>').html(param).text() );
                        _add_custom_panel(opts);
                    }
                });

                return this;
            }
        }
    })());
}();
