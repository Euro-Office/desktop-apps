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

/*
*   base controller declaration
*/

+function() {
    var controller = function(args) {
        this.$menuitem = this.$panel = undefined;
    };

    controller.prototype.view = undefined;
    controller.prototype.init = function() {
        this.view && this.view.init();
    };

    var view = function(args) {
        this.rendered = false;

        var _action = args.action ? `action="${args.action}"` : '';
        var _itemcls = 'menu-item' + (args.itemcls?` ${args.itemcls}`:'');

        this.tplPage = args.tplPage || '<div class="center-panel">Hello, Word!</div>';
        this.tplItem = args.tplItem || `<li class="${_itemcls}"><a l10n ${_action}>${args.itemtext}</a></li>`;
        this.menuContainer = args.menu || '';
        this.panelContainer = args.field || '';
        this.opts = args;
    };

    view.prototype.init = function() {
    };

    view.prototype.render = function() {
        if (!this.rendered) {
            this.rendered = true;

            let _index = this.opts.itemindex;
            this.$menuitem = this.renderMenuItem.call(this, this.tplItem, _index);

            if (this.tplPage != 'empty') {
                let $parentview = $(this.panelContainer);
                if (_index >= 0) {
                    let $panels = $parentview.children();
                    this.$panel = $panels.size() > _index ?
                        $(this.tplPage).insertBefore($panels.eq(_index)) : $(this.tplPage).appendTo($parentview);

                        let a = $panels.eq(_index);
                        let b = 1;
                } else {
                    this.$panel = $(this.tplPage).appendTo($parentview);
                }
            }
        }
    };

    view.prototype.renderMenuItem = function(node, index) {
        let $itemout;
        if (node != 'nomenuitem') {
            $itemout = $(node);

            let _ib = utils.fn.getToolMenuItemOrder($itemout);
            if ( !_ib.item ) {
                let $menu = $(this.menuContainer);
                $itemout.appendTo($menu);
            } else {
                _ib.after ? $itemout.insertAfter(_ib.item) : $itemout.insertBefore(_ib.item);
            }
        }

        return $itemout;
    };

    window.baseView = view;
    window.baseController = controller;
}();

