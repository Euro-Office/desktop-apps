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


function checkScaling() {
    const matches = {
        'pixel-ratio__1': `screen (-webkit-max-device-pixel-ratio: 1.24)`,
        'pixel-ratio__1_25': `screen and (-webkit-min-device-pixel-ratio: 1.25) and (-webkit-max-device-pixel-ratio: 1.49),
                                screen and (min-resolution: 1.25dppx) and (max-resolution: 1.49dppx)`,
        'pixel-ratio__1_5': `screen and (-webkit-min-device-pixel-ratio: 1.5) and (-webkit-max-device-pixel-ratio: 1.74),
                                screen and (min-resolution: 1.5dppx) and (max-resolution: 1.74dppx)`,
        'pixel-ratio__1_75': `screen and (-webkit-min-device-pixel-ratio: 1.75) and (-webkit-max-device-pixel-ratio: 1.99),
                                screen and (min-resolution: 1.75dppx) and (max-resolution: 1.99dppx)`,
        'pixel-ratio__2': `screen and (-webkit-min-device-pixel-ratio: 2) and (-webkit-max-device-pixel-ratio: 2.24),
                                screen and (min-resolution: 2dppx) and (max-resolution: 2.24dppx),
                                screen and (min-resolution: 192dpi) and (max-resolution: 215dpi)`,
        'pixel-ratio__2_5': `screen and (-webkit-min-device-pixel-ratio: 2.25), screen and (min-resolution: 2.25dppx),
                                screen and (min-resolution: 216dpi)`,
    };

    for (var c in matches) {
        const media_query_list = window.matchMedia(matches[c]);
        media_query_list.addEventListener('change', function(cls, e) {
            if ( e.matches ) {
                document.body.className = document.body.className.replace(/pixel-ratio__[\d_]+/gi, '').trim();
                document.body.classList.add(cls);
            }
        }.bind(this, c));

        if ( media_query_list.matches ) {
            document.body.classList.add(c);
            // break;
        }
    }
}

checkScaling();

var params = (function() {
    var e,
        a = /\+/g,  // Regex for replacing addition symbol with a space
        r = /([^&=]+)=?([^&]*)/g,
        d = function (s) { return decodeURIComponent(s.replace(a, " ")); },
        q = window.location.search.substring(1),
        urlParams = {};

    while (e = r.exec(q))
        urlParams[d(e[1])] = d(e[2]);

    return urlParams;
})();

let ui_theme_name = params.uitheme || localStorage.getItem("ui-theme"), ui_theme_type;
if ( !!ui_theme_name ) {
    if ( /^{".+"}+$/.test(ui_theme_name) ) {
        const obj = JSON.parse(ui_theme_name);
        ui_theme_name = obj['id'] || 'theme-dark';
        ui_theme_type = obj['type'];
    }

    ui_theme_type = (ui_theme_type == 'dark' || /theme-(?:[a-z]+-)?dark(?:-[a-z]*)?/.test(ui_theme_name)) ? 'theme-type-dark' : 'theme-type-light';
    document.body.classList.add(ui_theme_name, ui_theme_type);
}
