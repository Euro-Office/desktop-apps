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

/*
#include "csplash.h"
#include "csplash_p.h"
#include "defines.h"
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include "utils.h"

CSplash * _splash;

CSplash::CSplash(const QPixmap &p, Qt::WindowFlags f)
    : QSplashScreen(p, f)
{
    _splash = NULL;
}

void CSplash::show(int scrnum)
{
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QApplication::screens().at(scrnum)->availableGeometry()));
    QSplashScreen::show();
}

void CSplash::showSplash()
{
    if ( !_splash ) {
        GET_REGISTRY_USER(reg_user)

        int _scr_num = 0;
        _splash = new CSplash(QPixmap(), Qt::WindowStaysOnTopHint);

        if (QApplication::screens().count() > 1) {
            QScreen * _screen = Utils::screenAt(reg_user.value("position").toRect().topLeft());

            if ( _screen ) {
                _splash->move(_screen->geometry().center());
                _scr_num = QApplication::screens().indexOf(_screen);
            }
        }

        double _dpi_ratio = Utils::getScreenDpiRatioByHWND(_splash->winId());

//        _splash->setPixmap(_dpi_ratio > 1 ? QPixmap(":/res/icons/splash_2x.png") : QPixmap(":/res/icons/splash.png"));
        _splash->setPixmap(getSplashImage(_dpi_ratio));
        _splash->show(_scr_num);
    }
}

void CSplash::hideSplash()
{
    if (_splash) {
//        g_splash->setParent((QWidget *)parent());
        _splash->close();

        delete _splash, _splash = NULL;
    }
}

double CSplash::startupDpiRatio()
{
    if (_splash) {
        return Utils::getScreenDpiRatioByHWND(_splash->winId());
    } else {
        QSplashScreen splash;

        if (QApplication::screens().count() > 1) {
            GET_REGISTRY_USER(reg_user)

            QScreen * _screen = Utils::screenAt(reg_user.value("position").toRect().topLeft());
            if ( _screen ) {
                splash.move(_screen->geometry().center());
            }
        }

        return Utils::getScreenDpiRatioByHWND(splash.winId());
    }
}
*/
