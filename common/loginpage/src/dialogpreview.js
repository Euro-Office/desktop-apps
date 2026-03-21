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