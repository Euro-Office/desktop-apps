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
