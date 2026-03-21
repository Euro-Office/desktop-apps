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

#ifndef CAPTION_H
#define CAPTION_H

#include <QWidget>
#include <QWindow>
#include <Windows.h>
#include <Windowsx.h>
#include <QStyle>
#include <QPushButton>
#include <QCoreApplication>
#include "utils.h"

#include <qtcomp/qnativeevent.h>

#define RESIZE_AREA_PART 0.14


class Caption: public QWidget
{
public:
    Caption(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()):
        QWidget(parent, f)
    {
        hwnd_root = ::GetAncestor((HWND)winId(), GA_ROOT);
        snapLayoutAllowed = isArrangingAllowed();
    }

private:
    HWND hwnd_root;
    bool snapLayoutAllowed = false;

    bool isArrangingAllowed() {
        BOOL arranging = FALSE;
        SystemParametersInfoA(SPI_GETWINARRANGING, 0, &arranging, 0);
        return (arranging == TRUE);
    }

    bool isResizingAvailable() {
        return Utils::getWinVersion() >= Utils::WinVer::Win10 && !IsZoomed(hwnd_root);
    }

    bool isPointInResizeArea(int posY) {
        return posY <= RESIZE_AREA_PART * height();
    }

    QPoint cursorPos() {
        POINT pt;
        ::GetCursorPos(&pt);
        return mapFromGlobal(QPoint(pt.x, pt.y));
    }

    QPushButton* buttonAtPos(const QPoint &pos) {
        QWidget *child = childAt(pos);
        return child ? qobject_cast<QPushButton*>(child) : nullptr;
    }

    QPushButton* buttonMaxUnderMouse() {
        QPushButton *btn = buttonAtPos(cursorPos());
        return (btn && btn->objectName() == "toolButtonMaximize") ? btn : nullptr;
    }

    bool postMsg(DWORD cmd) {
        POINT pt;
        ::GetCursorPos(&pt);
        QPoint pos = mapFromGlobal(QPoint(int(pt.x), int(pt.y)));
        if (!buttonAtPos(pos)) {
            ::ReleaseCapture();
            ::PostMessage(hwnd_root, cmd, isResizingAvailable() && isPointInResizeArea(pos.y()) ? HTTOP : HTCAPTION, POINTTOPOINTS(pt));
#ifndef QT_VERSION_6
            // TODO: crash on mouse down
            QCoreApplication::postEvent(parent(), new QEvent(QEvent::MouseButtonPress));
#endif
            return true;
        }
        return false;
    }

    virtual bool nativeEvent(const QByteArray &eventType, void *message, long_ptr *result) override
    {
    #if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
        MSG* msg = *reinterpret_cast<MSG**>(message);
    #else
        MSG* msg = reinterpret_cast<MSG*>(message);
    #endif

        switch (msg->message)
        {
        case WM_LBUTTONDOWN: {
            if (postMsg(WM_NCLBUTTONDOWN))
                return true;
            break;
        }
        case WM_LBUTTONDBLCLK: {
            if (postMsg(WM_NCLBUTTONDBLCLK))
                return true;
            break;
        }
        case WM_MOUSEMOVE: {
            if (isResizingAvailable()) {
                int y = GET_Y_LPARAM(msg->lParam);
                setCursor(!buttonAtPos(QPoint(GET_X_LPARAM(msg->lParam), y)) && isPointInResizeArea(y) ? Qt::SizeVerCursor : Qt::ArrowCursor);
            }
            break;
        }
        case WM_NCLBUTTONDOWN: {
            if (Utils::getWinVersion() < Utils::WinVer::Win11)
                break;
            if (QPushButton *btn = buttonMaxUnderMouse()) {
                btn->setProperty("hovered", false);
                btn->setProperty("pressed", true);
                btn->style()->polish(btn);
                btn->repaint();
            }
            break;
        }
        case WM_TIMER: {
            QPushButton *btn = buttonMaxUnderMouse();
            if (!btn) {
                KillTimer(msg->hwnd, msg->wParam);
                if (QPushButton *btn = findChild<QPushButton*>("toolButtonMaximize")) {
                    btn->setProperty("hovered", false);
                    btn->setProperty("pressed", false);
                    btn->style()->polish(btn);
                }
            }
            break;
        }
        case WM_NCHITTEST: {
            if (Utils::getWinVersion() < Utils::WinVer::Win11 || !snapLayoutAllowed)
                break;
            *result = 0;
            if (QPushButton *btn = buttonMaxUnderMouse()) {
                if (!btn->property("hovered").toBool()) {
                    btn->setProperty("hovered", true);
                    btn->style()->polish(btn);
                    SetTimer(msg->hwnd, 1, 200, NULL);
                }
                *result = HTMAXBUTTON;
            }
            return (*result != 0);
        }
        case WM_CAPTURECHANGED: {
            if (Utils::getWinVersion() < Utils::WinVer::Win11)
                break;
            if (QPushButton *btn = buttonMaxUnderMouse())
                btn->click();
            break;
        }
        case WM_SETTINGCHANGE: {
            snapLayoutAllowed = isArrangingAllowed();
            break;
        }
        default:
            break;
        }
        return QWidget::nativeEvent(eventType, message, result);
    }
};

#endif
