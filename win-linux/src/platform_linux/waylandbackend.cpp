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

#include "waylandbackend.h"
#include <QGuiApplication>
#include <QWindow>


WaylandBackend::WaylandBackend()
{
}

WaylandBackend::~WaylandBackend()
{
}

void WaylandBackend::moveWindow(WId window, int x, int y)
{
    // Wayland does not allow clients to position their own windows.
    Q_UNUSED(window);
    Q_UNUSED(x);
    Q_UNUSED(y);
}

bool WaylandBackend::isNativeFocus(WId window)
{
    QWindow *focusWindow = QGuiApplication::focusWindow();
    if (!focusWindow)
        return false;
    return focusWindow->winId() == window;
}

void WaylandBackend::setNativeFocusTo(WId window)
{
    QWidget *widget = QWidget::find(window);
    if (widget && widget->windowHandle())
        widget->windowHandle()->requestActivate();
}

void WaylandBackend::setInputEnabled(WId window, bool enabled)
{
    // XShape is not available on Wayland; use Qt's setEnabled as a simplified fallback.
    QWidget *widget = QWidget::find(window);
    if (widget)
        widget->setEnabled(enabled);
}

void WaylandBackend::getWindowStack(std::vector<WId> &winStack)
{
    // Window stacking order is not available on Wayland.
    // Consumers should use internal tracking instead.
    Q_UNUSED(winStack);
}

void WaylandBackend::findWindowAsync(const char *name, void *data,
                                     uint timeout_ms,
                                     void(*cb)(WId, void*))
{
    // Enumerating foreign windows is not possible on Wayland.
    Q_UNUSED(name);
    Q_UNUSED(data);
    Q_UNUSED(timeout_ms);
    Q_UNUSED(cb);
}

void WaylandBackend::raiseWindow(QWidget *window)
{
    // Best-effort: raise and request activation via Qt APIs.
    window->raise();
    window->activateWindow();
}

void WaylandBackend::minimizeWindow(QWidget *window)
{
    window->showMinimized();
}

void WaylandBackend::setCursorPos(int x, int y)
{
    // Warping the cursor is not allowed on Wayland.
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void WaylandBackend::sendButtonRelease(QWidget *window)
{
    // Synthetic input events are not possible on Wayland.
    Q_UNUSED(window);
}

void WaylandBackend::startInteractiveMove(QWidget *window, const QPoint &globalPos)
{
    Q_UNUSED(globalPos);
    if (window && window->windowHandle())
        window->windowHandle()->startSystemMove();
}

void WaylandBackend::startInteractiveResize(QWidget *window, Qt::Edges edges, const QPoint &globalPos)
{
    Q_UNUSED(globalPos);
    if (window && window->windowHandle())
        window->windowHandle()->startSystemResize(edges);
}

void WaylandBackend::setCursor(WId window, int cursorShape)
{
    Qt::CursorShape qt_shape = Qt::ArrowCursor;
    switch (cursorShape) {
        case 0: qt_shape = Qt::SizeFDiagCursor; break; // Top-left
        case 1: qt_shape = Qt::SizeVerCursor;  break; // Top
        case 2: qt_shape = Qt::SizeBDiagCursor; break; // Top-right
        case 3: qt_shape = Qt::SizeHorCursor;  break; // Right
        case 4: qt_shape = Qt::SizeFDiagCursor; break; // Bottom-right
        case 5: qt_shape = Qt::SizeVerCursor;  break; // Bottom
        case 6: qt_shape = Qt::SizeBDiagCursor; break; // Bottom-left
        case 7: qt_shape = Qt::SizeHorCursor;  break; // Left
        default: qt_shape = Qt::CursorShape(cursorShape); break; // fallback
    }
    QWidget *widget = QWidget::find(window);
    if (widget)
        widget->setCursor(qt_shape);
}

void WaylandBackend::resetCursor(WId window)
{
    QWidget *widget = QWidget::find(window);
    if (widget)
        widget->unsetCursor();
}

bool WaylandBackend::isCompositingAvailable()
{
    // Wayland is inherently composited.
    return true;
}

bool WaylandBackend::checkButtonState(Qt::MouseButton b)
{
    return QGuiApplication::mouseButtons().testFlag(b);
}

QString WaylandBackend::portalParentHandle(QWidget *parent)
{
    // XDG Desktop Portal works without a parent window handle on Wayland.
    // Full xdg_foreign support can be added later if needed.
    Q_UNUSED(parent);
    return QString();
}
