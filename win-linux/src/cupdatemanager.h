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

#ifndef CUPDATEMANAGER_H
#define CUPDATEMANAGER_H

#include <QObject>
#include <QTimer>

#ifdef __linux__
# define __STDC_WANT_LIB_EXT1__ 1
#endif
#include <ctime>
#include "csocket.h"

#ifdef _WIN32
# define QStrToTStr(a) a.toStdWString()
# define TStrToQStr(a) QString::fromStdWString(a)
#else
# define QStrToTStr(a) a.toStdString()
# define TStrToQStr(a) QString::fromStdString(a)
#endif

#define DLG_RESULT_NONE -2

using std::wstring;

enum UpdateMode {
    DISABLE=0, SILENT=1, ASK=2
};

struct ComplexText {
    ComplexText(const char *_text = nullptr, const QString &_arg1 = "", const QString &_arg2 = "") :
        text(_text), arg1(_arg1), arg2(_arg2) {}
    const char *text = nullptr;
    QString arg1, arg2;
};

struct Command {
    Command(const QString &_icon = "", const ComplexText &_text = ComplexText(), const char *_btn_text = nullptr,
                const QString &_btn_action = "", const QString &_btn_lock = "") :
        icon(_icon), text(_text), btn_text(_btn_text), btn_action(_btn_action), btn_lock(_btn_lock) {}
    bool isEmpty() const {
        return (icon.isEmpty() && text.text == nullptr && btn_text == nullptr &&
                   btn_action.isEmpty() && btn_lock.isEmpty());
    }
    QString icon;
    ComplexText text;
    const char *btn_text = nullptr;
    QString btn_action, btn_lock;
};

class CUpdateManager: public QObject
{
    Q_OBJECT
public:
    explicit CUpdateManager(QObject *parent = nullptr);
    ~CUpdateManager();

    void setNewUpdateSetting(const QString& _rate);
    void cancelLoading();
    void skipVersion();
    int  getUpdateMode();
    void handleAppClose();
    void loadUpdates();
    void installUpdates();
    void refreshStartPage(const Command &cmd = Command());
    void launchIntervalStartTimer();
    void setServiceLang(QString lang = QString());

public slots:
    void checkUpdates(bool manualCheck = false);

private:
    void init();
    void clearTempFiles(const QString &except = QString());
    void updateNeededCheking();
    void unzipIfNeeded();
    void savePackageData(const QString &version = QString(), const QString &fileName = QString(), const QString &fileType = QString());
    QString ignoredVersion();
    bool isSavedPackageValid();
    bool isVersionBHigherThanA(const QString &a, const QString &b);

    struct PackageData;
    struct SavedPackageData;
    PackageData      *m_packageData;
    SavedPackageData *m_savedPackageData;

    bool        m_startUpdateOnClose = false,
                m_restartAfterUpdate = false,
                m_manualCheck = false,
                m_lock = false;

    time_t      m_lastCheck = 0;
    int         m_interval = 0;

    QTimer      *m_pIntervalStartTimer = nullptr,
                *m_pLastCheckMsgTimer = nullptr,
                *m_pIntervalTimer = nullptr;

    Command     m_lastCommand;

    class DialogSchedule;
    DialogSchedule *m_dialogSchedule = nullptr;

    CSocket *m_socket = nullptr;

private slots:
    void onCheckFinished(bool error, bool updateExist, const QString &version, const QString &changelog);
    void onLoadCheckFinished(const QString &json);
    void showUpdateMessage(QWidget *parent, bool forceModal = false, int result = DLG_RESULT_NONE);
    void onLoadUpdateFinished(const QString &filePath);
    void showStartInstallMessage(QWidget *parent, bool forceModal = false, int result = DLG_RESULT_NONE);
    void onProgressSlot(const int percent);
    void onUnzipProgressSlot(const int percent);
    void onError(const QString &error);
    void criticalMsg(QWidget *parent, const QString &msg);
};

#endif // CUPDATEMANAGER_H
