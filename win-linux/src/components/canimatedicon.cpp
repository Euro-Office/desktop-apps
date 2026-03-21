#include "components/canimatedicon.h"
#include <QPainter>
#include <QVariant>
#include <qtcomp/qnativeevent.h>

#define RELEASE_OBJECT(obj) if (obj) delete obj, obj = nullptr;

CAnimatedIcon::CAnimatedIcon(QWidget * parent)
    : QLabel(parent)
{
    setProperty("class", "tab-icon");
}

void CAnimatedIcon::setPixmap(const QPixmap & pixmap, bool forcestop)
{
    if ( m_svgSize != pixmap.size() ) {
        m_svgSize = QSize(pixmap.size());

        if ( m_image ) {
            delete m_image, m_image = nullptr;

            m_image = new QPixmap(pixmap.size());
        }
    }

    if ( !forcestop && isStarted() ) {
        RELEASE_OBJECT(m_static);
        m_static = new QPixmap(pixmap);
    } else {
        RELEASE_OBJECT(m_svg);
        RELEASE_OBJECT(m_image);

        RELEASE_OBJECT(m_static);
        m_static = new QPixmap(pixmap);

        QLabel::setPixmap(pixmap);
    }
}

void CAnimatedIcon::startSvg(const QString& source, const QString& eid)
{
    if ( !m_svg ) m_svg = new QSvgRenderer(this);
    else disconnect(m_svg);

    if ( m_svg->load(source) ) {
//        setFixedSize( m_svg->defaultSize() );

        if ( !m_static ) {
            m_static = QtComp::Widget::copyPixmap(this);
        }

        if ( m_svg->animated() ) {
            if ( !eid.isEmpty() ) m_svgElemId = eid;

            if ( m_image ) delete m_image;
            int icon_size = qMin(width(), height());
            m_image = new QPixmap(icon_size, icon_size);

            connect(m_svg, &QSvgRenderer::repaintNeeded, this, &CAnimatedIcon::onSvgRepaint);
        } else {
            QPixmap image( m_svg->defaultSize() );
            QPainter painter( &image );

            if ( eid.isEmpty() )
                m_svg->render( &painter );
            else m_svg->render( &painter, eid );

            RELEASE_OBJECT(m_svg);
            QLabel::setPixmap( image );
        }
    }
}

void CAnimatedIcon::stop()
{
    RELEASE_OBJECT(m_svg);
    if ( m_static ) {
        QLabel::setPixmap( *m_static );
    }
}

void CAnimatedIcon::onSvgRepaint()
{
    if ( m_image ) {
        m_image->fill(Qt::transparent);
        QPainter painter( m_image );

        if ( m_svgElemId.isEmpty() )
            m_svg->render( &painter );
        else m_svg->render( &painter, m_svgElemId );

        QLabel::setPixmap( *m_image );
    }
}

void CAnimatedIcon::setSvgElement(const QString& id)
{
        m_svgElemId = id;
}

bool CAnimatedIcon::isStarted()
{
    if ( m_svg && m_svg->animated() )
        return true;

    return false;
}

void CAnimatedIcon::setIconSize(const QSize& s, bool forcegeometry)
{
    Q_UNUSED(forcegeometry)

    m_svgSize = QSize(s);
    if ( isStarted() ) {
        RELEASE_OBJECT(m_image);
        m_image = new QPixmap(s);
    }
}
