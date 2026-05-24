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

#include "clogger.h"
#include "utils.h"
#include <memory>


CLogger::CLogger(QObject *parent, QString fileName)
    : QObject(parent)
{
    if ( !fileName.isEmpty() ) {
        m_file = new QFile(fileName);
        m_file->open(QIODevice::Append | QIODevice::Text);
    }
}

CLogger::~CLogger()
{
    if ( m_file )
        m_file->close();
}

void CLogger::write(const QString &value)
{
    QString text(value);// + "";
    if ( m_showDate )
        text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ") + text;

    QTextStream out(m_file);
#ifdef QT_VERSION_6
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    if ( m_file ) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        out << text << Qt::endl;
#else
        out << text << endl;
#endif
    }
}

//void CLogger::setShowDateTime(bool value)
//{
//    m_showDate = value;
//}

void CLogger::log(const QString& str)
{
    static const bool _enabled = InputArgs::contains(L"--log");
    if ( _enabled ) {
        QString _file_name = Utils::getAppCommonPath() + "/app.log";
        std::unique_ptr<CLogger> _logger(new CLogger(0, _file_name));

        _logger->write(str);
    }
}
