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

#include "cfilechecker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QTextDocumentFragment>
#include <QUrl>
#include "utils.h"


#define FILE_UNKNOWN    0
#define FILE_EXISTS     1
#define FILE_ABSENT     2
#define FILE_REMOTE     3

#define ERASE_VALUE(iterator) \
    if ( iterator.second ) { \
        CFileInspector * p = iterator.second;  \
        delete p, p = nullptr;                  \
    }


CFileInspector::CFileInspector(QObject *parent, const QString& name, int uid)
    : QThread(parent)
    , m_file(QTextDocumentFragment::fromHtml(name).toPlainText())
    , m_uid(uid)
{
}

void CFileInspector::run()
{
    int result = FILE_UNKNOWN;
    if (QUrl::fromUserInput(m_file).isLocalFile()) {
        if ( !isInterruptionRequested() ) {
            result = QFileInfo::exists(m_file) ? FILE_EXISTS : FILE_ABSENT;
        }
    } else {
        result = FILE_REMOTE;
    }

    if ( !isInterruptionRequested() )
        emit examined(m_file, m_uid, result);
}

bool CFileInspector::isLocalFile(const QString& path)
{
    if (Utils::isFileLocal(path))
        return true;
    QUrl url = QUrl::fromUserInput(path);
    if (!url.isValid()) {
        QFileInfo info(path);
        if (info.isFile())
            return QUrl::fromUserInput(info.absoluteFilePath()).isLocalFile();
    }
    return url.isLocalFile();
}

/**/

CExistanceController::CExistanceController()
{
}

CExistanceController::~CExistanceController()
{
    for (auto &iter: m_mapStaff) {
        if ( iter.second ) {
            CFileInspector * worker = iter.second;
            if ( worker->isRunning() ) {
                worker->disconnect();

                worker->requestInterruption();
                worker->quit();
            }
        }
    }

    for (auto &iter: m_mapStaff) {
        if ( iter.second ) {
            CFileInspector * worker = iter.second;
            if ( worker->isRunning() ) {
//                worker->wait();
                worker->terminate();
            }

            ERASE_VALUE(iter)
        }
    }
}

CExistanceController * CExistanceController::getInstance()
{
    static CExistanceController _instance;
    return &_instance;
}

void CExistanceController::check(const QString& json)
{
    getInstance()->parseJson(json);
    getInstance()->processMap();
}

void CExistanceController::parseJson(const QString &json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8(), &jerror);

    if(jerror.error == QJsonParseError::NoError) {
        QMutexLocker locker(&m_mutex);
        QJsonObject objRoot = jdoc.object();

        if ( objRoot.size() ) {
            QString _file_name;
            int _uid;

            foreach (QString k, objRoot.keys()) {
                _file_name = objRoot[k].toString();
                _uid = k.toInt();

                if ( m_mapRemote.find(_uid) == m_mapRemote.end() ) {
                    m_mapStaff.insert(
                        std::pair<int, CFileInspector *>(_uid, new CFileInspector(this, _file_name, _uid)));
                }
            }
        }
    }
}

void CExistanceController::processMap()
{
    for (auto &iter: m_mapStaff) {
        CFileInspector * worker = iter.second;
        if ( worker && !worker->isFinished() && !worker->isRunning() ) {
            connect(worker, &CFileInspector::examined, this, &CExistanceController::handleResults);
            connect(worker, &CFileInspector::finished, worker, &CFileInspector::deleteLater);

            worker->start();
        }
    }
}

void CExistanceController::handleResults(const QString& name, int uid, int result)
{
    QMutexLocker locker(&m_mutex);
    auto iter = m_mapStaff.find(uid);

    if ( iter != m_mapStaff.end() ) {
        CFileInspector * worker = iter->second;
        if ( worker ) worker->disconnect(this);

//        ERASE_VALUE((*iter));
        m_mapStaff.erase(iter);
    }

    if ( result == FILE_REMOTE ) {
        m_mapRemote.insert(std::pair<int, QString>(uid, QString(name)));
    } else
    if ( result == FILE_ABSENT ) {
        locker.unlock();
        emit checked(name, uid, false);
    }
}

bool CExistanceController::isFileRemote(const QString& path)
{
    for (auto &iter: getInstance()->m_mapRemote) {
        if ( iter.second == path)
            return true;
    }

    return !QUrl::fromUserInput(path).isLocalFile();
}
