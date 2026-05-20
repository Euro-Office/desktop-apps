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

#include "components/celipsislabel.h"


auto ellipsis_text_(const QWidget * widget, const QString& str, Qt::TextElideMode mode = Qt::ElideRight) -> QString
{
    QMargins _margins = widget->contentsMargins();
    int _padding = _margins.left() + _margins.right();
    int _width = widget->maximumWidth() != QWIDGETSIZE_MAX ? widget->maximumWidth() : widget->width();
    QFontMetrics _metrics(widget->font());

    return _metrics.elidedText(str, mode, _width - _padding - 1);
}

CElipsisLabel::CElipsisLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    setTextFormat(Qt::PlainText);
}

CElipsisLabel::CElipsisLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
    , orig_text(text)
{
    setTextFormat(Qt::PlainText);
//    QString elt = elipsis_text(this, text, Qt::ElideMiddle);
//    setText(elt);
}

void CElipsisLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);

    if ( event->size().width() != event->oldSize().width() ) {
        QString elt = ellipsis_text_(this, orig_text, elide_mode);
        QLabel::setText(elt);
        QFontMetrics fm(font());
#if (QT_VERSION < QT_VERSION_CHECK(5,11,0))
        int textWidth = fm.width(elt);
#else
        int textWidth = fm.horizontalAdvance(elt);
#endif
        emit onResize(event->size(), textWidth);
    }
}

auto CElipsisLabel::setText(const QString& text) -> void
{
    orig_text = text;

    QString elt = ellipsis_text_(this, text, elide_mode);
    QLabel::setText(elt);
}

auto CElipsisLabel::setEllipsisMode(Qt::TextElideMode mode) -> void
{
    elide_mode = mode;
}

auto CElipsisLabel::textWidth() -> int
{
    QString elt = ellipsis_text_(this, orig_text, elide_mode);
    QFontMetrics fm(font());
#if (QT_VERSION < QT_VERSION_CHECK(5,11,0))
    int textWidth = fm.width(elt);
#else
    int textWidth = fm.horizontalAdvance(elt);
#endif
    return textWidth;
}

auto CElipsisLabel::updateText() -> void
{
    QString elt = ellipsis_text_(this, orig_text, elide_mode);
    if ( elt != text() ) {
        QLabel::setText(elt);
    }
}
