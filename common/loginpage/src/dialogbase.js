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