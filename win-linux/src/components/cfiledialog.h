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

#ifndef CFILEDIALOG_H
#define CFILEDIALOG_H

#include <QObject>
#include <QMap>


class CFileDialogWrapper : public QObject
{
    Q_OBJECT

public:
    explicit CFileDialogWrapper(QWidget * p = 0);
    ~CFileDialogWrapper();

    bool    modalSaveAs(QString&, int selected = -1);

//    QString modalOpen(const QString&, const QString& filter = QString(), QString * selectedFilter = Q_NULLPTR);
    QStringList modalOpen(const QString&, const QString& filter = QString(), QString * selectedFilter = Q_NULLPTR, bool multi = false);
    QString     modalOpenSingle(const QString&, const QString& filter = QString(), QString * selectedFilter = Q_NULLPTR);

    QStringList modalOpenImage(const QString&);
    QStringList modalOpenImages(const QString&);
    QStringList modalOpenPlugin(const QString&);
    QStringList modalOpenPlugins(const QString&);
    QStringList modalOpenAny(const QString&, bool multi = false);
    QStringList modalOpenDocuments(const QString&, bool multi = false);
    QStringList modalOpenSpreadsheets(const QString&, bool multi = false);
    QStringList modalOpenPresentations(const QString&, bool multi = false);
    QStringList modalOpenMedia(const QString& type, const QString& path, bool multi = false);
    QStringList modalOpenForEncrypt(const QString& path, bool multi = false);

    QString selectFolder(const QString& folder);
    void    setTitle(const QString&);
    void    setFormats(std::vector<int>&);
    int     getFormat();

private:
    QString getFilter(const QString&) const;
    int getKey(const QString &value);
//    QString joinFilters() const;
    void checkForMimeTypes(QStringList &files, const QString &type);
    QString joinExtentions(const QString&) const;
    QString m_title;
    QString m_filters;
    QMap<int, QString> m_mapFilters;
    int m_format;
};

#endif // CFILEDIALOG_H
