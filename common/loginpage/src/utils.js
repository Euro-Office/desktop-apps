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

'use strict';

(function($) {
    $.fn.hasScrollBar = function() {
        return this.get(0).scrollHeight > this.height();
    }

    $.fn.extend({
        disable: function(state) {
            if ( this.is('a, label') ) {
                state ? this.attr('disabled', 'disabled') : this.removeAttr('disabled');
            } else $(this).prop('disabled', state);

            return this;
        },
        isdisabled: function() {
            return !this.is('a') ? !!$(this).prop('disabled') : !!this.attr('disabled');
        }
    });
})(jQuery);

/*
*  @winxpsupport
*/
+function polyfill_object_entries(){
    if ( !Object.entries )
        Object.entries = function(obj){
            let ownProps = Object.keys(obj),
                i = ownProps.length,
            resArray = new Array(i); // preallocate the Array

        while (i--) resArray[i] = [ownProps[i], obj[ownProps[i]]];
        return resArray;
    };
}();

var utils = {};
window.utils = utils;

window.PortalsStore = (function() {
    function contain(arr, item) {
        var index = -1;
        for (let i in arr) {
            let pi = arr[i];
            if (pi.portal == item.portal /*&&
                    pi.user == item.user && pi.email == item.email*/) 
            {
                index = i; 
                break;
            }
        }

        return index;
    };

    function getModel(info) {
        if ( !!info.portal ) {
            let _p = portals(),
                _i = contain(_p, info);
            if ( _i >= 0 ) {
                return _p[_i];
            }
        }

        return undefined;
    };

    /* info format: {portal: any.portal.com, user: 'User Name', email: 'user@portal.com'} */
    function addPortal(info) {        
        var portals = !!localStorage.portals ? JSON.parse(localStorage.portals) : [];
        if (!(info instanceof Array)) {
            info = [info];
        } 

        for (let i in info) {
            var index = contain(portals, info[i]);
            !(index < 0) && portals.splice(index, 1);

            portals.push(info[i]);
        }

        localStorage.setItem('portals', JSON.stringify(portals));
    };

    function removePortal(name) {
        var portals = !!localStorage.portals ? JSON.parse(localStorage.portals) : [];
        var index = -1;        
        for (let i = portals.length; i-- > 0; ) {
            if (portals[i].portal == name) {
                index = i; 
                break;
            }
        }

        if (!(index < 0)) {
            portals.splice(index, 1);
            localStorage.setItem('portals', JSON.stringify(portals));

            return true;
        }

        return false;
    };

    function portals() {
        let _out_arr = !!localStorage.portals ? JSON.parse(localStorage.portals) : [];
        return _out_arr.length ? _out_arr.reverse() : _out_arr;
    };

    return {
        get: getModel,
        keep: addPortal,
        portals: portals,
        forget: removePortal
    };
})();

utils.skipUrlProtocol = function(url) {
    return /^(https?:\/{2})?([^\<\>\s\?\&]+)/i.exec(url)[2];
};
utils.getUrlProtocol = function(url) {
    return /^(https?:\/{2})?([^\<\>]+)/i.exec(url)[1];
};

var FILE_DOCUMENT = 0x0040,
    FILE_PRESENTATION = 0x0080,
    FILE_SPREADSHEET = 0x0100,
    FILE_CROSSPLATFORM = 0x0200,
    FILE_DRAW = 0x4000;

utils.defines = {}
utils.defines.FileFormat = {
    FILE_UNKNOWN:       0x0000,

    FILE_DOCUMENT:      FILE_DOCUMENT,
    FILE_DOCUMENT_DOCX: FILE_DOCUMENT + 0x0001,
    FILE_DOCUMENT_DOC:  FILE_DOCUMENT + 0x0002,
    FILE_DOCUMENT_ODT:  FILE_DOCUMENT + 0x0003,
    FILE_DOCUMENT_RTF:  FILE_DOCUMENT + 0x0004,
    FILE_DOCUMENT_TXT:  FILE_DOCUMENT + 0x0005,
    FILE_DOCUMENT_HTML: FILE_DOCUMENT + 0x0006,
    FILE_DOCUMENT_MHT:  FILE_DOCUMENT + 0x0007,
    FILE_DOCUMENT_EPUB: FILE_DOCUMENT + 0x0008,
    FILE_DOCUMENT_FB2:  FILE_DOCUMENT + 0x0009,
    FILE_DOCUMENT_MOBI: FILE_DOCUMENT + 0x000a,
    FILE_DOCUMENT_DOCM: FILE_DOCUMENT + 0x000b,
    FILE_DOCUMENT_DOTX: FILE_DOCUMENT + 0x000c,
    FILE_DOCUMENT_DOTM: FILE_DOCUMENT + 0x000d,
    FILE_DOCUMENT_ODT_FLAT: FILE_DOCUMENT + 0x000e,
    FILE_DOCUMENT_OTT:  FILE_DOCUMENT + 0x000f,
    FILE_DOCUMENT_DOC_FLAT: FILE_DOCUMENT + 0x0010,
    FILE_DOCUMENT_OFORM: FILE_DOCUMENT + 0x0015,
    FILE_DOCUMENT_DOCXF: FILE_DOCUMENT + 0x0016,
    FILE_DOCUMENT_OFORM_PDF: FILE_DOCUMENT + 0x0017,
    FILE_DOCUMENT_PAGES: FILE_DOCUMENT + 0x0018,
    FILE_DOCUMENT_HWP:  FILE_DOCUMENT + 0x0019,
    FILE_DOCUMENT_HWPX: FILE_DOCUMENT + 0x0020,
    FILE_DOCUMENT_XML:   FILE_DOCUMENT + 0x0030,


    FILE_PRESENTATION:      FILE_PRESENTATION,
    FILE_PRESENTATION_PPTX: FILE_PRESENTATION + 0x0001,
    FILE_PRESENTATION_PPT:  FILE_PRESENTATION + 0x0002,
    FILE_PRESENTATION_ODP:  FILE_PRESENTATION + 0x0003,
    FILE_PRESENTATION_PPSX: FILE_PRESENTATION + 0x0004,
    FILE_PRESENTATION_PPTM: FILE_PRESENTATION + 0x0005,
    FILE_PRESENTATION_PPSM: FILE_PRESENTATION + 0x0006,
    FILE_PRESENTATION_POTX: FILE_PRESENTATION + 0x0007,
    FILE_PRESENTATION_POTM: FILE_PRESENTATION + 0x0008,
    FILE_PRESENTATION_ODP_FLAT: FILE_PRESENTATION + 0x0009,
    FILE_PRESENTATION_OTP:  FILE_PRESENTATION + 0x000a,
    FILE_PRESENTATION_ODG:  FILE_PRESENTATION + 0x000c,
    FILE_PRESENTATION_KEY:  FILE_PRESENTATION + 0x000d,

    FILE_SPREADSHEET:       FILE_SPREADSHEET,
    FILE_SPREADSHEET_XLSX:  FILE_SPREADSHEET + 0x0001,
    FILE_SPREADSHEET_XLS:   FILE_SPREADSHEET + 0x0002,
    FILE_SPREADSHEET_ODS:   FILE_SPREADSHEET + 0x0003,
    FILE_SPREADSHEET_CSV:   FILE_SPREADSHEET + 0x0004,
    FILE_SPREADSHEET_XLSM:  FILE_SPREADSHEET + 0x0005,
    FILE_SPREADSHEET_XLTX:  FILE_SPREADSHEET + 0x0006,
    FILE_SPREADSHEET_XLTM:  FILE_SPREADSHEET + 0x0007,
    FILE_SPREADSHEET_XLSB:  FILE_SPREADSHEET + 0x0008,
    FILE_SPREADSHEET_ODS_FLAT: FILE_SPREADSHEET + 0x0009,
    FILE_SPREADSHEET_OTS:   FILE_SPREADSHEET + 0x000a,
    FILE_SPREADSHEET_NUMBERS: FILE_SPREADSHEET + 0x000d,

    FILE_CROSSPLATFORM:     FILE_CROSSPLATFORM,
    FILE_CROSSPLATFORM_PDF: FILE_CROSSPLATFORM + 0x0001,
    FILE_CROSSPLATFORM_SWF: FILE_CROSSPLATFORM + 0x0002,
    FILE_CROSSPLATFORM_DJVU: FILE_CROSSPLATFORM + 0x0003,
    FILE_CROSSPLATFORM_XPS: FILE_CROSSPLATFORM + 0x0004,
    FILE_CROSSPLATFORM_PDFA: FILE_CROSSPLATFORM + 0x0009,

    FILE_DRAW:              FILE_DRAW,
    FILE_DRAW_VSDX:         FILE_DRAW + 0x0001,
    FILE_DRAW_VSSX:         FILE_DRAW + 0x0002,
    FILE_DRAW_VSTX:         FILE_DRAW + 0x0003,
    FILE_DRAW_VSDM:         FILE_DRAW + 0x0004,
    FILE_DRAW_VSSM:         FILE_DRAW + 0x0005,
    FILE_DRAW_VSTM:         FILE_DRAW + 0x0006,
};

utils.defines.DBLCLICK_LOCK_TIMEOUT = 800;
utils.defines.links = {
    regnew: 'https://www.onlyoffice.com/registration.aspx?app=desktop',
    restorepass: 'https://www.onlyoffice.com/signin.aspx'
};

utils.formatToEditor = function(f) {
    if ( f > FILE_PRESENTATION && f < FILE_SPREADSHEET ) return 'slide'; else
    if ( f > FILE_SPREADSHEET && f < FILE_CROSSPLATFORM ) return 'cell'; else
    if ( f > FILE_CROSSPLATFORM || f === utils.defines.FileFormat.FILE_DOCUMENT_OFORM_PDF ) return 'pdf'; 
    else return 'word';
}

utils.parseFileFormat = function(format) {
    switch (format) {
    case utils.defines.FileFormat.FILE_DOCUMENT_DOC:        return 'doc';
    case utils.defines.FileFormat.FILE_DOCUMENT_DOCX:       return 'docx';
    case utils.defines.FileFormat.FILE_DOCUMENT_ODT:        return 'odt';
    case utils.defines.FileFormat.FILE_DOCUMENT_RTF:        return 'rtf';
    case utils.defines.FileFormat.FILE_DOCUMENT_TXT:        return 'txt';
    case utils.defines.FileFormat.FILE_DOCUMENT_HTML:       return 'html';
    case utils.defines.FileFormat.FILE_DOCUMENT_MHT:        return 'mht';
    case utils.defines.FileFormat.FILE_DOCUMENT_EPUB:       return 'epub';
    case utils.defines.FileFormat.FILE_DOCUMENT_FB2:        return 'fb2';
    case utils.defines.FileFormat.FILE_DOCUMENT_DOCM:       return 'docm';
    case utils.defines.FileFormat.FILE_DOCUMENT_DOTX:       return 'dotx';
    case utils.defines.FileFormat.FILE_DOCUMENT_OTT:        return 'ott';
    case utils.defines.FileFormat.FILE_DOCUMENT_OFORM:      return 'oform';
    case utils.defines.FileFormat.FILE_DOCUMENT_DOCXF:      return 'docxf';
    case utils.defines.FileFormat.FILE_DOCUMENT_OFORM_PDF:  return 'pdf';
    case utils.defines.FileFormat.FILE_DOCUMENT_ODT_FLAT:   return 'fodt';
    case utils.defines.FileFormat.FILE_DOCUMENT_DOTM:       return 'dotm';
    case utils.defines.FileFormat.FILE_DOCUMENT_DOTM:       return 'dotm';
    case utils.defines.FileFormat.FILE_DOCUMENT_XML:        return 'xml';
    case utils.defines.FileFormat.FILE_DOCUMENT_PAGES:      return 'pages';
    case utils.defines.FileFormat.FILE_DOCUMENT_HWP:        return 'hwpx';
    case utils.defines.FileFormat.FILE_DOCUMENT_HWPX:       return 'hwp';

    case utils.defines.FileFormat.FILE_SPREADSHEET_XLS:     return 'xls';
    case utils.defines.FileFormat.FILE_SPREADSHEET_XLTX:    return 'xltx';
    case utils.defines.FileFormat.FILE_SPREADSHEET_XLSX:    return 'xlsx';
    case utils.defines.FileFormat.FILE_SPREADSHEET_XLSB:    return 'xlsb';
    case utils.defines.FileFormat.FILE_SPREADSHEET_ODS:     return 'ods';
    case utils.defines.FileFormat.FILE_SPREADSHEET_CSV:     return 'csv';
    case utils.defines.FileFormat.FILE_SPREADSHEET_OTS:     return 'ots';
    case utils.defines.FileFormat.FILE_SPREADSHEET_XLTM:    return 'xltm';
    case utils.defines.FileFormat.FILE_SPREADSHEET_XLSM:    return 'xlsm';
    case utils.defines.FileFormat.FILE_SPREADSHEET_ODS_FLAT:return 'fods';
    case utils.defines.FileFormat.FILE_SPREADSHEET_NUMBERS: return 'numbers';

    case utils.defines.FileFormat.FILE_PRESENTATION_PPT:    return 'ppt';
    case utils.defines.FileFormat.FILE_PRESENTATION_POTX:   return 'potx';
    case utils.defines.FileFormat.FILE_PRESENTATION_PPTX:   return 'pptx';
    case utils.defines.FileFormat.FILE_PRESENTATION_ODP:    return 'odp';
    case utils.defines.FileFormat.FILE_PRESENTATION_PPSX:   return 'ppsx';
    case utils.defines.FileFormat.FILE_PRESENTATION_OTP:    return 'otp';
    case utils.defines.FileFormat.FILE_PRESENTATION_PPTM:   return 'pptm';
    case utils.defines.FileFormat.FILE_PRESENTATION_PPSM:   return 'ppsm';
    case utils.defines.FileFormat.FILE_PRESENTATION_POTM:   return 'potm';
    case utils.defines.FileFormat.FILE_PRESENTATION_ODP_FLAT: return 'fodp';
    case utils.defines.FileFormat.FILE_PRESENTATION_KEY:    return 'key';
    case utils.defines.FileFormat.FILE_PRESENTATION_ODG:    return 'odg';

    case utils.defines.FileFormat.FILE_CROSSPLATFORM_PDFA:
    case utils.defines.FileFormat.FILE_CROSSPLATFORM_PDF:   return 'pdf';
    case utils.defines.FileFormat.FILE_CROSSPLATFORM_DJVU:  return 'djvu';
    case utils.defines.FileFormat.FILE_CROSSPLATFORM_XPS:   return 'xps';

    case utils.defines.FileFormat.FILE_DRAW_VSTX:
    case utils.defines.FileFormat.FILE_DRAW_VSSX:
    case utils.defines.FileFormat.FILE_DRAW_VSDM:
    case utils.defines.FileFormat.FILE_DRAW_VSSM:
    case utils.defines.FileFormat.FILE_DRAW_VSTM:
    case utils.defines.FileFormat.FILE_DRAW_VSDX:           return 'vsdx';

    default: return 'neutral';
    }

    return '';
};

utils.fileExtensionToFileFormat = function(extension) {
    switch (extension[0] != '.' ? extension : extension.substring(1)) {
    case 'doc':     return utils.defines.FileFormat.FILE_DOCUMENT_DOC;
    case 'docx':    return utils.defines.FileFormat.FILE_DOCUMENT_DOCX;
    case 'odt':     return utils.defines.FileFormat.FILE_DOCUMENT_ODT;
    case 'rtf':     return utils.defines.FileFormat.FILE_DOCUMENT_RTF;
    case 'txt':     return utils.defines.FileFormat.FILE_DOCUMENT_TXT;
    case 'html':    return utils.defines.FileFormat.FILE_DOCUMENT_HTML;
    case 'mht':     return utils.defines.FileFormat.FILE_DOCUMENT_MHT;
    case 'epub':    return utils.defines.FileFormat.FILE_DOCUMENT_EPUB;
    case 'fb2':     return utils.defines.FileFormat.FILE_DOCUMENT_FB2;
    case 'docm':    return utils.defines.FileFormat.FILE_DOCUMENT_DOCM;
    case 'dotx':    return utils.defines.FileFormat.FILE_DOCUMENT_DOTX;
    case 'ott':     return utils.defines.FileFormat.FILE_DOCUMENT_OTT;
    case 'oform':   return utils.defines.FileFormat.FILE_DOCUMENT_OFORM;
    case 'docxf':   return utils.defines.FileFormat.FILE_DOCUMENT_DOCXF;
    case 'pdf':     return utils.defines.FileFormat.FILE_DOCUMENT_OFORM_PDF;
    case 'fodt':    return utils.defines.FileFormat.FILE_DOCUMENT_ODT_FLAT;
    case 'dotm':    return utils.defines.FileFormat.FILE_DOCUMENT_DOTM;
    case 'dotm':    return utils.defines.FileFormat.FILE_DOCUMENT_DOTM;
    case 'xml':     return utils.defines.FileFormat.FILE_DOCUMENT_XML;

    case 'xls':     return utils.defines.FileFormat.FILE_SPREADSHEET_XLS;
    case 'xltx':    return utils.defines.FileFormat.FILE_SPREADSHEET_XLTX;
    case 'xlsx':    return utils.defines.FileFormat.FILE_SPREADSHEET_XLSX;
    case 'xlsb':    return utils.defines.FileFormat.FILE_SPREADSHEET_XLSB;
    case 'ods':     return utils.defines.FileFormat.FILE_SPREADSHEET_ODS;
    case 'csv':     return utils.defines.FileFormat.FILE_SPREADSHEET_CSV;
    case 'ots':     return utils.defines.FileFormat.FILE_SPREADSHEET_OTS;
    case 'xltm':    return utils.defines.FileFormat.FILE_SPREADSHEET_XLTM;
    case 'xlsm':    return utils.defines.FileFormat.FILE_SPREADSHEET_XLSM;
    case 'fods':    return utils.defines.FileFormat.FILE_SPREADSHEET_ODS_FLAT;

    case 'ppt':     return utils.defines.FileFormat.FILE_PRESENTATION_PPT;
    case 'potx':    return utils.defines.FileFormat.FILE_PRESENTATION_POTX;
    case 'pptx':    return utils.defines.FileFormat.FILE_PRESENTATION_PPTX;
    case 'odp':     return utils.defines.FileFormat.FILE_PRESENTATION_ODP;
    case 'ppsx':    return utils.defines.FileFormat.FILE_PRESENTATION_PPSX;
    case 'otp':     return utils.defines.FileFormat.FILE_PRESENTATION_OTP;
    case 'pptm':    return utils.defines.FileFormat.FILE_PRESENTATION_PPTM;
    case 'ppsm':    return utils.defines.FileFormat.FILE_PRESENTATION_PPSM;
    case 'potm':    return utils.defines.FileFormat.FILE_PRESENTATION_POTM;
    case 'fodp':    return utils.defines.FileFormat.FILE_PRESENTATION_ODP_FLAT;

    // case utils.defines.FileFormat.FILE_CROSSPLATFORM_PDFA:
    case 'pdf':     return utils.defines.FileFormat.FILE_CROSSPLATFORM_PDF;
    case 'djvu':    return utils.defines.FileFormat.FILE_CROSSPLATFORM_DJVU;
    case 'xps':     return utils.defines.FileFormat.FILE_CROSSPLATFORM_XPS;

    default: return utils.defines.FileFormat.FILE_UNKNOWN;
    }

    return '';
};

utils.fn = {};
utils.fn.extend = function(dest, src) {
    for (var prop in src) {
        if (src.hasOwnProperty(prop)) {
            if (typeof dest[prop] === 'object' &&
                        typeof src[prop] === 'object') {
                utils.extend(dest[prop], src[prop])
            } else {
                dest[prop] = src[prop];
            }
        }
    }

    return dest;
};

utils.fn.pinnedFolders = function(path, action) {
    const key = 'pinnedFolders';
    let pinned = JSON.parse(localStorage.getItem(key) || '[]');

    if (action === 'check') return pinned.includes(path);

    const i = pinned.indexOf(path);
    if (action === 'toggle') {
        i === -1 ? pinned.push(path) : pinned.splice(i, 1);
    }

    localStorage.setItem(key, JSON.stringify(pinned));
};

utils.fn.parseRecent = function(arr, out = 'files') {
    var _files_arr = [], _dirs_arr = [];

    const _is_win = /Win/.test(navigator.platform);
    const _re_name = !_is_win ? /([^/]+\.[a-zA-Z0-9]{1,})$/ : /([^\\/]+\.[a-zA-Z0-9]{1,})$/;
    for (let _f_ of arr) {
        if ( _is_win && /^\w:[\\\/]/.test(_f_.path) && /(?:\\{2,})+/.test(_f_.path) )
            _f_.path = _f_.path.replace(/(?:\\{2,})+/g,"\\");

        let fn = _f_.path;
        if ( _re_name.test(fn) ) {
            let name = _re_name.exec(_f_.path)[1],
            path = _f_.path.slice(0, fn.length - name.length - 1);

            _files_arr.push({
                fileid: _f_.id,
                pinid: !_f_.pin ? _f_.id : -_f_.id,
                type: _f_.type,
                format: utils.parseFileFormat(_f_.type),
                name: name,
                descr: path,
                date: _f_.modifyed,
                path: $('<div>').html(fn).text(),
                cloud: _f_.cloud,
                pinned: _f_.pin,
            });

            _dirs_arr.indexOf(path) < 0 && _dirs_arr.push(path);
        }
    }

    if (out == 'files') return _files_arr;

    const pinned = JSON.parse(localStorage.getItem('pinnedFolders') || '[]');
    for (const pinnedPath of pinned) {
        if (!_dirs_arr.includes(pinnedPath)) {
            _dirs_arr.push(pinnedPath);
        }
    }

    var out_dirs_arr = [];
    for (let _d_ of _dirs_arr) {
        let name = (!_is_win ? /([^/]+)$/ : /([^\\/]+)$/).exec(_d_)[1],
                parent;
        if (!name) {
            name = _d_;
            parent = 'My Computer' /*utils.Lang.textMyComputer*/;
        } else
            parent = _d_.slice(0, _d_.length - name.length - 1);

        let pinned = utils.fn.pinnedFolders(_d_, 'check');
        let id = _d_.hashCode();

        out_dirs_arr.push({
                type: 'folder',
                full: _d_,
                name: name,
                descr: parent,
                pinid: !pinned ? id : -id,
                pinned: pinned,
                uid: `folder-${id}`
        });
    }

    return out_dirs_arr;
}

utils.fn.decodeHtml = function(str) {
    return $('<div>').html(str).text();
}

utils.fn.getToolMenuItemOrder = function(item) {
    return { item: $('.menu-item a[action=settings]').parent(), after: false };

    let $item = $(item);

    let _action = $item.find('[action]').attr('action'),
        _is_top_group = !$item.hasClass('bottom');

    let _items_top_order = ['templates', 'recent', 'open', 'connect', 'activation', 'external-'],
        _items_bottom_order = ['about', 'settings'],
        _items_order = _is_top_group ? _items_top_order : _items_bottom_order;

    let $menu = $('.main-column.tool-menu');
    let $itemBefore = $menu.find(`.menu-item [action=${_action}]`);

    if ( $itemBefore.length ) return {item: $itemBefore.parent(), after: _is_top_group};
    else {
        let _index = _items_order.findIndex(element => {
            if ( element.endsWith('-') ) return _action.startsWith(element);
            else return _action == element;
        });
        if ( _index > 0 ) {
            while ( _index > 0 ) {
                let _ab = _items_order[--_index];
                $itemBefore = $menu.find(`.menu-item [action=${_ab}]`).parent();

                if ( $itemBefore.length ) return {item: $itemBefore, after: _is_top_group};
            }
            return _is_top_group ? {item: $menu.find('.tool-quick-menu').get(0), after: true} : {item: undefined, after: true};
        } else
        if ( _index == 0 ) {
            return _is_top_group ? {item: $menu.find('.tool-quick-menu').get(0), after: true} : {item: undefined, after: true};
        }

        let $items = $menu.find('.menu-item:not(.bottom)');
        return { item: $items.length ? $items.last() : $menu.find('.tool-quick-menu').get(0), after: true };
    }
};

utils.fn.uuid = function() {
    return ([1e7]+-1e3+-4e3+-8e3+-1e11).replace(/[018]/g, c =>
        (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16));
};

utils.fn.needUseSvg = function () {
    return !(window.devicePixelRatio < 2) || window.devicePixelRatio == 1;
}

utils.fn.sortProviders = function(list) {
    return (list || []).slice().sort((a, b) => {

        if (a.provider === 'onlyoffice' && b.provider !== 'onlyoffice') return -1;
        if (b.provider === 'onlyoffice' && a.provider !== 'onlyoffice') return 1;

        const aNoOrder = (a.order === undefined || a.order === null || a.order === '');
        const bNoOrder = (b.order === undefined || b.order === null || b.order === '');

        if (aNoOrder && !bNoOrder) return 1;
        if (!aNoOrder && bNoOrder) return -1;
        if (aNoOrder && bNoOrder) return 0;

        return Number(a.order) - Number(b.order);
    });
};

function getUrlParams() {
    var e,
    a = /\+/g,  
    r = /([^&=]+)=?([^&]*)/g,
    d = function (s) { return decodeURIComponent(s.replace(a, " ")); },
    q = window.location.search.substring(1),
    urlParams = {lang:'en'};

    while (e = r.exec(q))
        urlParams[d(e[1])] = d(e[2]);
    
    return urlParams;
}

utils.inParams = getUrlParams();
utils.brandCheck = opts => false;
utils.isWinXp = utils.inParams.osver == 'winxp' || /windows nt 5/i.test(navigator.appVersion);
utils.isMacOS = /mac os/i.test(navigator.userAgent);
