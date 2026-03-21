"use strict";

/**
 * Document Creation Grid - Shows cards for creating new documents
 *
 * @param {Object} config
 * @param {Array} config.documentTypes - Document types to show
 * @param {Function} config.onDocumentSelect - Called when user selects a document
 *
 * Document Type Object:
 * - id: unique identifier
 * - title: display name
 * - formatLabel: { value, gradientColorStart, gradientColorEnd }
 * - icon: icon reference (e.g., '#docx-big')
 *
 * @example
 * const docGrid = new DocumentCreationGrid({
 *   documentTypes: [
 *     {
 *       id: 'docx',
 *       title: 'Word Document',
 *       formatLabel: {
 *         value: 'DOCX',
 *         gradientColorStart: '#4298C5',
 *         gradientColorEnd: '#2D84B2'
 *       },
 *       icon: '#docx-big'
 *     }
 *   ],
 *   onDocumentSelect: (docType) => console.log(docType)
 * });
 */
window.DocumentCreationGrid = function (config = {}) {
	const {
		documentTypes = [],
		onDocumentSelect = null
	} = config;


	let $el, $parent;

	/**
	 * Global handler for document selection
	 * @param {string} docType - Selected document type ID
	 * @param {HTMLElement} element - Clicked element
	 */
	window.DocumentCreationGrid.handleDocumentSelect = function (docType, element) {
		const selectedDoc = documentTypes.find(doc => doc.id === docType);

		if (!selectedDoc) {
			console.warn('Document type not found:', docType);
			return;
		}

		if (onDocumentSelect && typeof onDocumentSelect === 'function') {
			onDocumentSelect(docType, selectedDoc, element);
		}
	};

	return {
		/**
		 * Renders the component in the specified parent element
		 * @param {jquery} parentElement - Parent element to render the component in
		 */
		render: (parentElement) => {
			if (!parentElement) {
				throw new Error('Parent element is required for rendering');
			}

			//language=HTML
			const _template = `
                <div class="document-creation-grid">
                    ${documentTypes.map(doc => `
                    <div class="document-creation-item" data-id="${doc.id}" onclick="window.DocumentCreationGrid.handleDocumentSelect('${doc.id}', this)">

                        <div class="format-label" style="--format-bg-start: ${doc.formatLabel.gradientColorStart};
														--format-bg-end: ${doc.formatLabel.gradientColorEnd};
														--format-bg-winxp: ${doc.formatLabel.bgColorWinXP}">
                            <span>${doc.formatLabel.value}</span>
                        </div>

						${doc.icon.startsWith('#') ? `<svg class="icon"><use xlink:href="${doc.icon}"></use></svg>` : `<i class="icon ${doc.icon}"></i>`}

                        <div class="title" l10n="${doc.langKey}">
                            ${doc.title}
                        </div>
                    </div>
                `).join('')}
                </div>`;

			$parent = parentElement;
			$el = $parent.append(_template).find('.document-creation-grid');
		},
	};
};