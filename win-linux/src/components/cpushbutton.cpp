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

#include "components/cpushbutton.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QVariantAnimation>
#include <QGraphicsOpacityEffect>
#include <QEvent>

#define ANIMATION_MS 2500


CPushButton::CPushButton(QWidget *parent)
    : QPushButton(parent)
{}

CPushButton::~CPushButton()
{
    releaseSvg();
}

void CPushButton::setAnimatedIcon(const QString &path)
{
    releaseSvg();
    if (path.isEmpty())
        return;
    m_renderer = new QSvgRenderer(path, this);

    m_animation = new QVariantAnimation(this);
    m_animation->setStartValue(0);
    m_animation->setEndValue(360);
    m_animation->setDuration(ANIMATION_MS);
    m_animation->setLoopCount(-1);
    m_animation->setEasingCurve(QEasingCurve::Linear);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [=](const QVariant &val) {
        onSvgRepaint(val.toDouble());
    });
    m_animation->start(QAbstractAnimation::KeepWhenStopped);
}

void CPushButton::setStaticIcon(const QString &path)
{
    releaseSvg();
    if (path.isEmpty())
        return;
//    m_renderer = new QSvgRenderer(path, this);
//    onSvgRepaint(0);
    setIcon(QIcon(path));
}

void CPushButton::setFaded(bool faded)
{
    m_faded = faded;
    setOpacity(m_faded ? 0.5 : 1.0);
}

bool CPushButton::isStarted()
{
    return m_animation && m_animation->state() == QAbstractAnimation::Running;
}

bool CPushButton::event(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::Enter:
        if (m_faded)
            setOpacity(1.0);
        break;
    case QEvent::Leave:
        if (m_faded)
            setOpacity(0.5);
        break;
    default:
        break;
    }
    return QPushButton::event(ev);
}

void CPushButton::releaseSvg()
{
    if (m_animation) {
        if (m_animation->state() != QAbstractAnimation::Stopped)
            m_animation->stop();
        m_animation->disconnect();
        delete m_animation, m_animation = nullptr;
    }
    if (m_renderer)
        delete m_renderer, m_renderer = nullptr;
}

void CPushButton::onSvgRepaint(double angle)
{
    if (m_renderer && m_renderer->isValid()) {
        QSize icon_size = iconSize();
        double offset = (double)icon_size.height()/2;
        QPixmap pixmap(icon_size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(offset, offset);
        painter.rotate(angle);
        painter.translate(-offset, -offset);
        m_renderer->render(&painter);
        painter.end();
        setIcon(QIcon(pixmap));
    }
}

void CPushButton::setOpacity(double opacity)
{
    QGraphicsOpacityEffect *efct = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!efct) {
        efct = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(efct);
    }
    efct->setOpacity(opacity);
}
