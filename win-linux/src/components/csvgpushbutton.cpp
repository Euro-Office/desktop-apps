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

#include "components/csvgpushbutton.h"
#include <QStyle>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QTextStream>


CSVGPushButton::CSVGPushButton(QWidget * parent)
    : QPushButton(parent)
    , m_opacitynormal(QColor(255,255,255,255))
    , m_opacitydisabled(QColor(255,255,255,100))
{

}

void CSVGPushButton::setIcon(const QByteArray& svgstr)
{
    m_svglayout = svgstr;
    updateIcon();
}

void CSVGPushButton::setIcon(const QString& path, const QString& node)
{
    m_svgnode = node;

    QFile _f(path);
    if( _f.open(QIODevice::ReadOnly)) {
        QTextStream in(&_f);

        m_svglayout = in.readAll().toLocal8Bit();

        _f.close();

        updateIcon();
    }
}

void CSVGPushButton::setIconSize(const QSize& size)
{
    QPushButton::setIconSize(size);
    updateIcon();
}

void CSVGPushButton::setIconOpacity(const QColor& c)
{
    m_opacitynormal = c;
    m_usestateopacity = true;
    updateIcon();
}

void CSVGPushButton::setDisabled(bool status)
{
    QPushButton::setDisabled(status);
    updateIcon();
}

void CSVGPushButton::setUseStateOpacity(bool value)
{
    m_usestateopacity = value;
}

void CSVGPushButton::setFillDark(bool dark)
{
    if ( dark )
        m_opacitynormal = QColor(0,0,0,200),
        m_opacitydisabled = QColor(0,0,0,100);
    else
        m_opacitynormal = QColor(255,255,255,255),
        m_opacitydisabled = QColor(255,255,255,100);

    updateIcon();
}

void CSVGPushButton::updateIcon()
{
    if ( !m_svglayout.isEmpty() ) {
        QImage img(iconSize(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPixmap pixmap = QPixmap::fromImage(img, Qt::NoFormatConversion);

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        QSvgRenderer r(m_svglayout);
        if ( m_svgnode.isEmpty() ) r.render(&painter);
        else {
            QRectF bounds = r.boundsOnElement(m_svgnode);
            double kx = pixmap.width() / r.viewBoxF().width();
            double ky = pixmap.height() / r.viewBoxF().height();
            r.render(&painter, m_svgnode, QRectF(bounds.x() * kx, bounds.y() * ky, bounds.width() * kx, bounds.height() * ky));
        }

        if ( m_usestateopacity ) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            if ( !isEnabled() ) {
                painter.fillRect(pixmap.rect(), m_opacitydisabled);
            } else {
                painter.fillRect(pixmap.rect(), m_opacitynormal);
            }
        }
        painter.end();

        QPushButton::setIcon(QIcon(pixmap));
    }
}

bool CSVGPushButton::setProperty(const char *name, const QVariant &value)
{
    auto retval = QPushButton::setProperty(name, value);
    style()->polish(this);
    return retval;
}
