+function() {
    function dialogProviders(params) {
        params = params || {};
        Dialog.call(this, {
            dialogClass: 'dlg-providers',
            titleText: utils.Lang.loginTitleStart,
            bodyTemplate: params.bodyTemplate || '<div/>',
            defaultWidth: 590,
            onclose: params.onclose
        });

        this.connectHandler = params.onConnect || (() => {});
    }

    dialogProviders.prototype = Object.create(Dialog.prototype);
    dialogProviders.prototype.constructor = dialogProviders;

    dialogProviders.prototype.show = function(width) {
        Dialog.prototype.show.call(this, width);

        this.$body.on('click', '.link-connect-now, .link, .btn.login', (e) => {
            this.close();
            this.connectHandler(e, $(e.currentTarget).data('cprov'));
        });
    }

    window.DialogProviders = dialogProviders;
}();