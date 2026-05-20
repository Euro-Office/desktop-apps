/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

{

!window.l10n && (window.l10n = {boxNoConnect:{}});
!window.l10n.boxNoConnect && (window.l10n.boxNoConnect = {});

const langs = {
    en: {
        msgNoConn: 'Internet connection failed...',
        msgNoConnDesc: 'Check connection',
        msgFileNoConn: 'Check your Internet connection',
        msgFileNoConnDesc: "You are unable to edit the document because the Internet connection is lost or restricted. Please check your connection and reopen the document to continue.",
        msgTemplatesNoConn: 'Check your Internet connection',
        msgTemplatesNoConnDesc: "Couldn't load this section because you are experiencing possible network issues. Please check your Internet connection and try again.",
        msgFileError: 'Oops! Something went wrong',
        msgFileErrorDesc: "We lost access to your file due to a lack of memory or some other reason. Please don't worry and try reopening the file. Close this tab to continue.",
    }
}

const error_box = function() {
    let _page;
    const _tr = (n, l) => {
        return (l10n.boxNoConnect[l] && l10n.boxNoConnect[l][n]) || langs.en[n];
    }

    const page_config = {
        "def": ["msgNoConn", "msgNoConnDesc"],
        "cloudfile": ["msgFileNoConn", "msgFileNoConnDesc"],
        "templates": ["msgTemplatesNoConn", "msgTemplatesNoConnDesc"],
        "fileerr": ["msgFileError", "msgFileErrorDesc", "something_wrong"],
    };

    const _fix_lang = l => {
        if ( l && l10n && l10n.boxNoConnect ) {
            if ( l10n.boxNoConnect[l])
                return l;

            l = l.split(/[\-\_]/)[0];
            if ( l10n.boxNoConnect[l] )
                return l;
        }

        return 'en';
    }

    return {
        render: function(args = {}) {
            _page = args.page || 'def';
            const svg_id = page_config[_page] && page_config[_page][2] ? page_config[_page][2] : "connection_error";

            const html_ = `
                <section class="box-connection-error center">
                    <svg class="icon">
                        <use href="#${svg_id}"></use>
                    </svg>
                    <label id="idx-msg-short" class="description description__short"></label>
                    <label id="idx-msg-long" class="description description__long"></label>
                </section>`;

            if ( !args.parent ) args.parent = document.body;
            args.parent.insertAdjacentHTML('beforeend', html_);

            this.translate(args.lang);
        },
        translate: function(lang) {
            const page = !page_config[_page] ? 'def' : _page;
            lang = _fix_lang(lang);

            const ms = document.getElementById("idx-msg-short");
            if ( ms ) ms.innerText = _tr(page_config[page][0], lang);

            const ml = document.getElementById("idx-msg-long");
            if ( ml ) {
                ml.innerText = _tr(page_config[page][1], lang);

                // if ( _page == 'file' ) {
                //     ml.innerText = _tr("msgFileNoConnDesc", lang);
                // } else
                // if ( _page == 'templates') {
                //     ml.innerText = _tr("msgTemplatesNoConnDesc", lang);
                // } else ml.innerText = _tr("msgNoConnDesc", lang);
            }
        },
    }
}

window.errorBox = new error_box;
}

