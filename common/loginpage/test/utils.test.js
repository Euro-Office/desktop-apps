// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

// src/utils.js: URL helpers, file format mapping, the stored portals list,
// pinned folders, provider ordering and parsing of the recent files list.

import { beforeAll, beforeEach, describe, expect, it } from 'vitest';
import { loadUtils } from './load.js';

let utils, FileFormat;

beforeAll(() => {
    utils = loadUtils();
    FileFormat = utils.defines.FileFormat;
});

beforeEach(() => {
    localStorage.clear();
});

describe('URL helpers', () => {
    it('skipUrlProtocol drops the scheme and stops at the query', () => {
        expect(utils.skipUrlProtocol('https://cloud.example.com/index.php?x=1')).toBe('cloud.example.com/index.php');
        expect(utils.skipUrlProtocol('cloud.example.com')).toBe('cloud.example.com');
    });

    it('getUrlProtocol returns the scheme with slashes, if any', () => {
        expect(utils.getUrlProtocol('http://cloud.example.com')).toBe('http://');
        expect(utils.getUrlProtocol('https://cloud.example.com')).toBe('https://');
        expect(utils.getUrlProtocol('cloud.example.com')).toBeUndefined();
    });

    it('inParams defaults the language to English', () => {
        expect(utils.inParams.lang).toBe('en');
    });
});

describe('file formats', () => {
    it('formatToEditor picks the editor for a format', () => {
        expect(utils.formatToEditor(FileFormat.FILE_DOCUMENT_DOCX)).toBe('word');
        expect(utils.formatToEditor(FileFormat.FILE_SPREADSHEET_XLSX)).toBe('cell');
        expect(utils.formatToEditor(FileFormat.FILE_PRESENTATION_PPTX)).toBe('slide');
        expect(utils.formatToEditor(FileFormat.FILE_CROSSPLATFORM_PDF)).toBe('pdf');
        expect(utils.formatToEditor(FileFormat.FILE_DOCUMENT_OFORM_PDF)).toBe('pdf');
    });

    it.each(['doc', 'docx', 'odt', 'rtf', 'txt', 'html', 'mht', 'epub', 'fb2', 'docm',
             'dotx', 'ott', 'oform', 'docxf', 'pdf', 'fodt', 'dotm', 'xml',
             'xls', 'xltx', 'xlsx', 'xlsb', 'ods', 'csv', 'ots', 'xltm', 'xlsm', 'fods',
             'ppt', 'potx', 'pptx', 'odp', 'ppsx', 'otp', 'pptm', 'ppsm', 'potm', 'fodp',
             'djvu', 'xps'])(
        'extension %s maps to a format and back', ext => {
            const format = utils.fileExtensionToFileFormat(ext);
            expect(format).not.toBe(FileFormat.FILE_UNKNOWN);
            expect(utils.parseFileFormat(format)).toBe(ext);
        });

    it('fileExtensionToFileFormat accepts a leading dot', () => {
        expect(utils.fileExtensionToFileFormat('.docx')).toBe(FileFormat.FILE_DOCUMENT_DOCX);
    });

    it('fileExtensionToFileFormat returns FILE_UNKNOWN for unknown extensions', () => {
        expect(utils.fileExtensionToFileFormat('exe')).toBe(FileFormat.FILE_UNKNOWN);
    });

    it('parseFileFormat names spreadsheet, presentation and drawing formats', () => {
        expect(utils.parseFileFormat(FileFormat.FILE_SPREADSHEET_XLSX)).toBe('xlsx');
        expect(utils.parseFileFormat(FileFormat.FILE_PRESENTATION_PPTX)).toBe('pptx');
        expect(utils.parseFileFormat(FileFormat.FILE_CROSSPLATFORM_DJVU)).toBe('djvu');
        expect(utils.parseFileFormat(FileFormat.FILE_DRAW_VSTX)).toBe('vsdx');
        expect(utils.parseFileFormat(FileFormat.FILE_DRAW_VSDX)).toBe('vsdx');
    });

    it('parseFileFormat falls back to neutral', () => {
        expect(utils.parseFileFormat(-1)).toBe('neutral');
    });
});

describe('PortalsStore', () => {
    it('lists kept portals, newest first', () => {
        PortalsStore.keep({ portal: 'a.example.com', user: 'A' });
        PortalsStore.keep({ portal: 'b.example.com', user: 'B' });

        expect(PortalsStore.portals().map(p => p.portal)).toEqual(['b.example.com', 'a.example.com']);
    });

    it('keeping a known portal replaces it and makes it the newest', () => {
        PortalsStore.keep([{ portal: 'a.example.com', user: 'A' }, { portal: 'b.example.com', user: 'B' }]);
        PortalsStore.keep({ portal: 'a.example.com', user: 'A2' });

        const portals = PortalsStore.portals();
        expect(portals).toHaveLength(2);
        expect(portals[0]).toEqual({ portal: 'a.example.com', user: 'A2' });
        expect(PortalsStore.get({ portal: 'a.example.com' }).user).toBe('A2');
    });

    it('get returns undefined for unknown portals', () => {
        expect(PortalsStore.get({ portal: 'unknown.example.com' })).toBeUndefined();
        expect(PortalsStore.get({})).toBeUndefined();
    });

    it('forget removes a portal once', () => {
        PortalsStore.keep({ portal: 'a.example.com' });

        expect(PortalsStore.forget('a.example.com')).toBe(true);
        expect(PortalsStore.forget('a.example.com')).toBe(false);
        expect(PortalsStore.portals()).toEqual([]);
    });
});

describe('pinned folders', () => {
    it('toggle pins and unpins a folder', () => {
        expect(utils.fn.pinnedFolders('/home/me/docs', 'check')).toBe(false);

        utils.fn.pinnedFolders('/home/me/docs', 'toggle');
        expect(utils.fn.pinnedFolders('/home/me/docs', 'check')).toBe(true);

        utils.fn.pinnedFolders('/home/me/docs', 'toggle');
        expect(utils.fn.pinnedFolders('/home/me/docs', 'check')).toBe(false);
    });
});

describe('sortProviders', () => {
    it('puts onlyoffice first, then by order, providers without order last', () => {
        const list = [
            { provider: 'c' },
            { provider: 'b', order: '2' },
            { provider: 'onlyoffice', order: '9' },
            { provider: 'a', order: 1 },
        ];

        expect(utils.fn.sortProviders(list).map(p => p.provider)).toEqual(['onlyoffice', 'a', 'b', 'c']);
        // the input stays as it was
        expect(list[0].provider).toBe('c');
    });

    it('handles a missing list', () => {
        expect(utils.fn.sortProviders(undefined)).toEqual([]);
    });
});

describe('decodeHtml', () => {
    it('decodes entities', () => {
        expect(utils.fn.decodeHtml('&lt;b&gt; &amp; &quot;x&quot;')).toBe('<b> & "x"');
    });
});

describe('parseRecent', () => {
    const recent = [
        { id: 1, path: '/home/me/docs/report.docx', type: 0x0041, modifyed: '01.01.2026', pin: false },
        { id: 2, path: '/home/me/sheets/budget.xlsx', type: 0x0101, modifyed: '02.01.2026', pin: true },
        { id: 3, path: '/home/me/no-extension', type: 0, modifyed: '', pin: false },
    ];

    it('splits paths into name and folder and skips files without extension', () => {
        const files = utils.fn.parseRecent(recent);

        expect(files).toHaveLength(2);
        expect(files[0]).toMatchObject({
            fileid: 1, name: 'report.docx', descr: '/home/me/docs',
            path: '/home/me/docs/report.docx', format: 'docx', pinned: false, pinid: 1,
        });
    });

    it('marks pinned files with a negative pin id', () => {
        const files = utils.fn.parseRecent(recent);

        expect(files[1]).toMatchObject({ fileid: 2, name: 'budget.xlsx', format: 'xlsx', pinned: true, pinid: -2 });
    });
});
