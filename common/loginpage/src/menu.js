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

'use strict';

var Menu = function(args) {
    this.config = Object.assign({}, args);
    this.id = args.id;
    this.items = args.items;
    this.className = args.className;

    this.prefix = args.prefix || 'asc-gen';
    this.events = {};
    this.events.itemclick = new ModelEvent(this);
};

Menu.prototype.init = function(parent) {
    var me = this;

    //language=HTML
    var _tpl_ = `
        <div id="%id" class="menu-container">
            <div class="dropdown-toggle" data-toggle="dropdown"></div>
            <ul class="dropdown-menu ${this.className || ''}" role="menu"></ul>
        </div>`;

    var $container = $(_tpl_.replace(/\%id/, this.id)).appendTo(parent);
    var $list = $container.find('ul');

    var _tpl_item_ = '<li><a id="%id" class="dd-item" tabindex="-1" type="menuitem" l10n>%icon%caption</a></li>',
        _tpl_divider_ = '<li class="divider"></li>';

    this.items.forEach(function(item) {
        let $item;

        !item.id && (item.id = me.prefix + ++nCounter);
        if (item.caption === '--') {
            $item = $(_tpl_divider_);
        } else {
            let iconHtml = '';
            if (item.icon) {
                if (item.icon.startsWith('#')) {
                    iconHtml = `<svg class="menu-icon"><use xlink:href="${item.icon}"></use></svg>`;
                } else {
                    iconHtml = `<i class="menu-icon ${item.icon}"></i>`;
                }
            }

            $item = $(_tpl_item_
                .replace(/%caption/, item.caption)
                .replace(/%id/, item.id)
                .replace(/%icon/, iconHtml));

            if (item.variant) {
                $item.find('a').attr('data-variant', item.variant);
            }

            $item.on('click', {'action': item.action}, function(e) {
                if ( !e.target.hasAttribute('disabled') )
                    me.events.itemclick.notify(e.data['action'], me.contextdata);
            });
        }

        $list.append($item);
    });

    $container.on({
        'hide.bs.dropdown': function(){
            // console.log('before hide menu');
        },
        'hidden.bs.dropdown': function() {
            me.contextdata = [];
            // console.log('after hide menu');
        }
    });
};

Menu.prototype.show = function(pos, data) {
    $('.menu-container').removeClass('open');
    let $el = $('#'+this.id);

    let $dd = $el.find('.dropdown-menu');
    let dd_width = $dd.outerWidth();
    let dd_height = $dd.outerHeight();

    let scrollLeft = $(window).scrollLeft();
    let viewportWidth = $(window).width();
    let _right = dd_width + pos.left;
    if (_right > scrollLeft + viewportWidth) {
        pos.left = pos.left - dd_width - 4;
    }

    let scrollTop = $(window).scrollTop();
    let viewportHeight = $(window).height();
    let visible_bottom = scrollTop + viewportHeight;
    if (!!this.config.bottomlimitoffset) {
        visible_bottom -= this.config.bottomlimitoffset;
    }

    let _bottom = dd_height + pos.top;
    if (_bottom > visible_bottom) {
        pos.top = pos.top - dd_height - 4;
    }

    $el.css(pos);
    $dd.dropdown('toggle');
    this.contextdata = data;
    Menu.opened = true;
};

Menu.prototype.showUnderElem = function(el, data, align) {
    $('.menu-container').removeClass('open');
    let $el = $('#'+this.id);
    const $rel = $(el);

    const pos = $rel.offset();
    const $dd = $el.find('.dropdown-menu');

    let topPos = pos.top + $rel.outerHeight() + 2;

    const dropdownHeight = $dd.outerHeight();
    const viewportHeight = $(window).height();
    const scrollTop = $(window).scrollTop();

    if ((topPos + dropdownHeight) > (viewportHeight + scrollTop)) {
        topPos = pos.top - dropdownHeight - 2;
    }

    pos.top = topPos;

    if (align === 'right') {
        pos.left = pos.left - ($dd.outerWidth() - $rel.outerWidth());
    }

    $el.css(pos);
    $dd.dropdown('toggle');
    this.contextdata = data;
    Menu.opened = true;
};

Menu.prototype.disableItem = function(action, disable) {
    for (let item of this.items) {
        if (item.action == action) {
            let $el = $('#' + item.id);
            if ( disable ) {
                $el.attr('disabled', 'disabled');
                $el.parent()['addClass']('disabled');
            } else {
                !!$el.attr('disabled') && $el.removeAttr('disabled');
                $el.parent()['removeClass']('disabled');
            }
        }
    }
};

Menu.prototype.hideItem = function(action, hide) {
    for (let item of this.items) {
        if (item.action == action) {
            $('#' + item.id).parent()
                [hide?'hide':'show']();
        }
    }
};

Menu.opened = false;

Menu.closeAll = function() {
    $('.menu-container.open').removeClass('open');
    Menu.opened = false;
};

window.Menu = Menu;

$(document).on('keydown', function(e) {
    if (e.keyCode == 27) {
        // $('.menu-container').removeClass('open');
        Menu.closeAll();
    }
});

$(window).resize(function(){
    Menu.closeAll();
});
