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
