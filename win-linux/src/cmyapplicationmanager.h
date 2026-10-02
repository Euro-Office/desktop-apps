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
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#ifndef QASCAPPLICATIONMANAGER
#define QASCAPPLICATIONMANAGER

#include "../../lib/include/cefapplication.h"
#include "../../lib/include/applicationmanager.h"

#include <QFileDialog>
#include "qmainpanel.h"

class CMyApplicationManager : public CAscApplicationManager
{
private:
    QMainPanel * m_pPanel;

public:
    CMyApplicationManager()
    {
        m_pPanel = NULL;
        qRegisterMetaType<std::wstring>("std::wstring");
    }

public:
    void setMainPanel(QMainPanel * panel)
    {
        m_pPanel = panel;
    }

    virtual void StartSaveDialog(const std::wstring& sName)
    {
        // Route this through QMainPanel so we can use Qt signals/slots.
        // Making QAscApplicationManager inherit QObject would expose the slots through
        // CAscApplicationManager, but the library should stay independent of Qt.

//        ((QMainPanel*)m_pPanel)->sendDialogSave(sName);
        QMetaObject::invokeMethod(m_pPanel, "onDialogSave", Qt::QueuedConnection, Q_ARG(std::wstring, sName));
    }
};

#endif // QASCAPPLICATIONMANAGER

