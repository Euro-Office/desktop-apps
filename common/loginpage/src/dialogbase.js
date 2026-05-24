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

// +function() {
{
    function dialog(params = {}) {
        this.events = { close: params.onclose };
        this.dialogClass = params.dialogClass || '';
        this.titleText = params.titleText || '';
        this.bodyTemplate = params.bodyTemplate || '';
        this.defaultWidth = params.defaultWidth || 500;

        this.$el = null;
        this.$title = null;
        this.$body = null;
    }

    dialog.prototype.template = function() {
        return `<dialog class="dlg ${this.dialogClass}">
            <div class="title">
                <label class="caption">${this.titleText}</label>
                <span class="tool close"></span>
            </div>
            <div class="body">${this.bodyTemplate}</div>
        </dialog>`;
    };

    dialog.prototype.show = function(width) {
        this.$el = $('#placeholder').append(this.template()).find(`.${this.dialogClass}`);
        this.$el.width(width || this.defaultWidth);

        this.$title = this.$el.find('.title');
        this.$body = this.$el.find('.body');

        if (this.bodyTemplate)  this.$body.html(this.bodyTemplate);

        this.$title.find('.tool.close').on('click', () => this.close());
        this.$el.on('close', () => this.close());

        this.$el.get(0).showModal();
        this.$el.addClass('scaled');
    };

    dialog.prototype.close = function(opts) {
        this.$el.remove();

        if (this.events.close) this.events.close(opts);
    };

    dialog.prototype.setBody = function(html) {
        if (this.$body) {
          this.$body.html(html);
        }
    };

    dialog.prototype.getElements = function() {
        return {
            $el: this.$el,
            $title: this.$title,
            $body: this.$body
        };
    };

    window.Dialog = dialog;
}
// }();