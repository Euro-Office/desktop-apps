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

#ifndef CPRESENTERWINDOW_H
#define CPRESENTERWINDOW_H

#ifdef _WIN32
# include "windows/platform_win/cwindowplatform.h"
#else
# include "windows/platform_linux/cwindowplatform.h"
#endif
#include "qcefview.h"


class CPresenterWindow : public CWindowPlatform
{
public:
    explicit CPresenterWindow(const QRect&, const QString&, QCefView*);
    virtual ~CPresenterWindow();

    virtual void applyTheme(const std::wstring&) final;
    virtual bool holdView(int id) const final;

protected:
    void closeEvent(QCloseEvent *) final;
    virtual void onLayoutDirectionChanged() final;

private:
    QWidget * createMainPanel(QWidget *, const QString&, QWidget * view = nullptr);
    virtual void setScreenScalingFactor(double, bool resize = true) final;
    virtual void onCloseEvent() final;
    virtual void focus() final;
};

#endif // CPRESENTERWINDOW_H
