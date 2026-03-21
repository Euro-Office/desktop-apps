window.DnDFileZone = function () {
	"use strict";

	let $el, $parent;

	//language=HTML
	const _template = `
        <div class="dnd-zone">
            <div class="content">
                <p class="text-normal" l10n>${utils.Lang.labelDropFile}</p>
                <button class="btn btn--primary" l10n>${utils.Lang.labelSelectFile}</button>
            </div>
        </div>`;


	return {
		render: function (parentElement) {
			$parent = parentElement;
			$el = $parent.append(_template).find('.dnd-zone');

			$el.find('button').bind('click', function () {
				openFile(OPEN_FILE_FOLDER, '');
			});
		},
		detach: function () {
			$el.remove();
		},
		hide: function () {
			$parent.hide()
		},
		show: function () {
			$parent.show()
		}
	}
};