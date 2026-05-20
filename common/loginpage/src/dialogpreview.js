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

function PreviewTemplateDialog(model, params = {}) {
    const type = utils.parseFileFormat(model.type);
    const size = formatSize(model.size);

    const bodyTemplate = `
        <div class="template-preview-body">
            <div class="img-container">
                <img class='icon--default' src="${model.icon}">
                <img class="icon" src="${model.preview}" style="display:none;">
            </div>
            <div class="description">
                <h3 class="name">${model.name}</h3>
                <p class="pricing" l10n>${utils.Lang.tplFree}</p>
                <p class="descr">${model.descr}</p>
                <div class="file-info separator">
                    <div>
                        <span class="label" l10n>${utils.Lang.tplFileSize}:</span>
                        <span class="value">${size}</span>
                    </div>
                    <div>
                        <span class="label" l10n>${utils.Lang.tplFileType}:</span>
                        <span class="value">${type}</span>
                    </div>
                </div>
                <button class="btn btn--landing" l10n>${utils.Lang.tplUseTemplate}</button>
            </div>
        </div>
    `;

    function formatSize(size) {
        if (!size) return '';
        if (size < 1024) {
            return Math.round(size) + ' kb';
        } else {
            return Math.round(size / 1024) + ' mb';
        }
    }

    Object.assign(params, {
        dialogClass: 'dlg-template-preview',
        titleText: utils.Lang.actPreviewTemplates,
        defaultWidth: 800,
        bodyTemplate: bodyTemplate
    });

    Dialog.call(this, params);
    this.model = model;
}

PreviewTemplateDialog.prototype = Object.create(Dialog.prototype);
PreviewTemplateDialog.prototype.constructor = PreviewTemplateDialog;

PreviewTemplateDialog.prototype.show = function(width) {
    Dialog.prototype.show.call(this, width);

    const {$el} = this.getElements();
    const $img = $el.find('img.icon');
    const $icon = $el.find('img.icon--default');

    $img.on('load', () => {
        $icon.hide();
        $img.show();
    });

    $el.find('.btn.btn--landing').on('click', () => {
        this.close();
        window.sdk.openTemplate(this.model.path, this.model.fullName);
    });
};

window.PreviewTemplateDialog = PreviewTemplateDialog;