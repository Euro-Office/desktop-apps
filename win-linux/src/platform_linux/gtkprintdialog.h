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

#ifndef GTKPRINTDIALOG_H
#define GTKPRINTDIALOG_H

#include <QWidget>
#include <QString>
#include <QPrinter>
#include <QUrl>
#include "components/cprintdialog.h"

typedef QAbstractPrintDialog::PrintDialogOption PrintOption;
typedef QAbstractPrintDialog::PrintDialogOptions PrintOptions;
typedef QAbstractPrintDialog::PrintRange PrintRange;


class GtkPrintDialog
{
public:
    GtkPrintDialog(QPrinter *printer, QWidget *parent);
    ~GtkPrintDialog();

    void setWindowTitle(const QString &title);
    void setEnabledOptions(PrintOptions enbl_opts);
    void setOptions(PrintOptions opts);
    void setPrintRange(PrintRange print_range);
    QDialog::DialogCode exec();
    void accept();
    PrintRange printRange();
    PrintOptions enabledOptions();
    PrintOptions options();
    QVector<PageRanges> getPageRanges();
    int fromPage();
    int toPage();
    void setFromTo(int from, int to);

private:
    QPrinter *m_printer;
    QWidget *m_parent;
    QString m_title;
    PrintOptions m_options;
    PrintRange m_print_range;
    QVector<PageRanges> m_page_ranges;
    int m_pages_count;
};

#endif // GTKPRINTDIALOG_H
