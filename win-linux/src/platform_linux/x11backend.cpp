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

#include "x11backend.h"
#include <QtConcurrent/QtConcurrent>
#include <QGuiApplication>
#include <thread>
#include <cstdlib>
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib-xcb.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>


namespace {

auto getXDisplay() -> Display * {
    static Display* display = NULL;
    if ( !display )
        display = XOpenDisplay(NULL);

    return display;
}

void SetSkipTaskbar(Display* disp, Window win)
{
    Atom wm_state = XInternAtom(disp, "_NET_WM_STATE", True);
    Atom wm_state_skip_taskbar = XInternAtom(disp, "_NET_WM_STATE_SKIP_TASKBAR", True);
    if (wm_state != None && wm_state_skip_taskbar != None)
        XChangeProperty(disp, win, wm_state, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&wm_state_skip_taskbar, 1);
}

void GetWindowName(Display* disp, Window win, char **name) {
    XClassHint* class_hint = NULL;
    class_hint = XAllocClassHint();
    if (class_hint) {
        Status s = XGetClassHint(disp, win, class_hint);
        if (s == 1)
            *name = strdup(class_hint->res_name);
        XFree(class_hint);
    }
}

void GetWindowList(Display *disp, Window **list, unsigned long *len) {
    int form;
    unsigned long remain;
    unsigned char *win_list;
    Atom type;
    Atom prop = XInternAtom(disp, "_NET_CLIENT_LIST_STACKING", true);
    Window root = XDefaultRootWindow(disp);
    int res = XGetWindowProperty(disp, root, prop, 0, 1024, false, XA_WINDOW,
                                 &type, &form, len, &remain, &win_list);
    if (res == Success)
        *list = (Window*)win_list;
}

bool IsVisible(Display *disp, Window wnd)
{
    xcb_connection_t *conn = XGetXCBConnection(disp);
    if (conn) {
        xcb_get_window_attributes_cookie_t cookie;
        xcb_get_window_attributes_reply_t *reply;
        cookie = xcb_get_window_attributes(conn, wnd);
        reply = xcb_get_window_attributes_reply(conn, cookie, NULL);
        if (reply) {
            uint8_t state = reply->map_state;
            free(reply);
            if (state == XCB_MAP_STATE_VIEWABLE)
                return true;
        }
    }
    return false;
}

int edgesToMoveResizeDirection(Qt::Edges edges)
{
    // _NET_WM_MOVERESIZE direction constants
    const int SIZE_TOPLEFT =     0;
    const int SIZE_TOP =         1;
    const int SIZE_TOPRIGHT =    2;
    const int SIZE_RIGHT =       3;
    const int SIZE_BOTTOMRIGHT = 4;
    const int SIZE_BOTTOM =      5;
    const int SIZE_BOTTOMLEFT =  6;
    const int SIZE_LEFT =        7;

    if (edges == (Qt::TopEdge | Qt::LeftEdge))      return SIZE_TOPLEFT;
    if (edges == Qt::TopEdge)                        return SIZE_TOP;
    if (edges == (Qt::TopEdge | Qt::RightEdge))     return SIZE_TOPRIGHT;
    if (edges == Qt::RightEdge)                      return SIZE_RIGHT;
    if (edges == (Qt::BottomEdge | Qt::RightEdge))  return SIZE_BOTTOMRIGHT;
    if (edges == Qt::BottomEdge)                     return SIZE_BOTTOM;
    if (edges == (Qt::BottomEdge | Qt::LeftEdge))   return SIZE_BOTTOMLEFT;
    if (edges == Qt::LeftEdge)                       return SIZE_LEFT;

    return -1;
}

} // anonymous namespace


X11Backend::X11Backend()
{
}

X11Backend::~X11Backend()
{
}

void X11Backend::moveWindow(WId window, int x, int y)
{
    Display *disp = XOpenDisplay(NULL);
    if (disp) {
        xcb_connection_t *conn = XGetXCBConnection(disp);
        if (conn && (xcb_window_t)window != XCB_WINDOW_NONE) {
            uint32_t val[2];
            val[0] = x;
            val[1] = y;
            xcb_configure_window(conn, (xcb_window_t)window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, val);
            xcb_flush(conn);
        }
        XCloseDisplay(disp);
    }
}

bool X11Backend::isNativeFocus(WId window)
{
    xcb_window_t win = 0;
    Display *disp = XOpenDisplay(NULL);
    if (disp) {
        xcb_connection_t *conn = XGetXCBConnection(disp);
        if (conn) {
            xcb_get_input_focus_cookie_t cookie;
            xcb_get_input_focus_reply_t *reply;
            cookie = xcb_get_input_focus(conn);
            reply = xcb_get_input_focus_reply(conn, cookie, NULL);
            if (reply) {
                win = reply->focus;
                free(reply);
            }
            xcb_flush(conn);
        }
        XCloseDisplay(disp);
    }
    return (xcb_window_t)window == win;
}

void X11Backend::setNativeFocusTo(WId window)
{
    Display *disp = XOpenDisplay(NULL);
    if (disp) {
        xcb_connection_t *conn = XGetXCBConnection(disp);
        if (conn && (xcb_window_t)window != XCB_WINDOW_NONE) {
            xcb_void_cookie_t cookie;
            cookie = xcb_set_input_focus(conn, XCB_INPUT_FOCUS_PARENT,
                                         (xcb_window_t)window, XCB_CURRENT_TIME);
            xcb_flush(conn);
        }
        XCloseDisplay(disp);
    }
}

void X11Backend::setInputEnabled(WId window, bool enabled)
{
    Display* disp = XOpenDisplay(NULL);
    if (disp) {
        Window wnd = (Window)window;
        if (enabled) {
            XShapeCombineMask(disp, wnd, ShapeInput, 0, 0, None, ShapeSet);
        } else {
            XRectangle rc = {0, 0, 0, 0};
            XShapeCombineRectangles(disp, wnd, ShapeInput, 0, 0, &rc, 1, ShapeSet, YXBanded);
        }
        XFlush(disp);
        XCloseDisplay(disp);
    }
}

void X11Backend::getWindowStack(std::vector<WId> &winStack)
{
    Display *disp = XOpenDisplay(NULL);
    if (!disp)
        return;
    Window *win_list = NULL;
    unsigned long win_list_size = 0;
    GetWindowList(disp, &win_list, &win_list_size);
    if (win_list) {
        for (int i = 0; i < (int)win_list_size; i++)
            winStack.push_back((WId)win_list[i]);
        XFree(win_list);
    }
    XCloseDisplay(disp);
}

void X11Backend::findWindowAsync(const char *name, void *data,
                                 uint timeout_ms,
                                 void(*cb)(WId, void*))
{
    QtConcurrent::run([=]() {
        Display *disp = XOpenDisplay(NULL);
        if (!disp)
            return;
        int DELAY_MS = 50;
        int RETRIES = (int)((float)timeout_ms / DELAY_MS);
        Window win_found = None;
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
            Window *win_list = NULL;
            unsigned long win_list_size = 0;
            GetWindowList(disp, &win_list, &win_list_size);
            for (int i = 0; i < (int)win_list_size; i++) {
                char *wname = NULL;
                GetWindowName(disp, win_list[i], &wname);
                if (wname) {
                    if (strstr(wname, name) != NULL) {
                        if (IsVisible(disp, win_list[i])) {
                            win_found = win_list[i];
                            SetSkipTaskbar(disp, win_found);
                            cb((WId)win_found, data);
                        }
                        free(wname);
                        break;
                    }
                    free(wname);
                }
            }
            if (win_list)
                XFree(win_list);
        } while (--RETRIES > 0 && win_found == None);
        XCloseDisplay(disp);
    });
}

void X11Backend::raiseWindow(QWidget *window)
{
    Display *disp = getXDisplay();
    Atom atom_active_wnd = XInternAtom(disp, "_NET_ACTIVE_WINDOW", False);
    if (atom_active_wnd == None)
        return;
    Window wnd = (Window)window->winId();
    Window root = DefaultRootWindow(disp);
    XEvent event;
    memset(&event, 0, sizeof(XEvent));
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = atom_active_wnd;
    event.xclient.window = wnd;
    event.xclient.format = 32;
    XSendEvent(disp, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XMapRaised(disp, wnd);
    XFlush(disp);
}

void X11Backend::minimizeWindow(QWidget *window)
{
    Display * xdisplay_ = getXDisplay();
    Atom wm_change_state = XInternAtom(xdisplay_, "WM_CHANGE_STATE", False);

    XClientMessageEvent ev;
    ev.type = ClientMessage;
    ev.window = window->winId();
    ev.message_type = wm_change_state;
    ev.format = 32;
    ev.data.l[0] = IconicState;

    XSendEvent(xdisplay_, RootWindow(xdisplay_, DefaultScreen(xdisplay_)), False,
                    SubstructureRedirectMask|SubstructureNotifyMask, (XEvent *)&ev);

    // Also send a button release to avoid stuck states
    sendButtonRelease(window);
}

void X11Backend::setCursorPos(int x, int y)
{
    Display *xdisplay_ = getXDisplay();
    Window root_window = DefaultRootWindow(xdisplay_);
    XSelectInput(xdisplay_, root_window, KeyReleaseMask);
    XWarpPointer(xdisplay_, None, root_window, 0, 0, 0, 0, x, y);
    XFlush(xdisplay_);
}

void X11Backend::sendButtonRelease(QWidget *window)
{
    Display * xdisplay_ = getXDisplay();
    Window x_root_window_ = (Window)window->effectiveWinId();

    XEvent event;
    memset(&event, 0, sizeof(XEvent));

    event.type = ButtonRelease;
    event.xbutton.button = Button1;
    event.xbutton.same_screen = True;

    XQueryPointer(xdisplay_, x_root_window_, &event.xbutton.root, &event.xbutton.window,
                        &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    XSendEvent(xdisplay_, PointerWindow, True, ButtonReleaseMask, &event);
    XFlush(xdisplay_);
}

void X11Backend::startInteractiveMove(QWidget *window, const QPoint &globalPos)
{
    const int MOVE = 8;

    Display * xdisplay_ = getXDisplay();
    Window x_root_window_ = DefaultRootWindow(xdisplay_);

    XUngrabPointer(xdisplay_, CurrentTime);

    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.display = xdisplay_;
    event.xclient.window = window->winId();
    event.xclient.message_type = XInternAtom(xdisplay_, "_NET_WM_MOVERESIZE", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = globalPos.x();
    event.xclient.data.l[1] = globalPos.y();
    event.xclient.data.l[2] = MOVE;
    event.xclient.data.l[3] = Button1;
    event.xclient.data.l[4] = 0;

    XSendEvent(xdisplay_, x_root_window_, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(xdisplay_);
}

void X11Backend::startInteractiveResize(QWidget *window, Qt::Edges edges, const QPoint &globalPos)
{
    int direction = edgesToMoveResizeDirection(edges);
    if (direction < 0)
        return;

    Display * xdisplay_ = getXDisplay();
    Window x_root_window_ = DefaultRootWindow(xdisplay_);

    XUngrabPointer(xdisplay_, CurrentTime);

    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.display = xdisplay_;
    event.xclient.window = window->winId();
    event.xclient.message_type = XInternAtom(xdisplay_, "_NET_WM_MOVERESIZE", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = globalPos.x();
    event.xclient.data.l[1] = globalPos.y();
    event.xclient.data.l[2] = direction;
    event.xclient.data.l[3] = Button1;
    event.xclient.data.l[4] = 0;

    XSendEvent(xdisplay_, x_root_window_, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(xdisplay_);
}

void X11Backend::setCursor(WId window, int cursorShape)
{
    unsigned int x11_shape = 0;
    switch (cursorShape) {
        case 0: x11_shape = 134; break; // XC_top_left_corner
        case 1: x11_shape = 138; break; // XC_top_side
        case 2: x11_shape = 136; break; // XC_top_right_corner
        case 3: x11_shape = 96;  break; // XC_right_side
        case 4: x11_shape = 14;  break; // XC_bottom_right_corner
        case 5: x11_shape = 16;  break; // XC_bottom_side
        case 6: x11_shape = 12;  break; // XC_bottom_left_corner
        case 7: x11_shape = 70;  break; // XC_left_side
        default: x11_shape = cursorShape; break; // fallback
    }
    Display * _display = getXDisplay();
    if (_display) {
        Cursor cursor = XCreateFontCursor(_display, x11_shape);
        XDefineCursor(_display, (Window)window, cursor);
        XFlush(_display);
        XFreeCursor(_display, cursor);
    }
}

void X11Backend::resetCursor(WId window)
{
    Display * _display = getXDisplay();
    XUndefineCursor(_display, (Window)window);
    XFlush(_display);
}

bool X11Backend::isCompositingAvailable()
{
    // Qt 6 does not provide QX11Info; assume compositing is available
    return true;
}

bool X11Backend::checkButtonState(Qt::MouseButton b)
{
    Display * xdisplay_ = getXDisplay();
    Window x_root_window_ = DefaultRootWindow(xdisplay_);

    Window root_, child_;
    int root_x, root_y, child_x, child_y;
    uint mask;

    Bool res = XQueryPointer(xdisplay_, x_root_window_, &root_, &child_,
                                &root_x, &root_y, &child_x, &child_y, &mask);

    if ( res ) {
        if ( b == Qt::LeftButton)
            return mask & Button1MotionMask;
    }

    return false;
}

QString X11Backend::portalParentHandle(QWidget *parent)
{
    return "x11:" + QString::number((long)parent->winId(), 16);
}
