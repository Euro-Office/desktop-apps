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

#ifndef CEVENTDRIVER_H
#define CEVENTDRIVER_H

#include <QObject>
#include <QWidget>

/*#ifdef Q_OS_WIN
#include <Windows.h>
using NativeHandle = HWND;
#else*/
#include <QWidget>
using NativeHandle = WId;
//#endif

class CInAppEventBase
{
public:
    enum class CEventType
    {
        etModal
        ,etEditorClosed
    };

    CInAppEventBase(CEventType t)
        : m_type(t)
    {}

    virtual ~CInAppEventBase(){}
    virtual CEventType type() const { return m_type; }

private:
    CEventType m_type;
};

class CInAppRunnigEvent : public CInAppEventBase
{
public:
    CInAppRunnigEvent(CEventType t) : CInAppEventBase(t) {}
    virtual ~CInAppRunnigEvent(){}

    bool finished() const { return m_finished; }
    void setFinished(bool f) { m_finished = f; }
private:
    bool m_finished = false;
};

class CInAppEventModal : public CInAppRunnigEvent
{
    NativeHandle m_handle;

public:
    CInAppEventModal(NativeHandle h)
        : CInAppRunnigEvent(CEventType::etModal)
        , m_handle(h)
    {}

    NativeHandle handle() const { return m_handle; }
    void setHandle(NativeHandle h) { m_handle = h; }
};

class CEventDriver : public QObject
{
    Q_OBJECT

public:
    explicit CEventDriver(QObject *parent = nullptr);

//    void signal(edEventType);
    void signal(CInAppEventBase *);
signals:
    void onModalDialog(bool status, NativeHandle handle);
    void onEditorClosed();

public slots:
};

class CRunningEventHelper
{
public:
    CRunningEventHelper(CInAppRunnigEvent *);
    ~CRunningEventHelper();

private:
    CInAppRunnigEvent * m_event;
};

#endif // CEVENTDRIVER_H
