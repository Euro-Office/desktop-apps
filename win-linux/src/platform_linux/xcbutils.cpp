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

#include "xcbutils.h"
#include <QtConcurrent/QtConcurrent>
#include <QX11Info>
#include <thread>
#include <stdlib.h>
#include <xcb/shape.h>


static xcb_atom_t GetAtom(xcb_connection_t *conn, const char *property_name)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, 1, strlen(property_name), property_name);
    xcb_intern_atom_reply_t *reply  = xcb_intern_atom_reply(conn, cookie, nullptr);
    if (!reply) return XCB_ATOM_NONE;
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

void XcbUtils::moveWindow(xcb_window_t window, int x, int y)
{
    xcb_connection_t *conn = QX11Info::connection();
    if (conn && window != XCB_WINDOW_NONE) {
        uint32_t val[2];
        val[0] = x;
        val[1] = y;
        xcb_configure_window(conn, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, val);
        xcb_flush(conn);
    }
}

bool XcbUtils::isNativeFocus(xcb_window_t window)
{
    xcb_window_t win = 0;
    xcb_connection_t *conn = QX11Info::connection();
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
    return window == win;
}

void XcbUtils::setNativeFocusTo(xcb_window_t window)
{
    xcb_connection_t *conn = QX11Info::connection();
    if (conn && window != XCB_WINDOW_NONE) {
        xcb_void_cookie_t cookie;
        cookie = xcb_set_input_focus(conn, XCB_INPUT_FOCUS_PARENT,
                                     window, XCB_CURRENT_TIME);
        xcb_flush(conn);
    }
}

static void SetSkipTaskbar(xcb_connection_t *conn, xcb_window_t win)
{
    xcb_atom_t state_atom = GetAtom(conn, "_NET_WM_STATE");
    xcb_atom_t skip_atom  = GetAtom(conn, "_NET_WM_STATE_SKIP_TASKBAR");
    if (state_atom != XCB_ATOM_NONE && skip_atom != XCB_ATOM_NONE) {
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win, state_atom, XCB_ATOM_ATOM, 32, 1, &skip_atom);
        xcb_flush(conn);
    }
}

static char* GetWindowClassName(xcb_connection_t *conn, xcb_window_t win)
{
    xcb_atom_t class_atom = GetAtom(conn, "WM_CLASS");
    if (class_atom == XCB_ATOM_NONE) return nullptr;

    xcb_get_property_cookie_t cookie = xcb_get_property(conn, 0, win, class_atom, XCB_ATOM_STRING, 0, 256);
    xcb_get_property_reply_t *reply  = xcb_get_property_reply(conn, cookie, nullptr);
    if (!reply) return nullptr;

    char *name = nullptr;
    int len = xcb_get_property_value_length(reply);
    if (len > 0)
        name = strdup((char*)xcb_get_property_value(reply));

    free(reply);
    return name;
}

static xcb_get_property_reply_t* GetStackingList(xcb_connection_t *conn)
{
    xcb_atom_t stacking_atom = GetAtom(conn, "_NET_CLIENT_LIST_STACKING");
    if (stacking_atom == XCB_ATOM_NONE) return nullptr;

    const xcb_window_t root = (xcb_window_t)QX11Info::appRootWindow();
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, 0, root, stacking_atom, XCB_ATOM_WINDOW, 0, 1024);
    return xcb_get_property_reply(conn, cookie, nullptr);
}

static bool IsVisible(xcb_connection_t *conn, xcb_window_t wnd)
{
    xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(conn, wnd);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(conn, cookie, nullptr);
    if (!reply) return false;
    bool visible = (reply->map_state == XCB_MAP_STATE_VIEWABLE);
    free(reply);
    return visible;
}

void XcbUtils::findWindowAsync(const char *window_name, void *user_data,
                               uint timeout_ms,
                               void(*callback)(xcb_window_t, void*))
{
    QtConcurrent::run([=]() {
        xcb_connection_t *conn = QX11Info::connection();
        if (!conn) return;

        int DELAY_MS = 5;
        int RETRIES = (int)((float)timeout_ms / DELAY_MS);
        xcb_window_t win_found = XCB_WINDOW_NONE;
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));

            xcb_get_property_reply_t *prop_reply = GetStackingList(conn);
            if (!prop_reply) continue;

            int win_count = xcb_get_property_value_length(prop_reply) / sizeof(xcb_window_t);
            xcb_window_t *win_list = (xcb_window_t*)xcb_get_property_value(prop_reply);

            for (int i = 0; i < win_count; i++) {
                char *name = GetWindowClassName(conn, win_list[i]);
                if (name) {
                    if (strstr(name, window_name) != NULL) {
                        if (IsVisible(conn, win_list[i])) {
                            win_found = win_list[i];
                            SetSkipTaskbar(conn, win_found);
                            callback(win_found, user_data);
                        }
                        free(name);
                        break;
                    }
                    free(name);
                }
            }

            free(prop_reply);
        } while (--RETRIES > 0 && win_found == XCB_WINDOW_NONE);
    });
}

std::vector<xcb_window_t> XcbUtils::getWindowStack()
{
    xcb_connection_t *conn = QX11Info::connection();
    if (!conn) return {};

    xcb_get_property_reply_t *prop_reply = GetStackingList(conn);
    if (!prop_reply) return {};

    int win_count = xcb_get_property_value_length(prop_reply) / sizeof(xcb_window_t);
    xcb_window_t *win_list = (xcb_window_t*)xcb_get_property_value(prop_reply);

    std::vector<xcb_window_t> winStack;
    for (int i = 0; i < win_count; i++)
        winStack.push_back(win_list[i]);

    free(prop_reply);
    return winStack;
}

bool XcbUtils::isWindowCoveredAt(xcb_window_t winId, xcb_window_t ignoringWinId, int x, int y)
{
    xcb_connection_t *conn = QX11Info::connection();
    if (!conn) return false;

    xcb_get_property_reply_t *prop_reply = GetStackingList(conn);
    if (!prop_reply) return false;

    int win_count = xcb_get_property_value_length(prop_reply) / sizeof(xcb_window_t);
    xcb_window_t *win_list = static_cast<xcb_window_t *>(xcb_get_property_value(prop_reply));

    bool result = false;

    for (int i = win_count - 1; i >= 0; i--) {
        xcb_window_t wid = win_list[i];
        if (wid == ignoringWinId)
            continue;

        if (!IsVisible(conn, wid))
            continue;

        xcb_get_geometry_cookie_t geom_cookie = xcb_get_geometry(conn, wid);
        xcb_get_geometry_reply_t *geom_reply = xcb_get_geometry_reply(conn, geom_cookie, nullptr);
        if (!geom_reply) continue;

        int w = geom_reply->width;
        int h = geom_reply->height;
        const xcb_window_t wid_root = geom_reply->root;
        free(geom_reply);

        xcb_translate_coordinates_cookie_t trans_cookie = xcb_translate_coordinates(conn, wid, wid_root, 0, 0);
        xcb_translate_coordinates_reply_t *trans_reply = xcb_translate_coordinates_reply(conn, trans_cookie, nullptr);
        if (!trans_reply) continue;

        int wx = trans_reply->dst_x;
        int wy = trans_reply->dst_y;
        free(trans_reply);

        if (x < wx || x >= wx + w || y < wy || y >= wy + h)
            continue;

        result = (wid != winId);
        break;
    }

    free(prop_reply);
    return result;
}

void XcbUtils::setInputEnabled(xcb_window_t window, bool enabled)
{
    xcb_connection_t *conn = QX11Info::connection();
    if (!conn) return;

    if (enabled) {
        xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT, window, 0, 0, XCB_PIXMAP_NONE);
    } else {
        xcb_rectangle_t rc = {0, 0, 0, 0};
        xcb_shape_rectangles(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_YX_BANDED, window, 0, 0, 1, &rc);
    }
    xcb_flush(conn);
}

void XcbUtils::setInputShape(xcb_window_t window, const xcb_rectangle_t &rc)
{
    xcb_connection_t *conn = QX11Info::connection();
    if (!conn) return;

    if (rc.width == 0 && rc.height == 0) {
        xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT, window, 0, 0, XCB_PIXMAP_NONE);
    } else {
        xcb_shape_rectangles(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_YX_BANDED, window, 0, 0, 1, &rc);
    }
    xcb_flush(conn);
}
