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

#include "cx11decoration.h"
#include "windows/cwindowbase.h"
#include "utils.h"
#include "defines.h"
#include <QX11Info>
#include <QTimer>
#include <QApplication>
#include "X11/Xlib.h"
#include "X11/cursorfont.h"
#include <X11/Xutil.h>
#include "platform_linux/xcbutils.h"

#ifdef DONT_USE_GTK_MAINWINDOW
# define CUSTOM_BORDER_WIDTH MAIN_WINDOW_BORDER_WIDTH
#else
# define CUSTOM_BORDER_WIDTH (QX11Info::isCompositingManagerRunning() ? 0 : MAIN_WINDOW_BORDER_WIDTH)
#endif
#define MOTION_TIMER_MS 250

const int k_NET_WM_MOVERESIZE_SIZE_TOPLEFT =     0;
const int k_NET_WM_MOVERESIZE_SIZE_TOP =         1;
const int k_NET_WM_MOVERESIZE_SIZE_TOPRIGHT =    2;
const int k_NET_WM_MOVERESIZE_SIZE_RIGHT =       3;
const int k_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT = 4;
const int k_NET_WM_MOVERESIZE_SIZE_BOTTOM =      5;
const int k_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT =  6;
const int k_NET_WM_MOVERESIZE_SIZE_LEFT =        7;
const int k_NET_WM_MOVERESIZE_MOVE =             8;

#define MWM_HINTS_DECORATIONS   2
typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
} MotifWmHints;

namespace {
    enum X11_WindowManagerName {
        WM_OTHER,    // We were able to obtain the WM's name, but there is no corresponding entry in this enum.
        WM_UNNAMED,  // Either there is no WM or there is no way to obtain the WM name.
        WM_AWESOME,
        WM_BLACKBOX,
        WM_COMPIZ,
        WM_ENLIGHTENMENT,
        WM_FLUXBOX,
        WM_I3,
        WM_ICE_WM,
        WM_ION3,
        WM_KWIN,
        WM_MATCHBOX,
        WM_METACITY,
        WM_MUFFIN,
        WM_MUTTER,
        WM_NOTION,
        WM_OPENBOX,
        WM_QTILE,
        WM_RATPOISON,
        WM_STUMPWM,
        WM_WMII,
        WM_XFWM4,
        WM_XMONAD
    };

    constexpr const char* atomsToCache[] = {
        "_MOTIF_WM_HINTS",
        "_NET_WM_MOVERESIZE",
        "_NET_SUPPORTING_WM_CHECK",
        "_NET_WM_NAME",
        "WM_CHANGE_STATE"
    };

    template <typename T, size_t N>
    constexpr size_t size(const T (&array)[N]) noexcept {
      return N;
    }

    constexpr int cacheCount = size(atomsToCache);

    class X11AtomCache {
    public:
        static X11AtomCache& getInstance();

    private:
        X11AtomCache();
        ~X11AtomCache() {}
        X11AtomCache(const X11AtomCache&) = delete;
        X11AtomCache& operator=(const X11AtomCache&) = delete;

        Atom getAtom(const char*) const;

        Display* xdisplay_;
        mutable std::map<std::string, Atom> cached_atoms_;

        friend Atom GetAtom(const char* name);
    };

    Atom GetAtom(const char* name) {
      return X11AtomCache::getInstance().getAtom(name);
    }

    X11AtomCache& X11AtomCache::getInstance() {
        static X11AtomCache _instance;
        return _instance;
    }

    auto getXDisplay() -> Display * {
        static Display* display = NULL;
        if ( !display )
              display = XOpenDisplay(NULL);
//              display = QX11Info::display();

        return display;
    }

    X11AtomCache::X11AtomCache() : xdisplay_(getXDisplay()) {
      std::vector<Atom> cached_atoms(cacheCount);
      XInternAtoms(xdisplay_, const_cast<char**>(atomsToCache), cacheCount, False, cached_atoms.data());

      for (int i = 0; i < cacheCount; ++i)
        cached_atoms_[atomsToCache[i]] = cached_atoms[i];
    }

    Atom X11AtomCache::getAtom(const char* name) const {
      const auto it = cached_atoms_.find(name);
      if (it != cached_atoms_.end())
        return it->second;

      Atom atom = XInternAtom(xdisplay_, name, False);
      if (atom == None) {
//        static int error_count = 0;
//        ++error_count;
      }
      cached_atoms_.emplace(name, atom);
      return atom;
    }

    auto getX11RootWindow() -> XID {
        return DefaultRootWindow(getXDisplay());
    }

    // Note: The caller should free the resulting value data.
    bool get_property(XID window, const std::string& property_name, long max_length,
                         Atom* type, int* format, unsigned long* num_items, unsigned char** property)
    {
        Atom property_atom = GetAtom(property_name.c_str());
        unsigned long remaining_bytes = 0;
        return XGetWindowProperty(getXDisplay(), window, property_atom,
                                    0,           // offset into property data to read
                                    max_length,  // max length to get
                                    False,       // deleted
                                    AnyPropertyType, type, format, num_items,
                                    &remaining_bytes, property);
    }

    bool get_int_property(XID window, const std::string& property_name, int* value) {
        Atom type = None;
        int format = 0;  // size in bits of each item in 'property'
        unsigned long num_items = 0;
        unsigned char* property = nullptr;

        int result = get_property(window, property_name, 1, &type, &format, &num_items, &property);
        if (result == Success) {
            if (format == 32 && num_items == 1) {
                *value = static_cast<int>(*(reinterpret_cast<long*>(property)));
                return true;
            }
        }
        return false;
    }

    bool get_string_property(XID window, const std::string& property_name, std::string* value) {
        Atom type = None;
        int format = 0;  // size in bits of each item in 'property'
        unsigned long num_items = 0;
        unsigned char* property = nullptr;

        int result = get_property(window, property_name, 1024, &type, &format, &num_items, &property);
        if (result == Success) {
            if (format == 8) {
                value->assign(reinterpret_cast<char*>(property), num_items);
                return true;
            }
        }
        return false;
    }

    auto supports_ewmh() -> bool {
        static bool supports_ewmh = false;
        static bool supports_ewmh_cached = false;
        if (!supports_ewmh_cached) {
            supports_ewmh_cached = true;

            int wm_window = 0u;
            if (!get_int_property(getX11RootWindow(), "_NET_SUPPORTING_WM_CHECK", &wm_window)) {
                supports_ewmh = false;
                return false;
            }

            int wm_window_property = 0;
            bool result = get_int_property(wm_window, "_NET_SUPPORTING_WM_CHECK", &wm_window_property);
            supports_ewmh = result && wm_window_property == wm_window;
        }

        return supports_ewmh;
    }

    auto get_window_manager_name(std::string* wm_name) -> bool {
        if ( supports_ewmh() ) {
            int wm_window = 0;
            if (get_int_property(getX11RootWindow(), "_NET_SUPPORTING_WM_CHECK", &wm_window)) {
                return get_string_property(static_cast<XID>(wm_window), "_NET_WM_NAME", wm_name);
            }
        }

        return false;
    }

    auto guess_window_manager() -> X11_WindowManagerName {
        std::string name;
        if (!get_window_manager_name(&name)) return WM_UNNAMED;
        if (name == "awesome")            return WM_AWESOME;
        if (name == "Blackbox")           return WM_BLACKBOX;
        if (name == "Compiz" || name == "compiz") return WM_COMPIZ;
        if (name == "e16" || name == "Enlightenment") return WM_ENLIGHTENMENT;
        if (name == "Fluxbox")            return WM_FLUXBOX;
        if (name == "i3")                 return WM_I3;
//        if (base::StartsWith(name, "IceWM", base::CompareCase::SENSITIVE)) return WM_ICE_WM;
        if (name == "ion3")               return WM_ION3;
        if (name == "KWin")               return WM_KWIN;
        if (name == "matchbox")           return WM_MATCHBOX;
        if (name == "Metacity")           return WM_METACITY;
        if (name == "Mutter (Muffin)")    return WM_MUFFIN;
        if (name == "GNOME Shell")        return WM_MUTTER;  // GNOME Shell uses Mutter
        if (name == "Mutter")             return WM_MUTTER;
        if (name == "notion")             return WM_NOTION;
        if (name == "Openbox")            return WM_OPENBOX;
        if (name == "qtile")              return WM_QTILE;
        if (name == "ratpoison")          return WM_RATPOISON;
        if (name == "stumpwm")            return WM_STUMPWM;
        if (name == "wmii")               return WM_WMII;
        if (name == "Xfwm4")              return WM_XFWM4;
        if (name == "xmonad")             return WM_XMONAD;
        return X11_WindowManagerName::WM_OTHER;
    }

}

namespace WindowHelper {
    auto check_button_state(Qt::MouseButton b) -> bool {
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
}

CX11Decoration::CX11Decoration(QWidget * w)
    : m_window(w)
    , m_title(NULL)
    , m_motionTimer(nullptr)
    , m_currentCursor(0)
    , m_decoration(true)
    , m_nBorderSize(CUSTOM_BORDER_WIDTH)
    , m_bIsMaximized(false)
    , m_startSize(QSize())
{
    createCursors();
    m_nDirection = -1;

    need_to_check_motion = guess_window_manager() == WM_KWIN;
    dpi_ratio = Utils::getScreenDpiRatioByWidget(w);
    m_nBorderSize = CUSTOM_BORDER_WIDTH * dpi_ratio;
}

CX11Decoration::~CX11Decoration()
{
    freeCursors();
    if ( m_motionTimer ) {
        m_motionTimer->stop();
        m_motionTimer->deleteLater();
        m_motionTimer = nullptr;
    }
}

void CX11Decoration::setTitleWidget(QWidget * w)
{
    m_title = w;
    m_title->setMouseTracking(true);
}

void CX11Decoration::createCursors()
{
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_TOPLEFT]     = XCreateFontCursor(QX11Info::display(), XC_top_left_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_TOP]         = XCreateFontCursor(QX11Info::display(), XC_top_side);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_TOPRIGHT]    = XCreateFontCursor(QX11Info::display(), XC_top_right_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_RIGHT]       = XCreateFontCursor(QX11Info::display(), XC_right_side);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT] = XCreateFontCursor(QX11Info::display(), XC_bottom_right_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_BOTTOM]      = XCreateFontCursor(QX11Info::display(), XC_bottom_side);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT]  = XCreateFontCursor(QX11Info::display(), XC_bottom_left_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_LEFT]        = XCreateFontCursor(QX11Info::display(), XC_left_side);
}

void CX11Decoration::freeCursors()
{
    Display * _display = QX11Info::display();
    std::for_each(m_cursors.begin(), m_cursors.end(),
        [_display](std::pair<int, Cursor> i) {
            if (_display)
                XFreeCursor(_display, i.second);
        }
    );
}

int CX11Decoration::hitTest(int x, int y) const
{
    if (m_bIsMaximized)
        return -1;

    QRect rect = m_window->rect();
    int bw = m_nBorderSize, bbw = m_nBorderSize;
    int w = rect.width(), h = rect.height();

    QRect rc_top_left       = rect.adjusted(0, 0, -(w-bbw), -(h-bbw));
    QRect rc_top            = rect.adjusted(bbw, 0, -bbw, -(h-bw));
    QRect rc_top_right      = rect.adjusted(w-bbw, 0, 0, -(h-bbw));
    QRect rc_right          = rect.adjusted(w-bw, bbw, 0, -bbw);
    QRect rc_bottom_right   = rect.adjusted(w-bbw, h-bbw, 0, 0);
    QRect rc_bottom         = rect.adjusted(bbw, h-bw, -bbw, 0);
    QRect rc_bottom_left    = rect.adjusted(0, h-bbw, -(w-bbw), 0);
    QRect rc_left           = rect.adjusted(0, bbw, -(w-bw), -bbw);

    int _out_ret = -1;
    if (rc_top.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_TOP;
    } else
    if (rc_right.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_RIGHT;
    } else
    if (rc_bottom.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_BOTTOM;
    } else
    if (rc_left.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_LEFT;
    } else
    if (rc_top_left.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_TOPLEFT;
    } else
    if (rc_top_right.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
    } else
    if (rc_bottom_left.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
    } else
    if (rc_bottom_right.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
    }

    return _out_ret;
}

void CX11Decoration::checkCursor(QPoint & p)
{
    int _hit_test = hitTest(p.x(), p.y());

    Cursor _cursor = 0;
    if (!(_hit_test < 0)) {
        _cursor = m_cursors[_hit_test];
    }

    Display * _display = QX11Info::display();
    if (_cursor) {
        if (m_currentCursor == 0 || m_currentCursor != _cursor) {
            m_currentCursor = _cursor;
            XDefineCursor(_display, m_window->winId(), _cursor);

            XFlush(_display);
        }
    } else
    if (m_currentCursor) {
        m_currentCursor = 0;
        XUndefineCursor(_display, m_window->winId());

        XFlush(_display);
    }
}

void CX11Decoration::dispatchMouseDown(QMouseEvent *e)
{
    if (m_decoration) return;

    if (e->buttons() == Qt::LeftButton) {
        QRect oTitleRect = m_title->geometry();

        if (!m_bIsMaximized)
            oTitleRect.adjust(m_nBorderSize + 1, m_nBorderSize + 1, m_nBorderSize + 1, m_nBorderSize + 1);

        m_nDirection = oTitleRect.contains(e->pos()) ?
                        k_NET_WM_MOVERESIZE_MOVE : hitTest(e->pos().x(), e->pos().y());
    }
}

void CX11Decoration::dispatchMouseMove(QMouseEvent *e)
{
    if (m_decoration) return;

    if ( !m_motionTimer ) {
        m_motionTimer = new QTimer;

        QObject::connect(m_motionTimer, &QTimer::timeout, [=]{
            if ( WindowHelper::check_button_state(Qt::LeftButton) ) {
                if ( need_to_check_motion ) {
                    QMoveEvent _e{QCursor::pos(), m_window->pos()};
                    QApplication::sendEvent(m_window, &_e);
                }
            } else {
                m_motionTimer->stop();
                sendButtonRelease();
#ifndef DONT_USE_GTK_MAINWINDOW
                xcb_window_t wnd = m_window->property("gtk_window_xid").value<unsigned long>();
                XcbUtils::sendNativeFocusTo(wnd, 1);
#endif
                QApplication::postEvent(m_window, new QEvent(static_cast<QEvent::Type>(UM_ENDMOVE)));
                m_window->activateWindow();
//                QTimer::singleShot(25, [=]() {
//                    if (m_window->size() == m_startSize)
//                        QApplication::postEvent(m_window, new QEvent(QEvent::User));
//                });
            }
        });
    }

    if (m_nDirection >= 0 && e->buttons() == Qt::LeftButton)
    {
#ifdef DONT_USE_GTK_MAINWINDOW
        Window wnd = (Window)m_window->winId();
#else
        Window wnd = m_window->property("gtk_window_xid").value<unsigned long>();
#endif
        Display * xdisplay_ = QX11Info::display();
        Window x_root_window_ = DefaultRootWindow(xdisplay_);

        XUngrabPointer(xdisplay_, CurrentTime);
        if ( !m_motionTimer->isActive() ) m_motionTimer->start(MOTION_TIMER_MS);

        XEvent event;
        memset(&event, 0, sizeof(event));
        event.xclient.type = ClientMessage;
        event.xclient.display = xdisplay_;
        event.xclient.window = wnd;
//        event.xclient.message_type = XInternAtom(xdisplay_, "_NET_WM_MOVERESIZE", false);
        event.xclient.message_type = GetAtom("_NET_WM_MOVERESIZE");
        event.xclient.format = 32;
        event.xclient.data.l[0] = e->globalPos().x();
        event.xclient.data.l[1] = e->globalPos().y();
        event.xclient.data.l[2] = m_nDirection;
        event.xclient.data.l[3] = Button1;
        event.xclient.data.l[4] = 0;

        XSendEvent(xdisplay_, x_root_window_, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
        XFlush(xdisplay_);
        m_startSize = m_window->size();
        m_nDirection = -1;
    }
    else
    {
        QPoint p(e->pos().x(), e->pos().y());
        checkCursor(p);
    }
}

void CX11Decoration::dispatchMouseUp(QMouseEvent *)
{
    m_nDirection = -1;
}

void CX11Decoration::turnOn()
{
//    switchDecoration(true);

//    Display * _xdisplay = QX11Info::display();
//    Atom _atom = XInternAtom(_xdisplay, "_MOTIF_WM_HINTS", true);
//    if ( _atom != None  && XDeleteProperty(_xdisplay, m_window->winId(), _atom) < BadValue ) {
//        m_decoration = true;
//        XFlush(_xdisplay);
//    }
}

void CX11Decoration::turnOff()
{
//    switchDecoration(false);
#ifdef DONT_USE_GTK_MAINWINDOW
    m_window->setWindowFlags(Qt::FramelessWindowHint);
#else
    m_window->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
#endif
    m_decoration = false;
}

void CX11Decoration::switchDecoration(bool on)
{
#ifdef DONT_USE_GTK_MAINWINDOW
    if (m_decoration != on) {
        // Signals that the reader of the _MOTIF_WM_HINTS property should pay
        // attention to the value of |decorations|.

        MotifWmHints motif_hints = {MWM_HINTS_DECORATIONS, 0, 0, 0, 0};
        motif_hints.decorations = int(on);

        Display * _xdisplay = QX11Info::display();
//        Atom hint_atom = XInternAtom(_xdisplay, "_MOTIF_WM_HINTS", false);
        Atom hint_atom = GetAtom("_MOTIF_WM_HINTS");
        if ( hint_atom != None &&
                    XChangeProperty(_xdisplay, m_window->winId(),
                               hint_atom, hint_atom, 32, PropModeReplace,
                               (unsigned char *)&motif_hints, sizeof(MotifWmHints)/sizeof(long)) < BadValue )
        {
            m_decoration = on;
            XFlush(_xdisplay);
        }
    }
#endif
}

bool CX11Decoration::isDecorated()
{
    return m_decoration;
}

void CX11Decoration::setMaximized(bool bVal)
{
    m_bIsMaximized = bVal;
}

void CX11Decoration::onDpiChanged(double f)
{
    dpi_ratio = f;
    m_nBorderSize = CUSTOM_BORDER_WIDTH * dpi_ratio;
}

bool CX11Decoration::isNativeFocus()
{
#ifdef DONT_USE_GTK_MAINWINDOW
    Window wnd = (Window)m_window->winId();
#else
    Window wnd = m_window->property("gtk_window_xid").value<unsigned long>();
#endif
    return (wnd == None) ? false : XcbUtils::isNativeFocus(wnd);
}

int CX11Decoration::customWindowBorderWith()
{
    return CUSTOM_BORDER_WIDTH;
}

void CX11Decoration::raiseWindow()
{
#ifdef DONT_USE_GTK_MAINWINDOW
    Window wnd = (Window)m_window->winId();
#else
    Window wnd = m_window->property("gtk_window_xid").value<unsigned long>();
#endif
    Display *disp = QX11Info::display();
    Atom atom_active_wnd = XInternAtom(disp, "_NET_ACTIVE_WINDOW", False);
    if (atom_active_wnd == None)
        return;

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

void CX11Decoration::sendButtonRelease()
{
//#ifdef DONT_USE_GTK_MAINWINDOW
    Window wnd = (Window)m_window->winId();
// #else
//     Window wnd = m_window->property("gtk_window_xid").value<unsigned long>();
//     if (wnd == None)
//         return;
// #endif
    Display * xdisplay_ = QX11Info::display();
    Window x_root_window_ = wnd;

    XEvent event;
    memset(&event, 0, sizeof(XEvent));

    event.type = ButtonRelease;
    event.xbutton.button = Button1;
    event.xbutton.same_screen = True;

//    event.xbutton.root = x_root_window_;
//    event.xbutton.window = m_window->winId();

    XQueryPointer(xdisplay_, x_root_window_, &event.xbutton.root, &event.xbutton.window,
                        &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    XSendEvent(xdisplay_, PointerWindow, True, ButtonReleaseMask, &event);
    XFlush(xdisplay_);
}

void CX11Decoration::setCursorPos(int x, int y)
{
    Display *xdisplay_= QX11Info::display();
    Window root_window = DefaultRootWindow(xdisplay_);
    XSelectInput(xdisplay_, root_window, KeyReleaseMask);
    XWarpPointer(xdisplay_, None, root_window, 0, 0, 0, 0, x, y);
    XFlush(xdisplay_);
}

void CX11Decoration::setMinimized()
{
#ifdef DONT_USE_GTK_MAINWINDOW
    Window wnd = (Window)m_window->winId();
#else
    Window wnd = m_window->property("gtk_window_xid").value<unsigned long>();
#endif
    XClientMessageEvent ev;
    ev.type = ClientMessage;
    ev.window = wnd;
    ev.message_type = GetAtom("WM_CHANGE_STATE");
    ev.format = 32;
    ev.data.l[0] = IconicState;

    Display * xdisplay_ = QX11Info::display();
    XSendEvent(xdisplay_, RootWindow(xdisplay_, DefaultScreen(xdisplay_)), False,
                    SubstructureRedirectMask|SubstructureNotifyMask, (XEvent *)&ev);
    sendButtonRelease();
}
