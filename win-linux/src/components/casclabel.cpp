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

#include "casclabel.h"
#include <QGraphicsOpacityEffect>
#include <QResizeEvent>

#include <QDebug>

CAscLabel::CAscLabel(QWidget * parent) :
    QLabel(parent)
{

}

CAscLabel::CAscLabel(const QString& caption, QWidget * parent) :
    QLabel(caption, parent)
{
    QLinearGradient alphaGradient(rect().topLeft(), rect().topRight());

    alphaGradient.setColorAt(0.8, Qt::black);
    alphaGradient.setColorAt(1.0, Qt::transparent);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    effect->setOpacityMask(alphaGradient);

    setGraphicsEffect(effect);
}

CAscLabel::~CAscLabel()
{

}

void CAscLabel::paintEvent(QPaintEvent * e)
{
//    QPainter p(this);
//    QFontMetrics fm(font());

//    if (fm.width(text()) > contentsRect().width()) {
//        QString elided_txt;

//        if(ELIDE_MIDDLE) // ELIDE_MIDDLE is part of a class enum
//            elided_txt = this->fontMetrics().elidedText(text(), Qt::ElideMiddle, rect().width(), Qt::TextShowMnemonic);
//        else { //Handle all other elide modes you want to support.
//        }

//        p.drawText(rect(), elided_txt);
//    } else
        QLabel::paintEvent(e);
}

void CAscLabel::resizeEvent(QResizeEvent * e) {
    QLabel::resizeEvent(e);

    QLinearGradient alphaGradient(QPointF(0,0), QPointF(e->size().width(),0));

    alphaGradient.setColorAt(0.8, Qt::black);
    alphaGradient.setColorAt(1.0, Qt::transparent);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    effect->setOpacityMask(alphaGradient);

    setGraphicsEffect(effect);
}
