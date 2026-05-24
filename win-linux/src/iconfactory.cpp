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

#include "iconfactory.h"
#include "cascapplicationmanagerwrapper.h"
#include <QSvgRenderer>
#include <QPainter>

#define IMAGES 6
#define DEFAULT_SIZE 20
#define ADVANCED_OPACITY 0.8


QIcon IconFactory::icon(IconIndex icon, int pixelSize)
{
    const char* iconPath[IMAGES] = {
        ":/menu/icons/iconssmall_1x.png",
        ":/menu/icons/iconssmall_1.25x.png",
        ":/menu/icons/iconssmall_1.5x.png",
        ":/menu/icons/iconssmall_1.75x.png",
        ":/menu/icons/iconssmall_2x.png",
        ":/menu/icons/iconssmall_2.5x.svg"
    };

    int index = pixelSize == DEFAULT_SIZE ? 0 :
                pixelSize == 1.25 * DEFAULT_SIZE ? 1 :
                pixelSize == 1.5 * DEFAULT_SIZE ? 2 :
                pixelSize == 1.75 * DEFAULT_SIZE ? 3 :
                pixelSize == 2 * DEFAULT_SIZE ? 4 : IMAGES - 1;

    QPixmap pix(pixelSize, pixelSize);
    pix.fill(Qt::transparent);
    if (index < IMAGES - 1) {
        QPixmap image(iconPath[index]);
        // int iconsPerRow = pixmap.width() / pixelSize;
        // int iconsPerColumn = pixmap.height() / pixelSize;
        int x = AscAppManager::themes().current().isDark() ? pixelSize : 0;
        int y = static_cast<int>(icon) * pixelSize;
        QPainter p(&pix);
        p.setOpacity(ADVANCED_OPACITY);
        p.drawPixmap(0, 0, image.copy(x, y, pixelSize, pixelSize));
        p.end();

    } else {
        const QString node = icon == CreateNew ? "btn-add-text" :
                             icon == Browse ?    "btn-browse" :
                             icon == Pin ?       "btn-pin" :
                             icon == Unpin ?     "btn-unpin" : "";

        QString path(iconPath[index]);
        QSvgRenderer svg(path);
        QPainter p(&pix);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        QRectF bounds = svg.boundsOnElement(node);
        double kx = pixelSize / svg.viewBoxF().width();
        double ky = pixelSize / svg.viewBoxF().height();
        bounds = QRectF(bounds.x() * kx, bounds.y() * ky, bounds.width() * kx, bounds.height() * ky);
        svg.render(&p, node, bounds);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), AscAppManager::themes().current().isDark() ? QColor(255, 255, 255, 200) : QColor(0, 0, 0, 200));
        p.end();
    }
    return QIcon(pix);
}
