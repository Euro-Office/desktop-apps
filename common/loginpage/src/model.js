/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

'use strict';

var nCounter = 0;
function ModelEvent(sender) {
    this._sender = sender;
    this._listeners = [];
}

ModelEvent.prototype = {
    attach : function (listener) {
        return this._listeners.push(listener);
    },
    detach : function (value) {
        if ( !Number.isInteger(value) )
            value = this._listeners.indexOf(value);

        if ( !(value < 0) ) {
            this._listeners.splice(value, 1);
        }
    },
    clear: function() {
        this._listeners = [];
    },
    notify : function (args) {
        var index;
        var _args = Array.from(arguments);
        _args.unshift(this._sender);

        for (index = 0; index < this._listeners.length; index += 1) {
            // this._listeners[index](this._sender, args);
            this._listeners[index].apply(this, _args);
        }
    }
};

function Collection(attributes) {
    this.items = [];
    this.view = attributes.view;
    this.list = attributes.list;

    var _time = Date.now();
    this.on_item_changed = function(model, value) {
        this.events.changed.notify(model, value);
    }.bind(this);

    this.on_item_click = function(e) {
        Menu.opened && Menu.closeAll();

        if (Date.now()-_time > 800) {
            _time = Date.now();
            this.events.click.notify(e.data);
        }

        e.preventDefault();
        return false;
    }.bind(this);

    this.on_item_ctxmenu = function(e) {
        this.events.contextmenu.notify(e.data, e);
        e.preventDefault();
    }.bind(this);

    this.events = {};
    this.events.changed = new ModelEvent(this);
    this.events.erased = new ModelEvent(this);
    this.events.inserted = new ModelEvent(this);
    this.events.reset = new ModelEvent(this);
    this.events.click = new ModelEvent(this);
    this.events.contextmenu = new ModelEvent(this);
};

Collection.prototype.add = function(item, suppressevent) {
    const _add_model = (m, se) => {
        m.events.changed.attach(this.on_item_changed);

        this.items.push(m);

        // if ( !(suppressevent === true) )
        //     this.events.inserted.notify(m);

        // $('#' + m.uid).off('click contextmenu')
        //                 .on('click', m, this.on_item_click)
        //                 .on('contextmenu', m, this.on_item_ctxmenu);
    }

    if ( item instanceof Array ) {
        const items = item;
        items.forEach(i => {
            _add_model(i, true);
        });

        // if ( !(suppressevent === true) )
        this.events.inserted.notify(items);

        items.forEach(i => {
            $('#' + i.uid).off('click contextmenu')
                            .on('click', i, this.on_item_click)
                            .on('contextmenu', i, this.on_item_ctxmenu);
        });
    } else {
        _add_model(item)

        // if ( !(suppressevent === true) )
        this.events.inserted.notify(item);

        $('#' + item.uid).off('click contextmenu')
                        .on('click', item, this.on_item_click)
                        .on('contextmenu', item, this.on_item_ctxmenu);
    }
};

Collection.prototype.set = function(items) {
    if ( items instanceof Array ) {
        this.empty(true);

        items.forEach(i => {
            this.add(i, true);
        });

        // this.items = items;
        this.events.reset.notify(items);
    }
}

Collection.prototype.find = function(key, val) {
    return this.items.find(function(elem, i, arr){
        return elem[key] == val;
    });
};

Collection.prototype.empty = function(suppressevent) {
    this.items.forEach(function(model, i, a) {
        $('#'+model.uid).off();
    });

    this.items = [];

    if (!!this.list) this.view.find(this.list).empty();
    
    if ( !(suppressevent === true) )
        this.events.erased.notify();
};

Collection.prototype.size = function() {
    return this.items.length;
};

Collection.prototype.stringify = function() {
//    let narray = this.items.map(s => {
//        const {events, ...y} = s;    ancient chromium doesn't fully support 'spread' operator
//        return y;
//    });
//    return JSON.stringify(narray);
    return '';
};

function Model(attributes) {
    var attr = attributes || {};

    this.prefix = attr.prefix || 'asc-gen';
    this.uid = this.prefix + ++nCounter;
    this.events = {changed: new ModelEvent(this)};
};

Model.prototype.set = function(key, value, opts) {
    this[key] = value;

    let args = {};
    args[key] = value;

    if ( !opts || opts.silent !== true )
        this.events.changed.notify(args);
};

Model.prototype.setMany = function(args, opts) {
    for (const [key, value] of Object.entries(args)) {
        this[key] = value;
    }

    if (!opts || opts.silent !== true) {
        this.events.changed.notify(args);
    }
};

Model.prototype.get = function(key) {
    return this[key];
};

function PortalModel(attributes) {
    Model.prototype.constructor.call(this, {prefix: 'asc-portal-'});

    let _back_compat_provider = p => {
        switch (p) {
        case 'asc': return 'onlyoffice';
        case 'ownc': return 'owncloud';
        default: return p;
        }
    };

    this.name   = attributes.portal && utils.skipUrlProtocol(attributes.portal);
    this.path   = attributes.portal || '';
    this.logged = false;
    this.user   = attributes.user || '';
    this.email  = attributes.email || '';
    this.provider = _back_compat_provider(attributes.provider) || 'onlyoffice';
};

PortalModel.prototype = Object.create(Model.prototype); /*new Model();*/
PortalModel.prototype.constructor = PortalModel;

function FileModel(attributes) {
    Model.prototype.constructor.call(this);
    Object.assign(this, attributes);

    this.name   = attributes.name || '';
    this.descr  = attributes.descr || '';
    this.exist  = true;
    this.islocal = !/^https?:\/\//.test(this.path)
};

FileModel.prototype = new Model();
FileModel.prototype.constructor = FileModel;
