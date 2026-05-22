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
