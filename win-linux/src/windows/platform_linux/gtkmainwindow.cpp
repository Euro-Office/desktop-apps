#include "gtkmainwindow.h"
#include "platform_linux/xcbutils.h"
#include "utils.h"
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#pragma push_macro("signals")
#undef signals
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <cairo.h>
#pragma pop_macro("signals")

#define WINDOW_CORNER_RADIUS_DEFAULT   0
#define WINDOW_CORNER_RADIUS_GNOME     8
#define WINDOW_CORNER_RADIUS_KDE       6
#define WINDOW_CORNER_RADIUS_CINNAMON  3
#define WINDOW_CORNER_RADIUS_XFCE      4


static QMargins GetFrameBounds(GtkWidget *wnd)
{
    QMargins frame;
    if (GdkWindow *gdk_wnd = gtk_widget_get_window(wnd)) {
        Display *xdsp = GDK_WINDOW_XDISPLAY(gdk_wnd);
        Window xwnd = GDK_WINDOW_XID(gdk_wnd);
        Atom prop = gdk_x11_get_xatom_by_name("_GTK_FRAME_EXTENTS");
        if (xdsp && xwnd != None && prop != None) {
            Atom type = 0;
            int format = 0;
            unsigned long nitems = 0, bytes_after = 0;
            unsigned char *data = nullptr;
            if (XGetWindowProperty(xdsp, xwnd, prop, 0, 4, False, XA_CARDINAL, &type, &format, &nitems, &bytes_after, &data) == Success && data) {
                if (nitems == 4) {
                    long *ext = (long*)data;
                    frame = QMargins(ext[0], ext[2], ext[1], ext[3]);
                }
                XFree(data);
            }
        }
    }
    return frame;
}

class GtkMainWindowPrivate
{
public:
    GtkMainWindowPrivate();
    ~GtkMainWindowPrivate();

    void init();

    QWidget *underlay = nullptr;
    GtkWidget *wnd = nullptr;
    GtkWidget *socket = nullptr;
    guint state = 0;
    FnCloseEvent close_event;
    FnEvent event;
    QPoint normalPos;
    QSize min_size, max_size, normalSize;
    QMargins frame;
    int x = 0, y = 0, width = 0, height = 0;
    bool is_maximized = false,
         is_fullscreen = false,
         is_custom_style = false,
         is_focused = false,
         is_support_round_corners = true;

private:
    enum Corner {
        CornerLTop = 1,
        CornerRTop = 2,
        CornerLBottom = 4,
        CornerRBottom = 8,
        CornerAll  = CornerLTop | CornerRTop | CornerLBottom | CornerRBottom
    };

    int cornersPlacementAndRadius(int &radius);
    static gboolean on_event(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void on_event_after(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void set_content_rounded_corners(GtkWidget *wgt, int crn, double rad);
    static void on_content_size_allocate(GtkWidget *wgt, GdkRectangle *alloc, gpointer data);

    int radius = 0,
        corners = CornerAll;
};

GtkMainWindowPrivate::GtkMainWindowPrivate() :
    min_size(0, 0),
    max_size(G_MAXSHORT, G_MAXSHORT)
{}

GtkMainWindowPrivate::~GtkMainWindowPrivate()
{
    GdkWindow *gdk_wnd = gtk_widget_get_window(wnd);
    gpointer q_underlay_xid = g_object_get_data(G_OBJECT(gdk_wnd), "qt_underlay_xid");
    if (q_underlay_xid)
        g_free(q_underlay_xid);
    gtk_widget_destroy(GTK_WIDGET(wnd));
    wnd = nullptr;
}

void GtkMainWindowPrivate::init()
{
    wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(wnd), GdkWindowTypeHint::GDK_WINDOW_TYPE_HINT_NORMAL);

    gtk_widget_set_app_paintable(wnd, TRUE);
    GdkScreen *scr = gtk_widget_get_screen(wnd);
    if (GdkVisual *vis = gdk_screen_get_rgba_visual(scr))
        gtk_widget_set_visual(wnd, vis);

    if (is_custom_style) {
        if (gdk_screen_is_composited(scr)) {
            corners = cornersPlacementAndRadius(radius);
            GtkWidget *header = gtk_header_bar_new();
            gtk_window_set_titlebar(GTK_WINDOW(wnd), header);
            gtk_widget_destroy(header);
        } else {
            is_support_round_corners = false;
            gtk_window_set_decorated(GTK_WINDOW(wnd), FALSE);
        }
    } else {
        is_support_round_corners = false;
    }

    char style[256];
    snprintf(style, sizeof(style), "decoration {border-radius: %dpx %dpx %dpx %dpx;}", radius, radius,
             corners == CornerAll ? radius : 0, corners == CornerAll ? radius : 0);
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, style, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(wnd);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    gtk_widget_realize(wnd);

    if (WindowHelper::getEnvInfo() == WindowHelper::DesktopEnv::XFCE)
        frame = GetFrameBounds(wnd);

    socket = gtk_socket_new();
    /* Call the show according to the GtkSocket documentation */
    gtk_widget_show(socket);
    gtk_container_add(GTK_CONTAINER(wnd), socket);
    /* The following call is only necessary if one of
    the ancestors of the socket is not yet visible
    (according to the GtkSocket documentation) */
    gtk_widget_realize(socket);

    gtk_socket_add_id(GTK_SOCKET(socket), (Window)underlay->winId());

    g_signal_connect(G_OBJECT(socket), "size-allocate", G_CALLBACK(on_content_size_allocate), this);
    g_signal_connect(G_OBJECT(wnd), "event", G_CALLBACK(on_event), this);
    g_signal_connect(G_OBJECT(wnd), "event-after", G_CALLBACK(on_event_after), this);        
}

int GtkMainWindowPrivate::cornersPlacementAndRadius(int &radius)
{
    if (!is_support_round_corners) {
        radius = WINDOW_CORNER_RADIUS_DEFAULT;
        return CornerAll;
    }
    switch (WindowHelper::getEnvInfo()) {
    case WindowHelper::DesktopEnv::GNOME:
        radius = WINDOW_CORNER_RADIUS_GNOME;
        break;

    case WindowHelper::DesktopEnv::KDE:
        radius = WINDOW_CORNER_RADIUS_KDE;
        return CornerLTop | CornerRTop;

    case WindowHelper::DesktopEnv::UNITY:
        radius = WINDOW_CORNER_RADIUS_DEFAULT;
        break;

    case WindowHelper::DesktopEnv::CINNAMON:
        radius = WINDOW_CORNER_RADIUS_CINNAMON;
        return CornerLTop | CornerRTop;

    case WindowHelper::DesktopEnv::XFCE:
        radius = WINDOW_CORNER_RADIUS_XFCE;
        return CornerLTop | CornerRTop;

    default:
        radius = WINDOW_CORNER_RADIUS_DEFAULT;
        break;
    }
    return CornerAll;
}

gboolean GtkMainWindowPrivate::on_event(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    GtkMainWindowPrivate *pimpl = (GtkMainWindowPrivate*)data;
    switch (ev->type) {
    case GDK_DELETE: {
        QCloseEvent qtcev;
        (pimpl->close_event)(&qtcev);
        return !qtcev.isAccepted();
    }

    default:
        break;
    }
    return FALSE;
}

void GtkMainWindowPrivate::on_event_after(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    GtkMainWindowPrivate *pimpl = (GtkMainWindowPrivate*)data;
    switch (ev->type) {
    case GDK_CONFIGURE: {
        gint x, y, w, h;
        gtk_window_get_size(GTK_WINDOW(pimpl->wnd), &w, &h);
        gtk_window_get_position(GTK_WINDOW(pimpl->wnd), &x, &y);

        if (pimpl->x != x || pimpl->y != y) {
            pimpl->x = x;
            pimpl->y = y;

            gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
            x *= dpi; y *= dpi;
            if (!pimpl->is_maximized && !pimpl->is_fullscreen) {
                x += pimpl->frame.left();
                y += pimpl->frame.top();
            }

            XcbUtils::sendConfigureNotify(pimpl->underlay->winId(), x, y, pimpl->underlay->width(), pimpl->underlay->height());
        }

        if (pimpl->width != w || pimpl->height != h) {
            pimpl->width = w;
            pimpl->height = h;

            gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
            w *= dpi; h *= dpi;
            if (!pimpl->is_maximized && !pimpl->is_fullscreen) {
                w -= pimpl->frame.left() + pimpl->frame.right();
                h -= pimpl->frame.top() + pimpl->frame.bottom();
            }

            pimpl->underlay->resize(w, h);
            // pimpl->underlay->update();
        }
        break;
    }

    case GDK_FOCUS_CHANGE: {
        if ((pimpl->is_focused = ev->focus_change.in) == 1)
            XcbUtils::sendNativeFocusTo(pimpl->underlay->winId(), 1);
        break;
    }

    case GDK_WINDOW_STATE: {
        const guint mask = (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN);
        if (ev->window_state.changed_mask & mask) {
            pimpl->state = ev->window_state.new_window_state & mask;

            // if ((pimpl->state & GDK_WINDOW_STATE_MAXIMIZED && !pimpl->is_fullscreen)
            //         || (pimpl->state & GDK_WINDOW_STATE_FULLSCREEN && !pimpl->is_maximized)) {
            //     gint x, y, w, h;
            //     gtk_window_get_size(GTK_WINDOW(pimpl->wnd), &w, &h);
            //     gtk_window_get_position(GTK_WINDOW(pimpl->wnd), &x, &y);

            //     gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
            //     x *= dpi; y *= dpi;
            //     w *= dpi; h *= dpi;
            //     if (!pimpl->is_maximized && !pimpl->is_fullscreen) {
            //         w -= pimpl->frame.left() + pimpl->frame.right();
            //         h -= pimpl->frame.top() + pimpl->frame.bottom();
            //     }

            //     pimpl->normalPos = QPoint(x, y);
            //     pimpl->normalSize = QSize(w, h);
            // }

            pimpl->is_maximized = pimpl->state & GDK_WINDOW_STATE_MAXIMIZED;
            pimpl->is_fullscreen = pimpl->state & GDK_WINDOW_STATE_FULLSCREEN;

            QEvent ev(QEvent::WindowStateChange);
            (pimpl->event)(&ev);
        }
        break;
    }

    default:
        break;
    }
}

void GtkMainWindowPrivate::set_content_rounded_corners(GtkWidget *wgt, int crn, double rad)
{
    if (GdkWindow *gdk_window = gtk_widget_get_window(wgt)) {
        int w = gtk_widget_get_allocated_width(wgt);
        int h = gtk_widget_get_allocated_height(wgt);

        cairo_surface_t *sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cairo_t *cr = cairo_create(sfc);
        cairo_set_source_rgba(cr, 1, 1, 1, 1);
        cairo_move_to(cr, rad, 0);
        cairo_arc(cr, w - rad, rad, rad, -G_PI_2, 0);
        if (crn == CornerAll) {
            cairo_arc(cr, w - rad, h - rad, rad, 0, G_PI_2);
            cairo_arc(cr, rad, h - rad, rad, G_PI_2, G_PI);
        } else {
            cairo_line_to(cr, w, h);
            cairo_line_to(cr, 0, h);
        }
        cairo_line_to(cr, 0, rad);
        cairo_arc(cr, rad, rad, rad, G_PI, -G_PI_2);
        cairo_close_path(cr);
        cairo_fill(cr);

        cairo_surface_t *sfc_tgt = cairo_get_target(cr);
        cairo_surface_flush(sfc_tgt);

        cairo_region_t *mask = gdk_cairo_region_create_from_surface(sfc_tgt);
        gdk_window_shape_combine_region(gdk_window, mask, 0, 0);

        cairo_region_destroy(mask);
        cairo_destroy(cr);
        cairo_surface_destroy(sfc);
    }
}

void GtkMainWindowPrivate::on_content_size_allocate(GtkWidget *wgt, GdkRectangle*, gpointer data)
{
    GtkMainWindowPrivate *pimpl = (GtkMainWindowPrivate*)data;
    if (pimpl->is_support_round_corners)
        set_content_rounded_corners(wgt, pimpl->corners, pimpl->is_maximized ? 0 : 1.18 * pimpl->radius);
}


GtkMainWindow::GtkMainWindow(QWidget *underlay, bool isCustomStyle, const FnEvent &qev, const FnCloseEvent &qcev) :
    pimpl(new GtkMainWindowPrivate)
{
    pimpl->is_custom_style = isCustomStyle;
    pimpl->event = qev;
    pimpl->close_event = qcev;
    pimpl->underlay = underlay;
    pimpl->init();
}

GtkMainWindow::~GtkMainWindow()
{
    delete pimpl, pimpl = nullptr;
}

void GtkMainWindow::move(const QPoint &pos)
{
    int x = pos.x(), y = pos.y();
    if (!pimpl->is_maximized && !pimpl->is_fullscreen) {
        x -= pimpl->frame.left();
        y -= pimpl->frame.top();
    }
    gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_window_move(GTK_WINDOW(pimpl->wnd), x/dpi, y/dpi);
    gdk_window_process_all_updates();
}

void GtkMainWindow::setGeometry(const QRect &rc)
{
    int x = rc.x(), y = rc.y();
    int w = rc.width(), h = rc.height();
    if (!pimpl->is_maximized && !pimpl->is_fullscreen) {
        x -= pimpl->frame.left();
        y -= pimpl->frame.top();
        w += pimpl->frame.left() + pimpl->frame.right();
        h += pimpl->frame.top() + pimpl->frame.bottom();
    }
    gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_window_resize(GTK_WINDOW(pimpl->wnd), w/dpi, h/dpi);
    gtk_window_move(GTK_WINDOW(pimpl->wnd), x/dpi, y/dpi);
    gdk_window_process_all_updates();
}

void GtkMainWindow::setWindowIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        QImage img = icon.pixmap(96, 96).toImage().rgbSwapped();
        if (!img.isNull()) {
            if (GdkPixbuf *pb = gdk_pixbuf_new_from_data(img.constBits(), GDK_COLORSPACE_RGB, TRUE, 8, img.width(), img.height(),
                                                         img.bytesPerLine(), NULL, NULL)) {
                gtk_window_set_icon(GTK_WINDOW(pimpl->wnd), pb);
                g_object_unref(pb);
            }
        }
    }
}

void GtkMainWindow::setWindowTitle(const QString &title)
{
    gtk_window_set_title(GTK_WINDOW(pimpl->wnd), title.toLocal8Bit().constData());
}

void GtkMainWindow::setBackgroundColor(const QString &color)
{
    GdkRGBA c;
    gdk_rgba_parse(&c, color.toLocal8Bit().constData());
    gtk_widget_override_background_color(pimpl->wnd, GTK_STATE_FLAG_NORMAL, &c);
}

void GtkMainWindow::setFocus()
{
    gtk_window_present(GTK_WINDOW(pimpl->wnd));
    XcbUtils::sendNativeFocusTo(pimpl->underlay->winId(), 1);
}

void GtkMainWindow::setWindowState(Qt::WindowStates ws)
{
    if (!ws.testFlag(Qt::WindowMaximized))
        gtk_window_unmaximize(GTK_WINDOW(pimpl->wnd));
    if (!ws.testFlag(Qt::WindowFullScreen))
        gtk_window_unfullscreen(GTK_WINDOW(pimpl->wnd));
    if (!ws.testFlag(Qt::WindowMinimized))
        gtk_window_deiconify(GTK_WINDOW(pimpl->wnd));
    if (ws.testFlag(Qt::WindowMaximized))
        gtk_window_maximize(GTK_WINDOW(pimpl->wnd));
    if (ws.testFlag(Qt::WindowMinimized))
        gtk_window_iconify(GTK_WINDOW(pimpl->wnd));
    if (ws.testFlag(Qt::WindowFullScreen))
        gtk_window_fullscreen(GTK_WINDOW(pimpl->wnd));
    if (ws.testFlag(Qt::WindowActive))
        gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::show()
{
    pimpl->underlay->show();
    gtk_widget_show_all(pimpl->wnd);

    GdkWindow *gdk_wnd = gtk_widget_get_window(pimpl->wnd);
    Window *qt_underlay_xid = (Window*)g_malloc(sizeof(Window));
    *qt_underlay_xid = (Window)pimpl->underlay->winId();
    g_object_set_data(G_OBJECT(gdk_wnd), "qt_underlay_xid", qt_underlay_xid);

    Window xid = GDK_WINDOW_XID(gdk_wnd);
    pimpl->underlay->setProperty("gtk_window_xid", QVariant::fromValue(xid));
    gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::showMinimized()
{
    gtk_window_iconify(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::showMaximized()
{
    gtk_window_deiconify(GTK_WINDOW(pimpl->wnd));
    gtk_window_maximize(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::showNormal()
{
    gtk_window_unmaximize(GTK_WINDOW(pimpl->wnd));
    gtk_window_unfullscreen(GTK_WINDOW(pimpl->wnd));
    gtk_window_deiconify(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::activateWindow()
{
    gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::setMinimumSize(int w, int h)
{
    gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
    pimpl->min_size = QSize(w/dpi, h/dpi);

    GdkGeometry hints = {};
    hints.min_width = w/dpi;
    hints.min_height = h/dpi;
    hints.max_width = pimpl->max_size.width();
    hints.max_height = pimpl->max_size.height();
    gtk_window_set_geometry_hints(GTK_WINDOW(pimpl->wnd), GTK_WIDGET(pimpl->wnd), &hints, GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));

}

void GtkMainWindow::hide() const
{
    gtk_widget_hide(pimpl->wnd);
}

bool GtkMainWindow::isMaximized()
{
    return pimpl->state & GDK_WINDOW_STATE_MAXIMIZED;
}

bool GtkMainWindow::isMinimized()
{
    return pimpl->state & GDK_WINDOW_STATE_ICONIFIED;
}

bool GtkMainWindow::isActiveWindow()
{
    return gtk_window_is_active(GTK_WINDOW(pimpl->wnd));
}

bool GtkMainWindow::isFocused()
{
    return pimpl->is_focused;
}

bool GtkMainWindow::isVisible() const
{
    return gtk_widget_is_visible(pimpl->wnd);
}

bool GtkMainWindow::isHidden() const
{
    return !gtk_widget_is_visible(pimpl->wnd);
}

QString GtkMainWindow::windowTitle() const
{
    return gtk_window_get_title(GTK_WINDOW(pimpl->wnd));
}

// QPoint GtkMainWindow::mapToGlobal(const QPoint &pt) const
// {
//     return (pt + geometry().topLeft());
// }

// QPoint GtkMainWindow::mapFromGlobal(const QPoint &pt) const
// {
//     return (pt - geometry().topLeft());
// }

QSize GtkMainWindow::size() const
{
    int w = 0, h = 0;
    gtk_window_get_size(GTK_WINDOW(pimpl->wnd), &w, &h);
    gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
    w *= dpi; h *= dpi;
    if (!pimpl->is_maximized && !pimpl->is_fullscreen) {
        w -= pimpl->frame.left() + pimpl->frame.right();
        h -= pimpl->frame.top() + pimpl->frame.bottom();
    }
    return QSize(w, h);
}

QPoint GtkMainWindow::pos() const
{
    int x, y;
    gtk_window_get_position(GTK_WINDOW(pimpl->wnd), &x, &y);
    gint dpi = gtk_widget_get_scale_factor(pimpl->wnd);
    x *= dpi; y *= dpi;
    if (!pimpl->is_maximized && !pimpl->is_fullscreen) {
        x += pimpl->frame.left();
        y += pimpl->frame.top();
    }
    return QPoint(x, y);
}

QRect GtkMainWindow::geometry() const
{
    return QRect(pos(), size());
}

QRect GtkMainWindow::normalGeometry() const
{
    if (pimpl->is_maximized || pimpl->is_fullscreen) {
        return QRect(pimpl->normalPos, pimpl->normalSize);
    } else {
        return geometry();
    }
}

Qt::WindowStates GtkMainWindow::windowState() const
{
    Qt::WindowStates ws;
    if (pimpl->state & GDK_WINDOW_STATE_MAXIMIZED)
        ws |= Qt::WindowMaximized;
    if (pimpl->state & GDK_WINDOW_STATE_ICONIFIED)
        ws |= Qt::WindowMinimized;
    if (pimpl->state & GDK_WINDOW_STATE_FULLSCREEN)
        ws |= Qt::WindowFullScreen;
    return ws;
}
