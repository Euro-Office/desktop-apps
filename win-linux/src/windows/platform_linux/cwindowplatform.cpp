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

#include "windows/platform_linux/cwindowplatform.h"
#include "windows/platform_linux/gtkmainwindow.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "platform_linux/xcbutils.h"
#include "utils.h"
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QX11Info>
#include <xcb/xcb.h>

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "platform_linux/cdialogopenssl.h"
#endif
#define WINDOW_CORNER_RADIUS 6


CWindowPlatform::CWindowPlatform(const QRect &rect) :
    CWindowBase(rect),
    CX11Decoration(this)
{
#ifndef DONT_USE_GTK_MAINWINDOW
    m_gtk_wnd = new GtkMainWindow(this, isCustomWindowStyle(), std::bind(&CWindowPlatform::event, this, std::placeholders::_1),
                                      std::bind(&CWindowPlatform::closeEvent, this, std::placeholders::_1));
    setWindowIcon(Utils::appIcon());
    setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);
    setGeometry(m_window_rect);
#endif
    if (AscAppManager::isRtlEnabled())
        setLayoutDirection(Qt::RightToLeft);
    if (isCustomWindowStyle()) {
#ifdef DONT_USE_GTK_MAINWINDOW
        if (QX11Info::isCompositingManagerRunning())
            setAttribute(Qt::WA_TranslucentBackground);
#endif
        CX11Decoration::turnOff();
    }
    setIsCustomWindowStyle(!CX11Decoration::isDecorated());
    adjustGeometry();
    setFocusPolicy(Qt::StrongFocus);
    setProperty("stabilized", true);
    m_propertyTimer = new QTimer(this);
    m_propertyTimer->setSingleShot(true);
    m_propertyTimer->setInterval(100);
    connect(m_propertyTimer, &QTimer::timeout, this, [=]() {
        setProperty("stabilized", true);
    });
}

CWindowPlatform::~CWindowPlatform()
{
#ifndef DONT_USE_GTK_MAINWINDOW
    if (m_gtk_wnd)
        delete m_gtk_wnd, m_gtk_wnd = nullptr;
#endif
}

/** Public **/

void CWindowPlatform::bringToTop()
{
    if (isMinimized()) {
        windowState() == (Qt::WindowMinimized | Qt::WindowMaximized) ?
                    showMaximized() : showNormal();
    }
    CX11Decoration::raiseWindow();
}

void CWindowPlatform::show(bool maximized)
{
#ifdef DONT_USE_GTK_MAINWINDOW
    QMainWindow::show();
    if (maximized) {
        QMainWindow::setWindowState(Qt::WindowMaximized);
    }
#else
    m_gtk_wnd->show();
    if (maximized) {
        m_gtk_wnd->setWindowState(Qt::WindowMaximized);
    }
#endif
}

void CWindowPlatform::setWindowColors(const QColor& background, const QColor& border, bool isActive)
{
    Q_UNUSED(border)
    if (!CX11Decoration::isDecorated()) {
        m_brdColor = border;
#ifdef DONT_USE_GTK_MAINWINDOW
        setStyleSheet(QString("QMainWindow{border:1px solid %1; background-color: %2;}").arg(border.name(), background.name()));
#else
        if (!QX11Info::isCompositingManagerRunning())
            setStyleSheet(QString("QMainWindow{border:1px solid %1; background-color: %2;}").arg(border.name(), background.name()));
        else {
            m_gtk_wnd->setBackgroundColor(background.name());
        }
#endif
    }
}

void CWindowPlatform::adjustGeometry()
{
#ifndef DONT_USE_GTK_MAINWINDOW
    if (!QX11Info::isCompositingManagerRunning()) {
#endif
        int border = (CX11Decoration::isDecorated() || isMaximized()) ? 0 : qRound(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
        setContentsMargins(border, border, border, border);
#ifndef DONT_USE_GTK_MAINWINDOW
    }
#endif
}

#ifndef DONT_USE_GTK_MAINWINDOW
void CWindowPlatform::move(const QPoint &pos)
{
    m_gtk_wnd->move(pos);
}

void CWindowPlatform::setGeometry(const QRect &rc)
{
    m_gtk_wnd->setGeometry(rc);
}

void CWindowPlatform::setWindowIcon(const QIcon &icon)
{
    m_gtk_wnd->setWindowIcon(icon);
}

void CWindowPlatform::setWindowTitle(const QString &title)
{
    m_gtk_wnd->setWindowTitle(title);
    if (m_labelTitle)
        m_labelTitle->setText(title);
}

void CWindowPlatform::setFocus()
{
    m_gtk_wnd->setFocus();
}

void CWindowPlatform::setWindowState(Qt::WindowStates ws)
{
    m_gtk_wnd->setWindowState(ws);
}

void CWindowPlatform::show()
{
    m_gtk_wnd->show();
}

void CWindowPlatform::showMinimized()
{
    m_gtk_wnd->showMinimized();
}

void CWindowPlatform::showMaximized()
{
    m_gtk_wnd->showMaximized();
}

void CWindowPlatform::showNormal()
{
    m_gtk_wnd->showNormal();
}

void CWindowPlatform::activateWindow()
{
    m_gtk_wnd->activateWindow();
}

void CWindowPlatform::setMinimumSize(int w, int h)
{
    m_gtk_wnd->setMinimumSize(w, h);
}

void CWindowPlatform::hide() const
{
    m_gtk_wnd->hide();
}

bool CWindowPlatform::isMaximized()
{
    return m_gtk_wnd->isMaximized();
}

bool CWindowPlatform::isMinimized()
{
    return m_gtk_wnd->isMinimized();
}

bool CWindowPlatform::isActiveWindow()
{
    return m_gtk_wnd->isActiveWindow();
}

bool CWindowPlatform::isVisible() const
{
    return m_gtk_wnd->isVisible();
}

bool CWindowPlatform::isHidden() const
{
    return !m_gtk_wnd->isVisible();
}

QString CWindowPlatform::windowTitle() const
{
    return m_gtk_wnd->windowTitle();
}

QPoint CWindowPlatform::mapToGlobal(const QPoint &pt) const
{
    return m_gtk_wnd->mapToGlobal(pt);
}

QPoint CWindowPlatform::mapFromGlobal(const QPoint &pt) const
{
    return m_gtk_wnd->mapFromGlobal(pt);
}

QSize CWindowPlatform::size() const
{
    return m_gtk_wnd->size();
}

QRect CWindowPlatform::geometry() const
{
    return m_gtk_wnd->geometry();
}

QRect CWindowPlatform::normalGeometry() const
{
    return m_gtk_wnd->normalGeometry();
}

Qt::WindowStates CWindowPlatform::windowState() const
{
    return m_gtk_wnd->windowState();
}
#endif

/** Protected **/
void CWindowPlatform::onMaximizeEvent()
{
    isMaximized() ? showNormal() : showMaximized();
}

void CWindowPlatform::onWindowActivate(bool is_active)
{
    for (auto *btn : m_pTopButtons) {
        if (btn)
            btn->setFaded(!is_active);
    }
}

void CWindowPlatform::onMinimizeEvent()
{
#ifdef DONT_USE_GTK_MAINWINDOW
    CX11Decoration::setMinimized();
#else
    showMinimized();
#endif
}

bool CWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange) {
        CX11Decoration::setMaximized(isMaximized());
        applyWindowState();
        adjustGeometry();
    } else
    if (event->type() == QEvent::HoverLeave) {
        if (m_boxTitleBtns)
            m_boxTitleBtns->setCursor(QCursor(Qt::ArrowCursor));
    } else
    if (event->type() == QEvent::LayoutDirectionChange) {
        if (m_pMainPanel) {
            m_pMainPanel->setProperty("rtl", AscAppManager::isRtlEnabled());
            onLayoutDirectionChanged();
        }
    } else
    if (event->type() == QEvent::MouseMove) {
        // if (!property("blocked").toBool()) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            CX11Decoration::dispatchMouseMove(me);
        // }
    } else
    if (event->type() == QEvent::WindowActivate) {
        onWindowActivate(true);
    }
    else
    if (event->type() == QEvent::WindowDeactivate) {
        onWindowActivate(false);
    } else
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        CX11Decoration::dispatchMouseDown(me);
    } else
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        CX11Decoration::dispatchMouseUp(me);
    } else
    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (m_boxTitleBtns && m_boxTitleBtns->geometry().contains(me->pos()))
            onMaximizeEvent();
    }
// #ifndef DONT_USE_GTK_MAINWINDOW
//     else
//     if (event->type() == Event_GtkFocusIn) {
//         if (isNativeFocus()) {
//             focus();
//             m_propertyTimer->stop();
//             if (property("stabilized").toBool())
//                 setProperty("stabilized", false);
//             m_propertyTimer->start();
//         }
//     }
// #endif
    return CWindowBase::event(event);
}

void CWindowPlatform::setScreenScalingFactor(double factor, bool resize)
{
    CX11Decoration::onDpiChanged(factor);
    CWindowBase::setScreenScalingFactor(factor, resize);
}

bool CWindowPlatform::nativeEvent(const QByteArray &ev_type, void *msg, long *res)
{
    if (ev_type == "xcb_generic_event_t") {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t*>(msg);
        switch (ev->response_type & ~0x80) {
        case XCB_FOCUS_IN:
#ifdef DONT_USE_GTK_MAINWINDOW
            if (isNativeFocus()) {
#else
            if (m_gtk_wnd->isFocused()) {
#endif
                focus();
                m_propertyTimer->stop();
                if (property("stabilized").toBool())
                    setProperty("stabilized", false);
                m_propertyTimer->start();
            }
            break;
        case XCB_FOCUS_OUT: {
#ifndef DONT_USE_GTK_MAINWINDOW
            /* Workaround when focus is lost on click in Wayland */
            xcb_focus_in_event_t *fe = reinterpret_cast<xcb_focus_in_event_t*>(ev);
            if (fe->mode != XCB_NOTIFY_MODE_NORMAL)
                return true;
#endif
            break;
        }
        default:
            break;
        }
    }
    return CWindowBase::nativeEvent(ev_type, msg, res);
}

#ifdef DONT_USE_GTK_MAINWINDOW
void CWindowPlatform::paintEvent(QPaintEvent *event)
{
    if (!QX11Info::isCompositingManagerRunning()) {
        CWindowBase::paintEvent(event);
        return;
    }

    QPainter pnt(this);
    pnt.setRenderHint(QPainter::Antialiasing);
    int d = 2 * WINDOW_CORNER_RADIUS * m_dpiRatio;
    QPainterPath path;
    path.moveTo(width(), d/2);
    path.arcTo(width() - d, 0, d, d, 0, 90);
    path.lineTo(d/2, 0);
    path.arcTo(0, 0, d, d, 90, 90);
    path.lineTo(0, height());
    path.lineTo(width(), height());
    path.lineTo(width(), d/2);
    path.closeSubpath();
    pnt.fillPath(path, palette().window().color());
    pnt.strokePath(path, QPen(m_brdColor, 1));
    pnt.end();
}
#else
void CWindowPlatform::applyWindowState()
{
    if (isCustomWindowStyle() && m_pTopButtons[BtnType::Btn_Maximize]) {
        m_pTopButtons[BtnType::Btn_Maximize]->setProperty("class", isMaximized() ? "min" : "normal") ;
        m_pTopButtons[BtnType::Btn_Maximize]->style()->polish(m_pTopButtons[BtnType::Btn_Maximize]);
    }
}

void CWindowPlatform::saveWindowState(const QString &baseKey)
{
    if (!windowState().testFlag(Qt::WindowFullScreen)) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue(baseKey + "position", normalGeometry());
        if (windowState().testFlag(Qt::WindowMaximized)) {
            reg_user.setValue(baseKey + "maximized", true);
        } else {
            reg_user.remove(baseKey + "maximized");
        }
    }
}
#endif

/** Private **/
