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

#ifndef CASCTABDATA_H
#define CASCTABDATA_H


#include <QString>
#include "qcefview.h"

typedef CefViewWrapperType CefType;

struct CAscTabData
{
public:
    CAscTabData(const QString &, CefType wt = cvwtEditor);
    CAscTabData(const QString &, AscEditorType ct);
    ~CAscTabData() {}

    void    setTitle(const QString&);
    void    setChanged(bool);
    void    setIsLocal(bool);
    void    setUrl(const std::wstring&);
    void    setUrl(const QString&);
    void    setCloudName(const QString&);
    void    close();
    void    reuse();
    QString title(bool orig = false) const;
    bool    modified() const;
    bool    hasChanges() const;
    bool    closed() const;
    bool    isLocal() const;
    CefType viewType() const;
    std::wstring url() const;
    bool    isViewType(CefType) const;
    bool    eventLoadSupported() const;
    void    setHasError();
    void    setEventLoadSupported(bool);
    void    setFeatures(const std::wstring&);
    std::wstring features() const;
    bool    hasFeature(const std::wstring&) const;
    bool    hasFrame() const;
    bool    hasError() const;

    AscEditorType   contentType() const;
    void            setContentType(AscEditorType);
private:
    QString _title;
    bool    _is_changed = false,
            _is_readonly = false,
            _has_changes = false;
    bool    _is_closed = false;
    bool    _is_local;
    bool    _has_error = false;
    CefType _vtype;
    std::wstring _url;
    bool    _event_load_supported = false;
    std::wstring _features;
    QString _str_readonly;
    QString _cloud;

    AscEditorType _typeContent;
};

#endif // CASCTABDATA_H
