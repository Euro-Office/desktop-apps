// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

// src/panelconnect.js: index.html shows the SVG icon from 1 to 1.24 and from
// 2 up, and the PNG fallback in between. Both must be in the markup at any
// pixel ratio, or a ratio like 1.1 (110 % scaling) shows neither, and the
// "+" next to Clouds is blank.

import { beforeAll, describe, expect, it } from 'vitest';
import { loadScripts } from './load.js';

let view;

beforeAll(() => {
    Object.defineProperty(window, 'devicePixelRatio', {value: 1.1, configurable: true});
    window.AscDesktopEditor = {
        execCommand: () => {},
        GetExternalClouds: () => [],
        GetCryptoMode: () => 0,
        GetSupportCryptoModes: () => [],
    };
    loadScripts('vendor/jquery/jquery.min.js', 'vendor/bootstrap/dropdown.js',
                'vendor/bootstrap-select/js/bootstrap-select.js', 'src/utils.js', 'src/locale.js', 'src/events.js',
                'src/sdk.js', 'src/model.js', 'src/menu.js', 'src/base.js', 'src/dialogbase.js',
                'src/panelconnect.js');
    view = new ControllerPortals({}).view;
});

describe('icons in the Clouds panel at a pixel ratio of 1.1', () => {
    it('renders the "+" as SVG and as PNG fallback', () => {
        const $add = $(view.tpl_sidebar).find('.btn-quick.login');
        expect($add.find('svg.icon use').attr('xlink:href')).toBe('#plus');
        expect($add.find('i.icon.plus')).toHaveLength(1);
    });

    it('renders the "more" button of a cloud as SVG and as PNG fallback', () => {
        const $row = $(`<table><tr>${view.portaltemplate({iconid: 'nextcloud', portal: 'https://cloud.example.com', user: 'alice', email: ''})}</tr></table>`);
        expect($row.find('.btn-quick.more svg.icon')).toHaveLength(1);
        expect($row.find('.btn-quick.more i.icon.more')).toHaveLength(1);
    });
});
