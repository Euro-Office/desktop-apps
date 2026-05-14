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

#include "windows/platform_win/cwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QTimer>
#include <qtcomp/qdesktopwidget.h>
#include <QWindow>
#include <QScreen>
#include <QJsonObject>
#include <windowsx.h>
#include <shellapi.h>

//#define UM_SNAPPING 0x02
#define DCX_USESTYLE 0x00010000
#define NC_AREA_WIDTH 3
#define SKIP_EVENTS_QUEUE(callback) QTimer::singleShot(0, this, callback)

using WinVer = Utils::WinVer;
using GetSystemMetricsForDpi_t = int (WINAPI*)(int nIndex, UINT dpi);

static auto pGetSystemMetricsForDpi = []() -> GetSystemMetricsForDpi_t
{
    if (HMODULE hUser32 = GetModuleHandle(L"user32"))
        return (GetSystemMetricsForDpi_t)GetProcAddress(hUser32, "GetSystemMetricsForDpi");
    return nullptr;
}();

static double GetLogicalDpi(QWidget *wgt)
{
#ifdef __OS_WIN_XP
    HDC hdc = GetDC(NULL);
    double dpi = (double)GetDeviceCaps(hdc, LOGPIXELSX)/96;
    ReleaseDC(NULL, hdc);
    return dpi;
#else
    auto scr = wgt->windowHandle()->screen();
    return scr ? scr->logicalDotsPerInch()/96 : 1.0;
#endif
}

static void GetFrameMetricsForDpi(FRAME &frame, double dpi, bool maximized = false)
{
    if (pGetSystemMetricsForDpi) {
        frame.left = 0;
        frame.top = pGetSystemMetricsForDpi(SM_CYCAPTION, dpi * 96);
        if (!maximized) {
            int cyFrame = pGetSystemMetricsForDpi(SM_CYSIZEFRAME, dpi * 96);
            int cyBorder = pGetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi * 96);
            frame.top += (cyFrame + cyBorder);
        }
        return;
    }

    WinVer ver = Utils::getWinVersion();
    int row = ver == WinVer::WinXP ? 0 :
              ver <= WinVer::Win7 ? 1 :
              ver <= WinVer::Win8_1 ? 2 :
              ver <= WinVer::Win10 ? 3 : 4;

    int column = dpi <= 1.0 ? 0 :
                 dpi <= 1.25 ? 1 :
                 dpi <= 1.5 ? 2 :
                 dpi <= 1.75 ? 3 :
                 dpi <= 2.0 ? 4 :
                 dpi <= 2.25 ? 5 :
                 dpi <= 2.5 ? 6 :
                 dpi <= 3.0 ? 7 :
                 dpi <= 3.5 ? 8 :
                 dpi <= 4.0 ? 9 :
                 dpi <= 4.5 ? 10 :
                 dpi <= 5.0 ? 11 : 12;

    const int left[5][13] = { // Left margin for scales 100-500%
        {0, 0, 0,  0,  0,  1,  1,  1,  2,  2,  2,  2,  2}, // WinXp: for NC width 3px
        {7, 8, 10, 11, 12, 13, 15, 17, 20, 22, 25, 27, 32}, // WinVista - Win7
        {7, 8, 10, 11, 12, 13, 15, 17, 20, 22, 25, 27, 32}, // Win8 - Win8.1
        {0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win10
        {0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  // Win11
    };

    const int top[5][13] = { // Top margin for scales 100-500%
        {0,  0,  0,  0,  0,  1,  1,  1,  2,  2,   2,   2,   2}, // WinXp: for NC width 3px
        {7,  8,  10, 11, 12, 13, 15, 17, 20, 22,  25,  27,  32}, // WinVista - Win7
        {7,  8,  10, 11, 12, 13, 15, 17, 20, 22,  25,  27,  32}, // Win8 - Win8.1
        {31, 38, 45, 52, 58, 65, 72, 85, 99, 112, 126, 139, 167}, // Win10
        {30, 37, 43, 50, 56, 63, 69, 82, 95, 108, 121, 134, 161}  // Win11
    };

    const int left_ofs[5][13] = { // Left offset for scales 100-500%
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinXp
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinVista - Win7
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win8 - Win8.1
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win10
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  // Win11
    };

    const int top_ofs[5][13] = { // Top offset for scales 100-500%
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinXp
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinVista - Win7
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win8 - Win8.1
        {8,  9,  11, 12, 13, 14, 16, 18, 21, 24, 27, 30, 36}, // Win10
        {7,  8,  9,  10, 11, 12, 13, 15, 17, 19, 21, 23, 28}  // Win11
    };

    frame.left = left[row][column];
    frame.top = top[row][column];
    if (maximized) {
        frame.left -= left_ofs[row][column];
        frame.top -= top_ofs[row][column];
    }
}

static QColor GetBorderColor(bool isActive, const QColor &bkgColor)
{
    int lum = int(0.299 * bkgColor.red() + 0.587 * bkgColor.green() + 0.114 * bkgColor.blue());
    if (isActive) {
        QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\DWM", QSettings::NativeFormat);
        if (reg.value("ColorPrevalence", 0).toInt() != 0) {
            DWORD dwcolor = 0;
            BOOL opaque = TRUE;
            static HRESULT(WINAPI *GetColorizationColor)(DWORD*, BOOL*) = NULL;
            if (!GetColorizationColor) {
                if (HMODULE module = GetModuleHandleA("dwmapi"))
                    *(FARPROC*)&GetColorizationColor = GetProcAddress(module, "DwmGetColorizationColor");
            }
            if (GetColorizationColor && SUCCEEDED(GetColorizationColor(&dwcolor, &opaque))) {
                float a = (float)((dwcolor >> 24) & 0xff)/255;
                if (a < 0.8f)
                    a = 0.8f;
                int r = (int)(((dwcolor >> 16) & 0xff) * a + 255 * (1 - a));
                int g = (int)(((dwcolor >> 8) & 0xff) * a + 255 * (1 - a));
                int b = (int)((dwcolor & 0xff) * a + 255 * (1 - a));
                return QColor(r, g, b);
            }
        } else {
            QSettings reg_lt("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
            if (reg_lt.value("SystemUsesLightTheme", 1).toInt() != 0) {
                QString userSid = Utils::GetCurrentUserSID();
                if (!userSid.isEmpty()) {
                    QSettings reg_ac("HKEY_USERS\\" + userSid + "\\Control Panel\\Desktop", QSettings::NativeFormat);
                    if (reg_ac.value("AutoColorization", 0).toInt() != 0)
                        return bkgColor.lighter(95);
                }
            }
        }
        int res = -0.002*lum*lum + 0.93*lum + 6;
        return QColor(res, res, res);
    }
    int res = -0.0007*lum*lum + 0.78*lum + 25;
    return QColor(res, res, res);
}

static bool isTaskbarAutoHideOn()
{
    APPBARDATA ABData = { sizeof(APPBARDATA) };
    return (SHAppBarMessage(ABM_GETSTATE, &ABData) & ABS_AUTOHIDE) != 0;
}

static bool hasAppBarOnMonitor(const RECT &rc)
{
    HMONITOR hMon = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
    if (!hMon) return false;

    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMon, &mi);
    return !EqualRect(&mi.rcMonitor, &mi.rcWork);
}

static bool isThemeActive()
{
    static BOOL(WINAPI *IsThemeActive)() = NULL;
    if (!IsThemeActive) {
        if (HMODULE module = GetModuleHandleA("uxtheme"))
            *(FARPROC*)&IsThemeActive = GetProcAddress(module, "IsThemeActive");
    }
    return IsThemeActive ? (bool)IsThemeActive() : true;
}

CWindowPlatform::CWindowPlatform(const QRect &rect) :
    CWindowBase(rect),
    m_dpi(1.0),
    m_hWnd(nullptr),
    m_resAreaWidth(MAIN_WINDOW_BORDER_WIDTH),
    m_borderless(true),
    m_closed(false),
    m_isResizeable(true)
//    m_allowMaximize(true)
{
    m_isThemeActive = isThemeActive();
    m_isTaskbarAutoHideOn = isTaskbarAutoHideOn();
    m_borderless = isCustomWindowStyle();
    if (AscAppManager::isRtlEnabled())
        setLayoutDirection(Qt::RightToLeft);
    if (m_borderless && Utils::getWinVersion() <= WinVer::WinVista)
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setGeometry(m_window_rect);
    m_hWnd = (HWND)winId();
    if (m_borderless && Utils::getWinVersion() < WinVer::Win10) {
        LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW;
        ::SetWindowLong(m_hWnd, GWL_STYLE, style & ~WS_CAPTION);
    }

    setProperty("stabilized", true);
    m_propertyTimer = new QTimer(this);
    m_propertyTimer->setSingleShot(true);
    m_propertyTimer->setInterval(100);
    connect(m_propertyTimer, &QTimer::timeout, this, [=]() {
        setProperty("stabilized", true);
    });

    m_isMaximized = IsZoomed(m_hWnd);
    m_dpi = GetLogicalDpi(this);
    GetFrameMetricsForDpi(m_frame, m_dpi, m_isMaximized);
    SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    connect(this->window()->windowHandle(), &QWindow::screenChanged, this, [=]() {
        SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    });
}

CWindowPlatform::~CWindowPlatform()
{
    m_closed = true;
}

/** Public **/

//void CWindowPlatform::toggleBorderless(bool showmax)
//{
//    if (isVisible()) {
//        m_borderless = !m_borderless;
//        show(showmax);
//    }
//}

//void CWindowPlatform::toggleResizeable()
//{
//    m_isResizeable = !m_isResizeable;
//}

void CWindowPlatform::bringToTop()
{
    if (IsIconic(m_hWnd)) {
        ShowWindow(m_hWnd, SW_RESTORE);
    }
    WindowHelper::bringToTop(m_hWnd);
}

void CWindowPlatform::show(bool maximized)
{
    maximized ? CWindowBase::showMaximized() : CWindowBase::show();
}

void CWindowPlatform::setWindowColors(const QColor& background, const QColor& border, bool isActive)
{
    m_brdColor = border;
    m_bkgColor = background;
    QString css;
    if (Utils::getWinVersion() == Utils::WinVer::WinXP) {
        css = QString("QMainWindow{background-color: %1;}").arg(background.name());
        RedrawWindow((HWND)winId(), NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW); // Apply colors to NC-area
    } else
    if (Utils::getWinVersion() < Utils::WinVer::Win10) {
        css = QString("QMainWindow{border:1px solid %1; background-color: %2;}").arg(border.name(), background.name());
    } else
    if (Utils::getWinVersion() == Utils::WinVer::Win10) {
        int brdWidth = 0;
        HDC hdc = GetDC(NULL);
        brdWidth = GetSystemMetrics(SM_CXBORDER) * GetDeviceCaps(hdc, LOGPIXELSX)/96;
        ReleaseDC(NULL, hdc);
        QColor brdColor = GetBorderColor(isActive, background);
        css = QString("QMainWindow{border-top: %1px solid %2; background-color: %3;}").arg(QString::number(brdWidth), brdColor.name(), background.name());
    } else {
        css = QString("QMainWindow{background-color: %1;}").arg(background.name());
    }
    setStyleSheet(css);
}

void CWindowPlatform::adjustGeometry()
{
    QMargins mrg;
    if (!m_borderless) {
        setContentsMargins(mrg);
        return;
    }
    if (isMaximized()) {
        if (Utils::getWinVersion() < WinVer::Win10) {
            QTimer::singleShot(25, this, [=]() {
                auto rc = QtComp::DesktopWidget::availableGeometry(this);
                int offset = 0;
                if (Utils::getWinVersion() == WinVer::WinXP) {
                    if (isTaskbarAutoHideOn())
                        offset += NC_AREA_WIDTH + 1;
                    if (m_isThemeActive)
                        rc.adjust(-NC_AREA_WIDTH, -NC_AREA_WIDTH, NC_AREA_WIDTH, NC_AREA_WIDTH);
                } else
                if (Utils::getWinVersion() > WinVer::WinXP && isTaskbarAutoHideOn())
                    offset += 2;
                SetWindowPos(m_hWnd, NULL, rc.x(), rc.y(), rc.width(), rc.height() - offset, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
            });
        }
    } else {
        if (Utils::getWinVersion() < WinVer::Win10) {
            int border = qRound(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
            if (Utils::getWinVersion() == WinVer::WinXP)
                border -= NC_AREA_WIDTH;
            mrg = QMargins(border, border, border, border);
        } else
        if (Utils::getWinVersion() == WinVer::Win10) {
            int brdWidth = 0;
            HDC hdc = GetDC(NULL);
            brdWidth = GetSystemMetrics(SM_CXBORDER) * GetDeviceCaps(hdc, LOGPIXELSX)/96;
            ReleaseDC(NULL, hdc);
            mrg = QMargins(0, brdWidth, 0, 0);
        }
        m_resAreaWidth = mrg.top();
    }
    setContentsMargins(mrg);
}

/** Protected **/

bool CWindowPlatform::isSessionInProgress()
{
    return m_isSessionInProgress;
}

void CWindowPlatform::onWindowActivate(bool is_active)
{
    for (auto *btn : m_pTopButtons) {
        if (btn)
            btn->setFaded(!is_active);
    }
}

bool CWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        if (m_pMainPanel) {
            m_pMainPanel->setProperty("rtl", AscAppManager::isRtlEnabled());
            onLayoutDirectionChanged();
        }
    }
    return CWindowBase::event(event);
}

#ifdef __OS_WIN_XP
void CWindowPlatform::resizeEvent(QResizeEvent *ev)
{
    CWindowBase::resizeEvent(ev);
    if (m_borderless && Utils::getWinVersion() <= WinVer::WinVista) {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        if (centralWidget()) {
            QMargins mrg = contentsMargins();
            QSize s(rc.right - rc.left - mrg.left() - mrg.right(), rc.bottom - rc.top - mrg.top() - mrg.bottom());
            centralWidget()->setMaximumSize(s);
            centralWidget()->resize(s);
        }
    }
}
#endif

/** Private **/

void CWindowPlatform::changeEvent(QEvent *event)
{
    CWindowBase::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        applyWindowState();
        adjustGeometry();
    }
}

bool CWindowPlatform::nativeEvent(const QByteArray &eventType, void *message, long_ptr *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

//    static uchar movParam = 0;
    switch (msg->message)
    {
    case WM_ACTIVATE: {
        break;
    }

    case WM_DPICHANGED: {
        setMinimumSize(0,0);
        m_dpi = (double)HIWORD(msg->wParam)/96;
        GetFrameMetricsForDpi(m_frame, m_dpi, m_isMaximized);
        if (AscAppManager::IsUseSystemScaling())
            updateScaling(false);
        SKIP_EVENTS_QUEUE([=]() {
            double dpi = Utils::getScreenDpiRatioByWidget(this);
            setMinimumSize(WINDOW_MIN_WIDTH * dpi, WINDOW_MIN_HEIGHT * dpi);
        });
        break;
    }

//    case WM_SYSKEYDOWN: {
//        if (msg->wParam == VK_SPACE) {
//            RECT winrect;
//            GetWindowRect(msg->hwnd, &winrect);
//            TrackPopupMenu(GetSystemMenu(msg->hwnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, msg->hwnd, NULL);
//        }
//        break;
//    }

    case WM_SYSCOMMAND: {
        if ((msg->wParam & 0xFFF0) == SC_KEYMENU) {
            if (GetKeyState(VK_RETURN) & 0x8000)
                return true;
        }
        break;
    }

    case WM_NCCALCSIZE: {
        if (!m_borderless || !msg->wParam)
            break;
        NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS*)msg->lParam;
        if (!m_isThemeActive) {
            *result = m_isMaximized ? 0 : DefWindowProc(msg->hwnd, WM_NCCALCSIZE, msg->wParam, msg->lParam);
            return true;
        }
        *result = DefWindowProc(msg->hwnd, WM_NCCALCSIZE, msg->wParam, msg->lParam);
        params->rgrc[0].left -= m_frame.left;
        params->rgrc[0].top -= m_frame.top;
        params->rgrc[0].right += m_frame.left;
        params->rgrc[0].bottom += m_frame.left;
        if (m_isMaximized && (Utils::getWinVersion() >= WinVer::Win10) && (isTaskbarAutoHideOn() || !hasAppBarOnMonitor(params->rgrc[0])))
            params->rgrc[0].bottom -= 1 * round(m_dpi);
        return true;
    }

    case WM_NCHITTEST: {
        if (m_borderless && m_isResizeable) {
            *result = 0;
            RECT rect;
            GetWindowRect(msg->hwnd, &rect);
            int x = GET_X_LPARAM(msg->lParam);
            int y = GET_Y_LPARAM(msg->lParam);
            if (x <= rect.left + m_resAreaWidth) {
                if (y <= rect.top + m_resAreaWidth)
                    *result = HTTOPLEFT;
                else
                if (y > rect.top + m_resAreaWidth && y < rect.bottom - m_resAreaWidth)
                    *result = HTLEFT;
                else
                if (y >= rect.bottom - m_resAreaWidth)
                    *result = HTBOTTOMLEFT;
            } else
            if (x > rect.left + m_resAreaWidth && x < rect.right - m_resAreaWidth) {
                if (y <= rect.top + m_resAreaWidth)
                    *result = HTTOP;
                else
                if (y >= rect.bottom - m_resAreaWidth)
                    *result = HTBOTTOM;
            } else
            if (x >= rect.right - m_resAreaWidth) {
                if (y <= rect.top + m_resAreaWidth)
                    *result = HTTOPRIGHT;
                else
                if (y > rect.top + m_resAreaWidth && y < rect.bottom - m_resAreaWidth)
                    *result = HTRIGHT;
                else
                if (y >= rect.bottom - m_resAreaWidth)
                    *result = HTBOTTOMRIGHT;
            }
            return (*result != 0);
        }
        break;
    }

    case WM_SETFOCUS: {
        if (!m_closed && IsWindowEnabled(m_hWnd)) {
            SKIP_EVENTS_QUEUE([=]() {
                focus();
            });
        }
        m_propertyTimer->stop();
        if (property("stabilized").toBool())
            setProperty("stabilized", false);
        m_propertyTimer->start();
        break;
    }

    case WM_SETTINGCHANGE: {
        if (msg->wParam == SPI_SETNONCLIENTMETRICS) {
            GetFrameMetricsForDpi(m_frame, m_dpi, m_isMaximized);
            SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        } else
        if (msg->wParam == SPI_SETWINARRANGING) {
            if (Utils::getWinVersion() > Utils::WinVer::Win10 && m_boxTitleBtns)
                SendMessage((HWND)m_boxTitleBtns->winId(), WM_SETTINGCHANGE, 0, 0);
        } else
        if (msg->wParam == SPI_SETWORKAREA) {
            static RECT oldWorkArea = {0,0,0,0};
            RECT workArea; // Taskbar show/hide detection
            SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
            if (!EqualRect(&oldWorkArea, &workArea)) {
                oldWorkArea = workArea;
                m_isTaskbarAutoHideOn = isTaskbarAutoHideOn();
                if (Utils::getWinVersion() < WinVer::Win10)
                    adjustGeometry();
            }
        } else if (msg->wParam == 0 && msg->lParam) {
            const std::wstring param{(wchar_t*)msg->lParam};
            if (param == L"ImmersiveColorSet") {
                QSettings _reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
                bool _changed_theme_dark = _reg.value("AppsUseLightTheme", 1).toInt() == 0;

                if (_changed_theme_dark != AscAppManager::themes().isSystemSchemeDark()) {
                    NSEditorApi::CAscCefMenuEvent * ns_event = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND);
                    NSEditorApi::CAscExecCommand * pData = new NSEditorApi::CAscExecCommand;
                    pData->put_Command(L"system:changed");

                    QJsonObject _json_obj{{"colorscheme", _changed_theme_dark ? "dark" : "light"}};
                    pData->put_Param(Utils::stringifyJson(_json_obj).toStdWString());

                    ns_event->m_pData = pData;
                    ns_event->m_nSenderId = 0;
                    AscAppManager::getInstance().OnEvent(ns_event);
                }
            }
        }
        break;
    }

    case WM_TIMER:
        AscAppManager::getInstance().CheckKeyboard();
        break;

    case WM_POWERBROADCAST: {
        if (msg->wParam == PBT_APMRESUMEAUTOMATIC) {
            HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONULL);
            if (!monitor) {
                moveToPrimaryScreen();
            } else {
                MONITORINFOEX monInfo;
                ZeroMemory(&monInfo, sizeof(monInfo));
                monInfo.cbSize = sizeof(MONITORINFOEX);
                if (GetMonitorInfo(monitor, &monInfo)) {
                    DISPLAY_DEVICE  dispDevice;
                    ZeroMemory(&dispDevice, sizeof(dispDevice));
                    dispDevice.cb = sizeof(dispDevice);
                    if (EnumDisplayDevices(monInfo.szDevice, 0, &dispDevice, EDD_GET_DEVICE_INTERFACE_NAME)) {
                        HANDLE hDevice;
                        hDevice = CreateFile(dispDevice.DeviceID, GENERIC_READ, FILE_SHARE_READ,
                                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
                        if (hDevice != INVALID_HANDLE_VALUE) {
                            BOOL res;
                            if (GetDevicePowerState(hDevice, &res)) {
                                if (res == FALSE)
                                    moveToPrimaryScreen();
                            } else {
                                moveToPrimaryScreen();
                            }
                            CloseHandle(hDevice);
                        } else {
                            moveToPrimaryScreen();
                        }
                    }
                } else {
                    moveToPrimaryScreen();
                }
            }
        }
        break;
    }

    case WM_EXITSIZEMOVE:
        QApplication::postEvent(this, new QEvent(static_cast<QEvent::Type>(UM_ENDMOVE)));
//        if (m_allowMaximize)
//            QApplication::postEvent(this, new QEvent(QEvent::User));
       break;

//    case WM_MOVE:
//        if (movParam != 0)
//            movParam = 0;
//        break;

//    case WM_MOVING:
//        if (m_allowMaximize && ++movParam == UM_SNAPPING)
//            m_allowMaximize = false;
//        break;

    case WM_SIZING:
        if (m_borderless)
            RedrawWindow(msg->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT);
        break;

    case WM_PAINT:
        return false;

    case WM_NCPAINT: {
        if (Utils::getWinVersion() > WinVer::Win7 || !m_borderless)
            break;
        if (HDC hdc = ::GetDCEx(msg->hwnd, 0, DCX_WINDOW | DCX_USESTYLE)) {
            RECT rcc, rcw;
            ::GetClientRect(msg->hwnd, &rcc);
            ::GetWindowRect(msg->hwnd, &rcw);
            POINT pt;
            pt.x = rcw.left;
            pt.y = rcw.top;
            ::MapWindowPoints(0, msg->hwnd, (LPPOINT)&rcw, (sizeof(RECT)/sizeof(POINT)));
            ::OffsetRect(&rcc, -rcw.left, -rcw.top);
            ::OffsetRect(&rcw, -rcw.left, -rcw.top);
            HRGN rgntemp = NULL;
            if (msg->wParam == NULLREGION || msg->wParam == ERROR) {
                ::ExcludeClipRect(hdc, rcc.left, rcc.top, rcc.right, rcc.bottom);
            } else {
                rgntemp = ::CreateRectRgn(rcc.left + pt.x, rcc.top + pt.y, rcc.right + pt.x, rcc.bottom + pt.y);
                if (::CombineRgn(rgntemp, (HRGN)msg->wParam, rgntemp, RGN_DIFF) == NULLREGION) {
                    // nothing to paint
                }
                ::OffsetRgn(rgntemp, -pt.x, -pt.y);
                ::ExtSelectClipRgn(hdc, rgntemp, RGN_AND);
            }
            HBRUSH hbrushBkg = ::CreateSolidBrush(RGB(m_bkgColor.red(), m_bkgColor.green(), m_bkgColor.blue()));
            ::FillRect(hdc, &rcw, hbrushBkg);
            ::DeleteObject(hbrushBkg);

//            HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
//            GetWindowRgn(msg->hwnd, hrgn);
            HBRUSH hbrushBrd = ::CreateSolidBrush(RGB(m_brdColor.red(), m_brdColor.green(), m_brdColor.blue()));
            ::FrameRect(hdc, &rcw, hbrushBrd); // Drawing NC border when using ~WS_CAPTION
//            ::FrameRgn(hdc, hrgn, hbrushBrd, 1, 1); // Drawing NC border when using WS_CAPTION
            ::DeleteObject(hbrushBrd);
//            ::DeleteObject(hrgn);

            ::ReleaseDC(msg->hwnd, hdc);
            if (rgntemp != 0)
                ::DeleteObject(rgntemp);
            return true;
        }
        break;
    }

    case WM_ERASEBKGND:
        return true;

    case WM_NCACTIVATE: {
        if (m_borderless) {
            onWindowActivate(LOWORD(msg->wParam));
            if (Utils::getWinVersion() > WinVer::WinXP && Utils::getWinVersion() < WinVer::Win10) {
                // Prevent drawing of inactive system frame (needs ~WS_CAPTION or temporary ~WS_VISIBLE to work)
                *result = DefWindowProc(msg->hwnd, WM_NCACTIVATE, msg->wParam, -1);
                return true;
            } else
            if (Utils::getWinVersion() == WinVer::Win10) {
                setWindowColors(m_bkgColor, m_brdColor, LOWORD(msg->wParam));
                repaint();
            }
        }
        break;
    }

    case WM_QUERYENDSESSION:
        Utils::setSessionInProgress(false);
        m_isSessionInProgress = false;
        break;

    case WM_ENDSESSION:
        if (!msg->wParam) {
            Utils::setSessionInProgress(true);
            m_isSessionInProgress = true;
        }
        break;

    case WM_GETMINMAXINFO: {
        bool isMaximized = (bool)IsZoomed(msg->hwnd);
        if (m_isMaximized != isMaximized) {
            m_isMaximized = isMaximized;
            GetFrameMetricsForDpi(m_frame, m_dpi, isMaximized);
        }
        break;
    }

    case WM_THEMECHANGED: {
        bool _isThemeActive = isThemeActive();
        if (m_isThemeActive != _isThemeActive)
            m_isThemeActive = _isThemeActive;
        break;
    }

    default:
        break;
    }
    return CWindowBase::nativeEvent(eventType, message, result);
}
