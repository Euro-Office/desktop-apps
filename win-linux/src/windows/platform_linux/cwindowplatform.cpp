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
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "platform_linux/xcbutils.h"
#include "utils.h"
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QX11Info>

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "platform_linux/cdialogopenssl.h"
#endif

#define RESIZABLE_AREA_WIDTH  9


AscMainPanel::AscMainPanel(QWidget *parent)
    : QWidget(parent)
    , parent(static_cast<CWindowBase*>(parent))
{}

AscMainPanel::~AscMainPanel()
{}

void AscMainPanel::resizeEvent(QResizeEvent *re)
{
    QWidget::resizeEvent(re);
    if ( !parent->m_shouldUseThinFrame ) {
        return;
    }

    double radius = 0;
    if ( !parent->isMaximized() )
        radius = WINDOW_CORNER_RADIUS * parent->m_dpiRatio;

    QPainterPath path;
    path.addRoundedRect(rect(), radius, radius);

    QRegion rg(path.toFillPolygon().toPolygon());
    setMask(rg);
}


CWindowPlatform::CWindowPlatform(const QRect &rect) :
    CWindowBase(rect),
    CX11Decoration(this),
    m_efectiveFrameMargin(0),
    m_isWindowActive(false)
{
    if (AscAppManager::isRtlEnabled())
        setLayoutDirection(Qt::RightToLeft);
    if (isCustomWindowStyle()) {
        CX11Decoration::turnOff();
        if ( m_shouldUseThinFrame )
            setAttribute(Qt::WA_TranslucentBackground);

        m_efectiveFrameMargin = CX11Decoration::effectiveFrameMargin();
    }
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
    QMainWindow::show();
    if (maximized) {
        QMainWindow::setWindowState(Qt::WindowMaximized);
    }
}

void CWindowPlatform::setWindowColors(const QColor& background, const QColor& border, bool isActive)
{
    Q_UNUSED(border)
    if (!CX11Decoration::isDecorated()) {
        m_brdColor = border;
        setStyleSheet(QString("QMainWindow{border:1px solid %1; background-color: %2;}").arg(border.name(), background.name()));
    }
}

void CWindowPlatform::adjustGeometry()
{
    if ( !CX11Decoration::isDecorated() ) {
        m_efectiveFrameMargin = CX11Decoration::effectiveFrameMargin();
        int top = m_efectiveFrameMargin;
        int bottom = m_efectiveFrameMargin;
        if ( m_shouldUseThinFrame ) {
            if ( m_efectiveFrameMargin != 0 ) {
                top -= int(SHADOW_OFFSET_Y * m_dpiRatio);
                bottom += int(SHADOW_OFFSET_Y * m_dpiRatio);
            }
            CX11Decoration::updateFrameExtents();
        }
        setContentsMargins(m_efectiveFrameMargin, top, m_efectiveFrameMargin, bottom);
    }
}

/** Protected **/

void CWindowPlatform::onWindowActivate(bool is_active)
{
    m_isWindowActive = is_active;
    for (auto *btn : m_pTopButtons) {
        if (btn)
            btn->setFaded(!is_active);
    }
    if ( m_shouldUseThinFrame ) {
        update();
    }
}

void CWindowPlatform::onMinimizeEvent()
{
    CX11Decoration::setMinimized();
}

bool CWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange) {
        CX11Decoration::setMaximized(isMaximized());
        applyWindowState();
        adjustGeometry();
    } else
    if (event->type() == QEvent::Resize) {
        if ( m_shouldUseThinFrame )
            updateInputShape();
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
    if (event->type() == QEvent::WindowActivate) {
        onWindowActivate(true);
    }
    else
    if (event->type() == QEvent::WindowDeactivate) {
        onWindowActivate(false);
    }
    return CWindowBase::event(event);
}

bool CWindowPlatform::nativeEvent(const QByteArray &ev_type, void *msg, long *res)
{
    if (ev_type == "xcb_generic_event_t") {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t*>(msg);
        switch (ev->response_type & ~0x80) {
        case XCB_FOCUS_IN:
            if (isNativeFocus()) {
                focus();
                m_propertyTimer->stop();
                if (property("stabilized").toBool())
                    setProperty("stabilized", false);
                m_propertyTimer->start();
            }
            break;
        default:
            break;
        }
    }
    return CWindowBase::nativeEvent(ev_type, msg, res);
}

void CWindowPlatform::setScreenScalingFactor(double factor, bool resize)
{
    CX11Decoration::onDpiChanged(factor);
    CWindowBase::setScreenScalingFactor(factor, resize);
}

void CWindowPlatform::updateInputShape()
{
    if ( !m_shouldUseThinFrame || !isVisible() )
        return;

    const int resizable_area = RESIZABLE_AREA_WIDTH * dpi_ratio;

    xcb_rectangle_t rc = {};
    if (m_efectiveFrameMargin != 0) {
        rc.x = m_efectiveFrameMargin - resizable_area;
        rc.y = m_efectiveFrameMargin - resizable_area - int(SHADOW_OFFSET_Y * m_dpiRatio);
        rc.width = width() - 2 * (m_efectiveFrameMargin - resizable_area);
        rc.height = height() - 2 * (m_efectiveFrameMargin - resizable_area);
    }
    XcbUtils::setInputShape((xcb_window_t)winId(), rc);
}

void CWindowPlatform::paintEvent(QPaintEvent *event)
{
    if ( !m_shouldUseThinFrame ) {
        CWindowBase::paintEvent(event);
        return;
    }

    int cornerRadius = 0;
    const int borderWidth = WINDOW_THIN_BORDER_WIDTH * m_dpiRatio;
    QRectF rc = rect();
    QPainter pnt(this);
    pnt.setRenderHint(QPainter::Antialiasing);

    if (m_efectiveFrameMargin > 0) {
        // Draw shadow
        const int shadowWidth = SHADOW_WIDTH * m_dpiRatio;
        const int shadowOffset = SHADOW_OFFSET_Y * m_dpiRatio;
        const int shadowRadius = SHADOW_RADIUS * m_dpiRatio;
        const int transparentAreaWidth = 9 * m_dpiRatio;
        constexpr int alphaThreshold = 2;
        int shadowTransparency = m_isWindowActive ? SHADOW_ALPHA_ACTIVE : SHADOW_ALPHA_INACTIVE;

        for (int i = transparentAreaWidth; i < shadowWidth + abs(shadowOffset) + 1; ++i) {
            double t = static_cast<double>(i) / (shadowWidth + shadowOffset);
            int alpha = shadowTransparency * std::pow(t, 3);
            if (alpha < alphaThreshold) continue;

            int x = rc.left() + i + 1;
            int y = rc.top() + i + 1;
            int w = rc.width() - i * 2 - 2;
            int h = rc.height() - i * 2 - 2;

            double radiusFactor = (2.0 - static_cast<double>(i) / (shadowWidth + shadowOffset)) * shadowRadius;

            QPainterPath path;
            path.addRoundedRect(x, y, w, h, radiusFactor, radiusFactor);

            QPen pen(QColor(0, 0, 0, alpha), 1);
            pnt.strokePath(path, pen);
        }

        const double halfBorderWidth = borderWidth / 2.0;
        cornerRadius = WINDOW_CORNER_RADIUS * m_dpiRatio;
        rc.adjust(shadowWidth + halfBorderWidth, shadowWidth - shadowOffset + halfBorderWidth,
                  -shadowWidth - halfBorderWidth, -shadowWidth - shadowOffset - halfBorderWidth);
    }

    QPainterPath path;
    path.addRoundedRect(rc, cornerRadius, cornerRadius);
    // Draw background
    pnt.fillPath(path, palette().window().color());    
    if (m_efectiveFrameMargin > 0) {
        // Draw border
        pnt.strokePath(path, QPen(m_brdColor, borderWidth));
    }
    pnt.end();
}

/** Private **/

void CWindowPlatform::mouseMoveEvent(QMouseEvent *e)
{
    if (!property("blocked").toBool())
        CX11Decoration::dispatchMouseMove(e);
}

void CWindowPlatform::mousePressEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseDown(e);
}

void CWindowPlatform::mouseReleaseEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseUp(e);
}

void CWindowPlatform::mouseDoubleClickEvent(QMouseEvent *me)
{
    if (m_boxTitleBtns) {
        QRect titleRect = m_boxTitleBtns->geometry().translated(m_efectiveFrameMargin, m_efectiveFrameMargin);
        if (titleRect.contains(me->pos()))
            onMaximizeEvent();
    }
}
