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

#ifndef CWINDOWPLATFORM_H
#define CWINDOWPLATFORM_H

#include "windows/cwindowbase.h"
#include "cx11decoration.h"


class CWindowPlatform : public CWindowBase, public CX11Decoration
{
public:
    explicit CWindowPlatform(const QRect&);
    virtual ~CWindowPlatform();

    void bringToTop();
    void show(bool);
    virtual void setWindowColors(const QColor&, const QColor& border = QColor(), bool isActive = false) final;
    virtual void adjustGeometry() final;

protected:
    void onWindowActivate(bool is_active);
    virtual void onMinimizeEvent() override;
    virtual bool event(QEvent *event) override;
    virtual bool nativeEvent(const QByteArray&, void*, long*) final;
    virtual void setScreenScalingFactor(double, bool resize = true) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void onLayoutDirectionChanged() = 0;

private:
    virtual void mouseMoveEvent(QMouseEvent *) final;
    virtual void mousePressEvent(QMouseEvent *) final;
    virtual void mouseReleaseEvent(QMouseEvent *) final;
    virtual void mouseDoubleClickEvent(QMouseEvent *) final;
    QTimer *m_propertyTimer;
};

#endif // CWINDOWPLATFORM_H
