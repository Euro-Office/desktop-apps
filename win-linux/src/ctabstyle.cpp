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

#include "ctabstyle.h"

#include <QPainter>
#include <QTabBar>
#include <QDebug>


CTabStyle::CTabStyle()
    : QProxyStyle()
{

}

CTabStyle::~CTabStyle()
{

}

void CTabStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *widget) const {
    if (element == CE_TabBarTabLabel) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            QStyleOptionTabV3 tabV2(*tab);
            QRect tr = tabV2.rect;
            bool verticalTabs = tabV2.shape == QTabBar::RoundedEast
                                || tabV2.shape == QTabBar::RoundedWest
                                || tabV2.shape == QTabBar::TriangularEast
                                || tabV2.shape == QTabBar::TriangularWest;

            int alignment = Qt::AlignVCenter | Qt::AlignLeft | Qt::TextShowMnemonic;
            if (!proxy()->styleHint(SH_UnderlineShortcut, opt, widget))
                alignment |= Qt::TextHideMnemonic;

            if (verticalTabs) {
                p->save();
                int newX, newY, newRot;
                if (tabV2.shape == QTabBar::RoundedEast || tabV2.shape == QTabBar::TriangularEast) {
                    newX = tr.width() + tr.x();
                    newY = tr.y();
                    newRot = 90;
                } else {
                    newX = tr.x();
                    newY = tr.y() + tr.height();
                    newRot = -90;
                }
                QTransform m = QTransform::fromTranslate(newX, newY);
                m.rotate(newRot);
                p->setTransform(m, true);
            }

            QRect iconRect;
            tabLayout(&tabV2, widget, &tr, &iconRect);
            tr = proxy()->subElementRect(SE_TabBarTabText, opt, widget); //we compute tr twice because the style may override subElementRect

            if (!tabV2.icon.isNull()) {
                QPixmap tabIcon = tabV2.icon.pixmap(tabV2.iconSize,
                                                    (tabV2.state & State_Enabled) ? QIcon::Normal
                                                                                  : QIcon::Disabled,
                                                    (tabV2.state & State_Selected) ? QIcon::On
                                                                                   : QIcon::Off);
                p->drawPixmap(iconRect.x(), iconRect.y(), tabIcon);
            }

            proxy()->drawItemText(p, tr, alignment, tab->palette, tab->state & State_Enabled, tab->text, QPalette::WindowText);
            if (verticalTabs)
                p->restore();

            if (tabV2.state & State_HasFocus) {
                const int OFFSET = 1 + pixelMetric(PM_DefaultFrameWidth);

                int x1, x2;
                x1 = tabV2.rect.left();
                x2 = tabV2.rect.right() - 1;

                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*tab);
                fropt.rect.setRect(x1 + 1 + OFFSET, tabV2.rect.y() + OFFSET,
                                   x2 - x1 - 2*OFFSET, tabV2.rect.height() - 2*OFFSET);
                drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
            }
        }

        qDebug() << "draw tabbar label";
        return;
    }

    QProxyStyle::drawControl(element, opt, p, widget);
}

QRect CTabStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
        if (element == SE_TabBarTabRightButton) {
            QRect r = QCommonStyle::subElementRect(element, option, widget);
            qDebug() << "get common style: tab right button " << r;
            r.setLeft(r.left()-10);
            return r;
        } else
        if (element == SE_TabBarTabText) {
            QRect r = QCommonStyle::subElementRect(element, option, widget);
            qDebug() << "get common style: tab text " << r;
//            r.moveLeft(-20);
            return r;
        }

    qDebug() << "subElementRect: " << element;
    return QProxyStyle::subElementRect(element, option, widget);
}

void CTabStyle::tabLayout(const QStyleOptionTabV3 *opt, const QWidget *widget, QRect *textRect, QRect *iconRect) const
{
    Q_ASSERT(textRect);
    Q_ASSERT(iconRect);
    QRect tr = opt->rect;
    bool verticalTabs = opt->shape == QTabBar::RoundedEast
                        || opt->shape == QTabBar::RoundedWest
                        || opt->shape == QTabBar::TriangularEast
                        || opt->shape == QTabBar::TriangularWest;
    if (verticalTabs)
        tr.setRect(0, 0, tr.height(), tr.width()); //0, 0 as we will have a translate transform

    int verticalShift = baseStyle()->proxy()->pixelMetric(QStyle::PM_TabBarTabShiftVertical, opt, widget);
    int horizontalShift = baseStyle()->proxy()->pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, opt, widget);
    int hpadding = baseStyle()->proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, opt, widget) / 2;
    int vpadding = baseStyle()->proxy()->pixelMetric(QStyle::PM_TabBarTabVSpace, opt, widget) / 2;
    if (opt->shape == QTabBar::RoundedSouth || opt->shape == QTabBar::TriangularSouth)
        verticalShift = -verticalShift;
    tr.adjust(hpadding, verticalShift - vpadding, horizontalShift - hpadding, vpadding);
    bool selected = opt->state & QStyle::State_Selected;
    if (selected) {
        tr.setTop(tr.top() - verticalShift);
        tr.setRight(tr.right() - horizontalShift);
    }

    // left widget
    if (!opt->leftButtonSize.isEmpty()) {
        tr.setLeft(tr.left() + 4 +
            (verticalTabs ? opt->leftButtonSize.height() : opt->leftButtonSize.width()));
    }
    // right widget
    if (!opt->rightButtonSize.isEmpty()) {
        tr.setRight(tr.right() - 4 -
            (verticalTabs ? opt->rightButtonSize.height() : opt->rightButtonSize.width()));
    }

    // icon
    if (!opt->icon.isNull()) {
        QSize iconSize = opt->iconSize;
        if (!iconSize.isValid()) {
            int iconExtent = baseStyle()->proxy()->pixelMetric(QStyle::PM_SmallIconSize);
            iconSize = QSize(iconExtent, iconExtent);
        }
        QSize tabIconSize = opt->icon.actualSize(iconSize,
                        (opt->state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                        (opt->state & QStyle::State_Selected) ? QIcon::On : QIcon::Off  );
        // High-dpi icons do not need adjustmet; make sure tabIconSize is not larger than iconSize
        tabIconSize = QSize(qMin(tabIconSize.width(), iconSize.width()), qMin(tabIconSize.height(), iconSize.height()));

        *iconRect = QRect(tr.left(), tr.center().y() - tabIconSize.height() / 2,
                    tabIconSize.width(), tabIconSize .height());
        if (!verticalTabs)
            *iconRect = baseStyle()->proxy()->visualRect(opt->direction, opt->rect, *iconRect);
        tr.setLeft(tr.left() + tabIconSize.width() + 4);
    }

    if (!verticalTabs)
        tr = baseStyle()->proxy()->visualRect(opt->direction, opt->rect, tr);

    *textRect = tr;
}
