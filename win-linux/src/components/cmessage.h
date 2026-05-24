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

#ifndef CMESSAGE_H
#define CMESSAGE_H

#include <QString>


enum class MsgBtns {
    mbOk = 0,
    mbYesNo,
    mbYesDefNo,
    mbYesNoCancel,
    mbYesDefNoCancel,
    mbOkCancel,
    mbOkDefCancel,
    mbYesDefSkipNo,
    mbBuy,
    mbActivateDefContinue,
    mbContinue,
    mbSkipRemindInstall,
    mbSkipRemindSaveandinstall,
    mbSkipRemindDownload,
    mbInslaterRestart
};
enum class MsgType {
    MSG_INFO = 0,
    MSG_WARN,
    MSG_CONFIRM,
    MSG_ERROR,
    MSG_BRAND
};
enum MsgRes {
    MODAL_RESULT_CANCEL = 0,
    MODAL_RESULT_YES,
    MODAL_RESULT_NO,
    MODAL_RESULT_OK,
    MODAL_RESULT_SKIP,
    MODAL_RESULT_BUY,
    MODAL_RESULT_ACTIVATE,
    MODAL_RESULT_CONTINUE,
    MODAL_RESULT_SKIPVER,
    MODAL_RESULT_REMIND,
    MODAL_RESULT_DOWNLOAD,
    MODAL_RESULT_INSTALL,
    MODAL_RESULT_INSLATER,
    MODAL_RESULT_RESTART
};

struct CMessageOpts {
    bool *checkBoxState = nullptr;
    QString chekBoxText;
    QString contentText;
    QString linkText;
};

class QWidget;
namespace CMessage
{
int showMessage(QWidget *parent,
                const QString &msg,
                MsgType msgType,
                MsgBtns msgBtns = MsgBtns::mbOk,
                const CMessageOpts &opts = {});

void confirm(QWidget *parent, const QString &msg);
void info(QWidget *parent, const QString &msg);
void warning(QWidget *parent, const QString &msg);
void error(QWidget *parent, const QString &msg);
}

#endif // CMESSAGE_H
