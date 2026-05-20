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

#ifndef CPRINTDATA_H
#define CPRINTDATA_H

#include "applicationmanager_events.h"
#include "components/cprintdialog.h"
#include <QPrinterInfo>
#include <QPrintDialog>
#include <functional>

typedef std::function<void(const QString&)> FnVoidStr;


class CPrintData
{
public:
    explicit CPrintData();
    ~CPrintData();

    auto init(NSEditorApi::CAscPrintEnd *) -> void;
    auto init(int, NSEditorApi::CAscPrintEnd *) -> void;
    auto printerInfo() const -> QPrinterInfo;
    auto setAppDataPath(const std::wstring&) -> void;
    auto setPrinterInfo(const QPrinterInfo&) -> void;
    auto setPrinterInfo(const QPrinter&) -> void;
    auto pageSize() const -> QPageSize;
    auto pageOrientation() const -> QPageLayout::Orientation;
    auto pagesCount() const -> int;
    auto pageCurrent() const -> int;
    auto pageFrom() const -> int;
    auto pageTo() const -> int;
    auto pageRanges() const -> QVector<PageRanges>;
    auto printRange() const -> QPrintDialog::PrintRange;
    auto isQuickPrint() const -> bool;
    auto useSystemDialog() const -> bool;
    auto colorMode() const -> bool;
    auto viewId() const -> int;
    auto copiesCount() const -> int;
    auto duplexMode() const -> QPrinter::DuplexMode;
    auto printerCapabilitiesReady() const -> bool;
    auto getPrinterCapabilitiesJson() const -> QString;
    auto queryPrinterCapabilitiesAsync(const FnVoidStr &callback) const -> void;

private:
    class CPrintDataPrivate;
    CPrintDataPrivate * m_priv;
};

#endif // CPRINTDATA_H
