// +function() {
  function AboutDialog(params = {}) {
    Object.assign(params, { dialogClass: 'dlg-about',
                            titleText: utils.Lang.actAbout,
                            defaultWidth: 570,
                            onclose: params.onclose,
                        });
    Dialog.call(this, params);
  }

  AboutDialog.prototype = Object.create(Dialog.prototype);
  AboutDialog.prototype.constructor = AboutDialog;

  // window.AboutDialog = AboutDialog;
// }();