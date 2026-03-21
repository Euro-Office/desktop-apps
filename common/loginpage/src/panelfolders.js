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
*   new inherited controller declaration
*/

+function(){ 'use strict'
    var ControllerFolders = function(args={}) {
        args.caption = 'Recent folders';
        args.action =
        this.action = "open";
        this.view = new ViewFolders(args);
    };

    ControllerFolders.prototype = Object.create(baseController.prototype);
    ControllerFolders.prototype.constructor = ControllerFolders;

    const isSvgIcons = window.devicePixelRatio >= 2 || window.devicePixelRatio === 1;
    var ViewFolders = function(args) {
        var _lang = utils.Lang;

        args.id&&(args.id=`"id=${args.id}"`)||(args.id='');

        //language=HTML
        args.tplPage = `
            <div ${args.id} class="action-panel ${args.action}">
                <div class="open-panel-container">
                    <div class="file-list-title">
                        <h3 l10n>${_lang.listRecentDirTitle}</h3>
                    </div>
                    <section id="area-dnd-file"></section>
                    <div class="file-list-body"></div>
                    <div id="box-open-acts">
                        <button id="btn-openlocal" class="btn btn--primary" l10n>${_lang.btnBrowse}</button>
                    </div>
                </div>
            </div>`;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 1;
        // args.itemtext = _lang.actOpenLocal
        args.tplItem = 'nomenuitem';

        baseView.prototype.constructor.call(this, args);
    };

    ViewFolders.prototype = Object.create(baseView.prototype);
    ViewFolders.prototype.constructor = ViewFolders;

    window.ControllerFolders = ControllerFolders;

    utils.fn.extend(ControllerFolders.prototype, (function() {
        var _on_update = function(params) {
            var _dirs = utils.fn.parseRecent(params, 'folders'), $item;
            _dirs.sort((a, b) => {
                if (a.pinned && !b.pinned) return -1;
                if (!a.pinned && b.pinned) return 1;
                return 0;
            });

            const $listRecentDirs = this.view.$panel.find('.file-list-body');

            $listRecentDirs.empty();

            for (let dir of _dirs) {
                if (!utils.getUrlProtocol(dir.full)) {
                    $item = $(app.controller.recent.view.listitemtemplate(dir));

                if (dir.pinned) {
                    $item.addClass('pinned');
                }

                $item.find('.col-pin .btn-quick').click(function (e) {
                    
                    const folderPath = dir.full;
                    const newPinState = utils.fn.pinnedFolders(folderPath, 'toggle');
                    
                    $item.toggleClass('pinned', newPinState);
                    
                    _on_update.call(this, params);
                }.bind(this));

                $item.click({path: dir.full}, e=>{
                        openFile(OPEN_FILE_FOLDER, e.data.path);

                        e.preventDefault();
                        return false;
                });

                    $listRecentDirs.append($item);
                }
            }

            if ($listRecentDirs.length) {
                this.dndZone.hide();
                $('#btn-openlocal').show();
            }
        };

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();

                this.dndZone = new DnDFileZone();
                this.dndZone.render(this.view.$panel.find("#area-dnd-file"));

                $('#btn-openlocal').hide();

                this.view.$panel.find('#btn-openlocal').click(()=>{
                    openFile(OPEN_FILE_FOLDER, '');
                });

                // window.CommonEvents.on("icons:svg", (pasteSvg)=>{
                //     let emptylist = $('[class*="text-emptylist"]', '#box-recent-folders');
                //     emptylist.toggleClass('text-emptylist text-emptylist-svg');
                //     if(pasteSvg && !emptylist.find('svg').length)
                //         emptylist.prepend($('<svg class = "empty-folder"><use xlink:href="#folder-big"></use></svg>'));
                // });

                window.sdk.on('onupdaterecents', _on_update.bind(this));

                return this;
            }
        };
    })());
}();

/*
*   controller definition
*/

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerFolders({});
//     p.init();
// });