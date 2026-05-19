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

#include "casctabdata.h"
#include "cproviders.h"
#include <QJsonDocument>
#include <QJsonObject>

using namespace std;

CAscTabData::CAscTabData(const QString& t, CefType wt)
    : _title(t)
    , _is_local(false)
    , _vtype(wt)
    , _url()
    , _typeContent(AscEditorType::etUndefined)
{}

CAscTabData::CAscTabData(const QString& t, AscEditorType ct)
    : _title(t)
    , _url()
    , _typeContent(ct)
{
    switch (ct) {
    case AscEditorType::etDocument:
    case AscEditorType::etSpreadsheet:
    case AscEditorType::etPresentation:
    case AscEditorType::etPdf:
    case AscEditorType::etDraw:
        _vtype = cvwtEditor;
        break;
    default:
        _vtype = cvwtSimple;
        break;
    }
}

void CAscTabData::setTitle(const QString& t)
{
    _title = t;
}

QString CAscTabData::title(bool orig) const
{
    if ( orig )
        return _title;
    else {
        QString out{_title};
        if ( _has_changes )
            out.prepend("*");

        if ( _is_readonly ) {
            out.append(_str_readonly);
        }

        return out;
    }
}

void CAscTabData::setChanged(bool s)
{
    _has_changes = s;
    s && !_is_changed && (_is_changed = true);
}

bool CAscTabData::modified() const
{
    return _is_changed;
}

bool CAscTabData::hasChanges() const
{
    return _has_changes;
}

void CAscTabData::close() {
    _is_closed = true;
}

void CAscTabData::reuse()
{
    _is_closed = false;
}

bool CAscTabData::closed() const
{
    return _is_closed;
}

CefType CAscTabData::viewType() const
{
    return _vtype;
}

void CAscTabData::setUrl(const wstring& u)
{
    _url = u;
}

void CAscTabData::setUrl(const QString& u)
{
    setUrl(u.toStdWString());
}

void CAscTabData::setCloudName(const QString &cloud)
{
    _cloud = cloud;
}

wstring CAscTabData::url() const
{
    return _url;
}

bool CAscTabData::isViewType(CefType vt) const
{
    return vt == _vtype;
}

AscEditorType CAscTabData::contentType() const
{
    return _typeContent;
}

void CAscTabData::setContentType(AscEditorType t)
{
    _typeContent = t;
}

void CAscTabData::setIsLocal(bool l)
{
    _is_local = l;
}

bool CAscTabData::isLocal() const
{
    return _is_local;
}

bool CAscTabData::eventLoadSupported() const
{
    return _event_load_supported;
}

void CAscTabData::setHasError()
{
    _has_error = true;
}

void CAscTabData::setEventLoadSupported(bool value)
{
    _event_load_supported = value;
}

void CAscTabData::setFeatures(const wstring& fs)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdWString(fs).toUtf8(), &err);
    if (err.error == QJsonParseError::NoError) {
        QJsonObject obj = doc.object();
        if (_features.empty()) {
            _features = QString(QJsonDocument(obj).toJson(QJsonDocument::Compact)).toStdWString();
//            qDebug() << QString::fromStdWString(_features);
        } else {
            QJsonDocument src_doc = QJsonDocument::fromJson(QString::fromStdWString(_features).toUtf8(), &err);
            if (err.error == QJsonParseError::NoError) {
                QJsonObject src_obj = src_doc.object();
                QVariantMap map = src_obj.toVariantMap();
#if (QT_VERSION < QT_VERSION_CHECK(5,15,0))
                QVariantMap fs_map = obj.toVariantMap();
                for (QVariantMap::iterator it = fs_map.begin(); it != fs_map.end(); it++)
                    map.insert(it.key(), it.value());
#else
                map.insert(obj.toVariantMap());
#endif
                QJsonObject res = QJsonObject::fromVariantMap(map);
                _features = QString(QJsonDocument(res).toJson(QJsonDocument::Compact)).toStdWString();
//                qDebug() << QString::fromStdWString(_features);
            } else {
//                qDebug() << "JSON has error: " << QString::fromStdWString(_features);
            }
        }
    } else {
//        qDebug() << "JSON has error: " << QString::fromStdWString(fs);
    }

    if ( hasFeature(L"readonly\":true") ) {
        _is_readonly = true;

        if ( _str_readonly.isEmpty() )
            _str_readonly = " (" + QObject::tr("Read only") + ")";
    } else
    if ( hasFeature(L"readonly\":false") ) {
        _is_readonly = false;
    }
}

wstring CAscTabData::features() const
{
    return _features;
}

bool CAscTabData::hasFeature(const wstring& f) const
{
    size_t _pos;
    if ((_pos = _features.find(f)) != wstring::npos) {
//        if (_features.find(L"true", _pos + 1) != wstring::npos)
            return true;
    }

    return false;
}

bool CAscTabData::hasFrame() const
{
    return CProviders::instance().editorsHasFrame(QString::fromStdWString(_url), _cloud);
}

bool CAscTabData::hasError() const
{
    return _has_error;
}
