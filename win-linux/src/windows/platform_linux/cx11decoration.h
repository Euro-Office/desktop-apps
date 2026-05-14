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

#ifndef CX11DECORATION_H
#define CX11DECORATION_H

#include <QWidget>
#include <QMouseEvent>

#define FORCE_LINUX_CUSTOMWINDOW_MARGINS

namespace WindowHelper {
    auto check_button_state(Qt::MouseButton b) -> bool;
}

class CX11Decoration
{
public:
    CX11Decoration(QWidget *);
    virtual ~CX11Decoration();

    void setTitleWidget(QWidget *);
    void dispatchMouseDown(QMouseEvent *);
    void dispatchMouseMove(QMouseEvent *);
    void dispatchMouseUp(QMouseEvent *);
    void setCursorPos(int x, int y);

    void turnOn();
    void turnOff();
    bool isDecorated();
    void setMaximized(bool);
    void setMinimized();
    void raiseWindow();

    static int customWindowBorderWith();

    int m_nDirection;
protected:
    double dpi_ratio = 1;
    void onDpiChanged(double);
    bool isNativeFocus();

private:
    QWidget * m_window;
    QWidget * m_title;
    QTimer * m_motionTimer;
    ulong m_currentCursor;
    bool m_decoration;
    int m_nBorderSize;
    bool m_bIsMaximized;
    bool need_to_check_motion = false;
    QSize m_startSize;

    std::map<int, ulong> m_cursors;

    void createCursors();
    void freeCursors();
    int  hitTest(int x, int y) const;
    void checkCursor(QPoint & p);
    void switchDecoration(bool);
    void sendButtonRelease();
};

#endif // CX11DECORATION_H
