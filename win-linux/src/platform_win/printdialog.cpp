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

#include <windows.h>
#include <commdlg.h>
#include <comdef.h>
#include <private/qprintengine_win_p.h>
#include "printdialog.h"
#include "utils.h"
#include "components/cmessage.h"

#define MAXPAGERANGES 32
#define PRINT_DIALOG_REG_KEY L"Software\\Microsoft\\Print\\UnifiedPrintDialog\0"
#define PRINT_DIALOG_REG_VALUE L"PreferLegacyPrintDialog\0"

typedef QPageSize::PageSizeId PageSize;


auto getPaperSizeFromPageSize(LPWSTR pPrinterName, const QPageSize &ps)->int
{
    switch (ps.id()) {
//    case PageSize::A0:
//        return DMPAPER_USER;
//    case PageSize::A1:
//        return DMPAPER_USER;
    case PageSize::A2:
        return DMPAPER_A2;
    case PageSize::A3:
        return DMPAPER_A3;
    case PageSize::A4:
        return DMPAPER_A4;
    case PageSize::A5:
        return DMPAPER_A5;
    case PageSize::A6:
        return DMPAPER_A6;
    case PageSize::B5:
        return DMPAPER_B5;
    case PageSize::Tabloid:
        return DMPAPER_TABLOID;
    case PageSize::EnvelopeDL:
        return DMPAPER_ENV_DL;
    case PageSize::Comm10E:
        return DMPAPER_ENV_10;
    case PageSize::SuperB:
        return DMPAPER_B_PLUS;
    case PageSize::TabloidExtra:
        return DMPAPER_TABLOID_EXTRA;
    case PageSize::Letter:
        return DMPAPER_LETTER;
    case PageSize::Legal:
        return DMPAPER_LEGAL;
    case PageSize::EnvelopeChou3:
        return DMPAPER_JENV_CHOU3;
    default: {
        bool paperNamesSuccess = false, paperSizeSuccess = false;
        std::vector<WORD> papers;
        std::vector<POINT> paperSize;
        int papersCount = DeviceCapabilities(pPrinterName, NULL, DC_PAPERS, NULL, NULL);
        if (papersCount > 0) {
            papers.assign(papersCount, 0);
            int res = DeviceCapabilities(pPrinterName, NULL, DC_PAPERS, (LPWSTR)papers.data(), NULL);
            if (res == papersCount)
                paperNamesSuccess = true;
        }
        int paperSizeCount = DeviceCapabilities(pPrinterName, NULL, DC_PAPERSIZE, NULL, NULL);
        if (paperSizeCount > 0) {
            paperSize.assign(paperSizeCount, {0, 0});
            int res = DeviceCapabilities(pPrinterName, NULL, DC_PAPERSIZE, (LPWSTR)paperSize.data(), NULL);
            if (res == paperSizeCount)
                paperSizeSuccess = true;
        }
        if (paperNamesSuccess && paperSizeSuccess && papersCount == paperSizeCount) {
            constexpr double diff = 1.0;
            QSizeF size = ps.size(QPageSize::Millimeter);
            for (int j = 0; j < papersCount; ++j) {
                double width = (double)paperSize[j].x/10;
                double height = (double)paperSize[j].y/10;
                if (std::abs(size.width() - width) < diff && std::abs(size.height() - height) < diff)
                    return papers[j];
            }
        }
        return DMPAPER_USER;
    }
    }
}

#ifndef __OS_WIN_XP
auto resetLegacyPrintDialog()->void
{
    HKEY hKey = NULL;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, PRINT_DIALOG_REG_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        DWORD pvData = 0, pcbData = sizeof(DWORD);
        if (RegGetValue(hKey, NULL, PRINT_DIALOG_REG_VALUE, RRF_RT_REG_DWORD, NULL, &pvData, &pcbData) == ERROR_SUCCESS) {
            if (pvData == 1) {
                pvData = 0;
                RegSetValueEx(hKey, PRINT_DIALOG_REG_VALUE, 0, REG_DWORD, (const BYTE*)&pvData, sizeof(DWORD));
            }
        }
        RegCloseKey(hKey);
    }
}
#endif

struct PrintDialogCallback : public IPrintDialogCallback
{
public:
    PrintDialogCallback(bool *dialog_was_changed) :
        m_dialog_was_changed(dialog_was_changed)
    {}
private:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) noexcept final {
        if (riid == IID_IUnknown || riid == IID_IPrintDialogCallback) {
            *ppv = static_cast<IPrintDialogCallback*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef() noexcept final {
        return 1;
    }
    virtual ULONG STDMETHODCALLTYPE Release() noexcept final {
        return 1;
    }
    virtual HRESULT STDMETHODCALLTYPE HandleMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                                       LRESULT *pResult) noexcept final {
        return S_FALSE;
    }
    virtual HRESULT STDMETHODCALLTYPE InitDone() noexcept final {
#ifndef __OS_WIN_XP
        if (m_dialog_was_changed && *m_dialog_was_changed)   // Restore print dialog type
            resetLegacyPrintDialog();
#endif
        return S_FALSE;
    }
    virtual HRESULT STDMETHODCALLTYPE SelectionChange() noexcept final {
        return S_FALSE;
    }

    bool *m_dialog_was_changed = nullptr;
};

PrintDialog::PrintDialog(QPrinter *printer, QWidget *parent) :
    m_printer(printer),
    m_parent(parent),
    m_title(QString()),
    m_options(PrintOptions())
{
    m_print_range = (PrintRange)printer->printRange();
    if (m_printer->collateCopies())
        m_options |= PrintOption::PrintCollateCopies;
    m_page_ranges.append(PageRanges(m_printer->fromPage(), m_printer->toPage()));
    m_pages_count = m_printer->toPage();
}

PrintDialog::~PrintDialog()
{

}

void PrintDialog::setWindowTitle(const QString &title)
{
    m_title = title;
}

void PrintDialog::setEnabledOptions(PrintOptions enbl_opts)
{
    m_options = enbl_opts;
}

void PrintDialog::setOptions(PrintOptions opts)
{
    m_options = opts;
}

void PrintDialog::setPrintRange(PrintRange print_range)
{
    m_print_range = print_range;
}

QDialog::DialogCode PrintDialog::exec()
{
    HWND parent_hwnd = (m_parent) ? (HWND)m_parent->winId() : NULL;
    auto qt_printer_name = m_printer->printerName().toStdWString();
    auto qt_resolution = m_printer->resolution();
    auto qt_orient = m_printer->pageLayout().orientation();
    auto qt_duplex = m_printer->duplex();
    auto qt_color_mode = m_printer->colorMode();
    auto qt_copy_count = m_printer->copyCount();
//    auto qt_page_order = m_printer->pageOrder();
//    auto qt_output_filename = m_printer->outputFileName().toStdWString();
//    auto qt_doc_name = m_printer->docName();
//    auto qt_full_page = m_printer->fullPage();
//    auto qt_color_count = m_printer->colorCount();
//    auto qt_supported_res = m_printer->supportedResolutions();
//    auto qt_supports_multi_copies = m_printer->supportsMultipleCopies();
//    auto qt_selection_option = m_printer->printerSelectionOption();
//    auto qt_output_format = m_printer->outputFormat();
    auto qt_paper_source = m_printer->paperSource();

    // Qt-PrintOptions:
    // None = 0
    // PrintToFile = 1
    // PrintSelection = 2
    // PrintPageRange = 4
    // PrintShowPageSize = 8    - not applied
    // PrintCollateCopies = 16
    // DontUseSheet = 32        - not applied
    // PrintCurrentPage = 64
    DWORD flags = (
        PD_ALLPAGES
//        PD_COLLATE |
//        PD_ENABLEPRINTTEMPLATE |
//        PD_ENABLEPRINTTEMPLATEHANDLE |
//        PD_EXCLUSIONFLAGS |
//        PD_HIDEPRINTTOFILE |
//        PD_NOWARNING |
//        PD_PRINTTOFILE |
//        PD_RETURNDC |
//        PD_RETURNDEFAULT |
//        PD_RETURNIC |
//        PD_USELARGETEMPLATE
    );

    if (!m_options.testFlag(PrintOption::PrintToFile))
        flags |= PD_DISABLEPRINTTOFILE;

    if (!m_options.testFlag(PrintOption::PrintSelection))
        flags |= PD_NOSELECTION;

    if (!m_options.testFlag(PrintOption::PrintPageRange))
        flags |= PD_NOPAGENUMS;

    if (!m_options.testFlag(PrintOption::PrintCollateCopies))
        flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!m_options.testFlag(PrintOption::PrintCurrentPage))
        flags |= PD_NOCURRENTPAGE;

    // Qt-PrintRange:
    // AllPages = 0
    // Selection = 1
    // PageRange = 2
    // CurrentPage = 3
    if (m_print_range == PrintRange::Selection)
        flags |= PD_SELECTION;
    else
    if (m_print_range == PrintRange::PageRange)
        flags |= PD_PAGENUMS;
    else
    if (m_print_range == PrintRange::CurrentPage)
        flags |= PD_CURRENTPAGE;

    PRINTPAGERANGE *page_ranges = (PRINTPAGERANGE*)GlobalAlloc(GPTR, MAXPAGERANGES * sizeof(PRINTPAGERANGE));
    Q_ASSERT(page_ranges != nullptr);
    page_ranges[0] = {m_page_ranges.isEmpty() ? 1 : (DWORD)m_page_ranges[0].fromPage,
                      m_page_ranges.isEmpty() ? m_pages_count : (DWORD)m_page_ranges[0].toPage};

    // Input settings
    HGLOBAL hDevMode = NULL;
    HGLOBAL hDevNames = NULL;
    {
        HANDLE hPrinter = NULL;
        LPWSTR pPrinterName = &qt_printer_name[0];
        if (OpenPrinter(pPrinterName, &hPrinter, NULL)) {
            DWORD dwNeeded = 0, dwRet = 0;
            GetPrinter(hPrinter, 2, NULL, 0, &dwNeeded);
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                PRINTER_INFO_2 *prntInfo = (PRINTER_INFO_2*)GlobalAlloc(GPTR, dwNeeded);
                if (GetPrinter(hPrinter, 2, (LPBYTE)prntInfo, dwNeeded, &dwRet)) {
                    dwNeeded = sizeof(DEVNAMES) + 3 * MAX_PATH * sizeof(WCHAR);
                    hDevNames = GlobalAlloc(GHND, dwNeeded);
                    Q_ASSERT(hDevNames);
                    LPDEVNAMES pDevNames = (LPDEVNAMES)GlobalLock(hDevNames);
                    Q_ASSERT(pDevNames);
                    pDevNames->wDriverOffset = sizeof(DEVNAMES)/sizeof(WCHAR);
                    pDevNames->wDeviceOffset = pDevNames->wDriverOffset + MAX_PATH;
                    pDevNames->wOutputOffset = pDevNames->wDeviceOffset + MAX_PATH;
                    pDevNames->wDefault = 0;
                    wcsncpy_s((LPWSTR)pDevNames + pDevNames->wDriverOffset, MAX_PATH, prntInfo->pDriverName, _TRUNCATE);
                    wcsncpy_s((LPWSTR)pDevNames + pDevNames->wDeviceOffset, MAX_PATH, prntInfo->pPrinterName, _TRUNCATE);
                    wcsncpy_s((LPWSTR)pDevNames + pDevNames->wOutputOffset, MAX_PATH, prntInfo->pPortName, _TRUNCATE);
                    GlobalUnlock(hDevNames);
                }
                GlobalFree(prntInfo);
            }

            dwNeeded = DocumentProperties(parent_hwnd, hPrinter, pPrinterName, NULL, NULL, 0);
            Q_ASSERT(dwNeeded >= sizeof(DEVMODE));
            hDevMode = GlobalAlloc(GHND, dwNeeded);
            Q_ASSERT(hDevMode);
            LPDEVMODE pDevMode = (LPDEVMODE)GlobalLock(hDevMode);
            Q_ASSERT(pDevMode);
            dwRet = DocumentProperties(parent_hwnd, hPrinter, pPrinterName, pDevMode, NULL, DM_OUT_BUFFER);
            if (dwRet == IDOK) {
                if (pDevMode->dmFields & DM_YRESOLUTION)
                    pDevMode->dmYResolution = qt_resolution;

                if (pDevMode->dmFields & DM_COLOR)
                    pDevMode->dmColor = (qt_color_mode == QPrinter::Color) ? DMCOLOR_COLOR : DMCOLOR_MONOCHROME;

                if (pDevMode->dmFields & DM_COLLATE)
                    pDevMode->dmCollate = m_printer->collateCopies() ? DMCOLLATE_TRUE : DMCOLLATE_FALSE;

                if (pDevMode->dmFields & DM_DEFAULTSOURCE)
                    pDevMode->dmDefaultSource = (qt_paper_source == QPrinter::Auto) ? DMBIN_AUTO :
                                                (qt_paper_source == QPrinter::Cassette) ? DMBIN_CASSETTE :
                                                (qt_paper_source == QPrinter::Envelope) ? DMBIN_ENVELOPE :
                                                (qt_paper_source == QPrinter::EnvelopeManual) ? DMBIN_ENVMANUAL :
                                                (qt_paper_source == QPrinter::FormSource) ? DMBIN_FORMSOURCE :
                                                (qt_paper_source == QPrinter::LargeCapacity) ? DMBIN_LARGECAPACITY :
                                                (qt_paper_source == QPrinter::LargeFormat) ? DMBIN_LARGEFMT :
                                                (qt_paper_source == QPrinter::LastPaperSource) ? DMBIN_LAST :
                                                (qt_paper_source == QPrinter::Lower) ? DMBIN_LOWER :
                                                (qt_paper_source == QPrinter::Manual) ? DMBIN_MANUAL :
                                                (qt_paper_source == QPrinter::Middle) ? DMBIN_MIDDLE :
                                                (qt_paper_source == QPrinter::OnlyOne) ? DMBIN_ONLYONE :
                                                (qt_paper_source == QPrinter::SmallFormat) ? DMBIN_SMALLFMT :
                                                (qt_paper_source == QPrinter::Tractor) ? DMBIN_TRACTOR :
                                                (qt_paper_source == QPrinter::Upper) ? DMBIN_UPPER : DMBIN_USER;

                if (pDevMode->dmFields & DM_COPIES)
                    pDevMode->dmCopies = qt_copy_count;

                if (pDevMode->dmFields & DM_ORIENTATION)
                    pDevMode->dmOrientation = (qt_orient == QPageLayout::Portrait) ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;

                // Qt-Duplex:
                // DuplexNone = 0
                // DuplexAuto = 1       - not applied
                // DuplexLongSide = 2
                // DuplexShortSide = 3
                if (pDevMode->dmFields & DM_DUPLEX) {
                    pDevMode->dmDuplex = (qt_duplex == QPrinter::DuplexLongSide) ? DMDUP_VERTICAL :
                                         (qt_duplex == QPrinter::DuplexShortSide) ? DMDUP_HORIZONTAL : DMDUP_SIMPLEX;
                }

                int paper_size = getPaperSizeFromPageSize(pPrinterName, m_printer->pageLayout().pageSize());
                if (pDevMode->dmFields & DM_PAPERSIZE)
                    pDevMode->dmPaperSize = paper_size;

                if (paper_size == DMPAPER_USER || !(pDevMode->dmFields & DM_PAPERSIZE)) {
                    QPageSize ps = m_printer->pageLayout().pageSize();
                    QSizeF page_size = ps.size(QPageSize::Millimeter);
                    if (pDevMode->dmFields & DM_PAPERWIDTH)
                        pDevMode->dmPaperWidth = qRound(10 * page_size.width());
                    if (pDevMode->dmFields & DM_PAPERLENGTH)
                        pDevMode->dmPaperLength = qRound(10 * page_size.height());
                }

                dwRet = DocumentProperties(parent_hwnd, hPrinter, pPrinterName, pDevMode, pDevMode, DM_IN_BUFFER | DM_OUT_BUFFER);
            }
            GlobalUnlock(hDevMode);
            if (dwRet != IDOK) {
                GlobalFree(hDevMode);
                hDevMode = NULL;
            }
            ClosePrinter(hPrinter);
        }
    }

    // Switch to legacy print dialog
    bool dialog_was_changed = false;
#ifndef __OS_WIN_XP
    if (Utils::getWinVersion() >= Utils::WinVer::Win11) {
        HKEY hKey = NULL;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, PRINT_DIALOG_REG_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
            DWORD pvData = 0, pcbData = sizeof(DWORD);
            LSTATUS status = RegGetValue(hKey, NULL, PRINT_DIALOG_REG_VALUE, RRF_RT_REG_DWORD, NULL, &pvData, &pcbData);
            if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
                if (pvData == 0) {
                    pvData = 1;
                    if (RegSetValueEx(hKey, PRINT_DIALOG_REG_VALUE, 0, REG_DWORD, (const BYTE*)&pvData, sizeof(DWORD)) == ERROR_SUCCESS)
                        dialog_was_changed = true;
                }
            }
            RegCloseKey(hKey);
        }
    }
#endif

    // Init dialog
    PRINTDLGEX dlg;
    ZeroMemory(&dlg, sizeof(dlg));
    dlg.lStructSize    = sizeof(dlg);
    dlg.Flags          = flags;
//    dlg.ExclusionFlags = PD_EXCL_COPIESANDCOLLATE;
    dlg.hwndOwner      = parent_hwnd;
    dlg.hInstance      = NULL;
    dlg.hDevNames      = hDevNames;
    dlg.hDevMode       = hDevMode;
    dlg.nStartPage     = START_PAGE_GENERAL;
    dlg.nCopies        = qt_copy_count;
    dlg.nMaxPageRanges = MAXPAGERANGES;
    dlg.nPageRanges    = 1;
    dlg.lpPageRanges   = page_ranges;
    dlg.nMinPage       = 1;
    dlg.nMaxPage       = (DWORD)m_pages_count;
    PrintDialogCallback clb(&dialog_was_changed);
    dlg.lpCallback     = static_cast<IPrintDialogCallback*>(&clb);

    WindowHelper::toggleLayoutDirection(parent_hwnd);
    QDialog::DialogCode exit_code = QDialog::DialogCode::Rejected;
    HRESULT hr = PrintDlgEx(&dlg);
    if (hr == S_OK) {
        switch (dlg.dwResultAction) {
        case PD_RESULT_PRINT: {
            if (dlg.hDevNames) {
                LPDEVNAMES pDevnames = (LPDEVNAMES)GlobalLock(dlg.hDevNames);
                LPWSTR lpDeviceName = (LPWSTR)pDevnames + pDevnames->wDeviceOffset;
                m_printer->setPrinterName(QString::fromWCharArray(lpDeviceName));
                GlobalUnlock(dlg.hDevNames);
            }
            if (dlg.hDevMode) {
                LPDEVMODE pDevmode = (LPDEVMODE)GlobalLock(dlg.hDevMode);
                m_printer->setColorMode(pDevmode->dmColor == DMCOLOR_COLOR ? QPrinter::Color : QPrinter::GrayScale);
                m_printer->setPaperSource(pDevmode->dmDefaultSource == DMBIN_AUTO ? QPrinter::Auto :
                                          pDevmode->dmDefaultSource == DMBIN_CASSETTE ? QPrinter::Cassette :
                                          pDevmode->dmDefaultSource == DMBIN_ENVELOPE ? QPrinter::Envelope :
                                          pDevmode->dmDefaultSource == DMBIN_ENVMANUAL ? QPrinter::EnvelopeManual :
                                          pDevmode->dmDefaultSource == DMBIN_FORMSOURCE ? QPrinter::FormSource :
                                          pDevmode->dmDefaultSource == DMBIN_LARGECAPACITY ? QPrinter::LargeCapacity :
                                          pDevmode->dmDefaultSource == DMBIN_LARGEFMT ? QPrinter::LargeFormat :
                                          pDevmode->dmDefaultSource == DMBIN_LAST ? QPrinter::LastPaperSource :
                                          pDevmode->dmDefaultSource == DMBIN_LOWER ? QPrinter::Lower :
                                          pDevmode->dmDefaultSource == DMBIN_MANUAL ? QPrinter::Manual :
                                          pDevmode->dmDefaultSource == DMBIN_MIDDLE ? QPrinter::Middle :
                                          pDevmode->dmDefaultSource == DMBIN_ONLYONE ? QPrinter::OnlyOne :
                                          pDevmode->dmDefaultSource == DMBIN_SMALLFMT ? QPrinter::SmallFormat :
                                          pDevmode->dmDefaultSource == DMBIN_TRACTOR ? QPrinter::Tractor :
                                          pDevmode->dmDefaultSource == DMBIN_UPPER ? QPrinter::Upper : QPrinter::CustomSource);
                m_print_range = (dlg.Flags & PD_SELECTION) ? PrintRange::Selection :
                                    (dlg.Flags & PD_PAGENUMS) ? PrintRange::PageRange :
                                    (dlg.Flags & PD_CURRENTPAGE) ? PrintRange::CurrentPage : PrintRange::AllPages;

                m_page_ranges.clear();
                for (DWORD i = 0; i < dlg.nPageRanges; i++) {
                    int start = dlg.lpPageRanges[i].nFromPage;
                    int end = dlg.lpPageRanges[i].nToPage;
                    if (start > end)
                        for (int j = start; j >= end; j--)
                            m_page_ranges.append(PageRanges(j, j));
                    else
                        m_page_ranges.append(PageRanges(start, end));

                    if (i == 0)
                        m_printer->setFromTo(start > end ? end : start, start > end ? start : end);
                }
                m_printer->setCollateCopies(pDevmode->dmCollate == DMCOLLATE_TRUE);
                m_printer->setDuplex(pDevmode->dmDuplex == DMDUP_VERTICAL ? QPrinter::DuplexLongSide :
                                         pDevmode->dmDuplex == DMDUP_HORIZONTAL ? QPrinter::DuplexShortSide : QPrinter::DuplexNone);

                m_printer->setCopyCount(pDevmode->dmCopies);
//                auto path = QUrl::fromPercentEncoding(QByteArray(output_uri)).replace("file://", "");
//                m_printer->setOutputFileName(path);
                double width = double(pDevmode->dmPaperWidth)/10;
                double height = double(pDevmode->dmPaperLength)/10;
                QPageSize ps(QSizeF(width, height), QPageSize::Millimeter);
                m_printer->setPageSize(ps);
                QPageLayout plt = m_printer->pageLayout();
                plt.setOrientation(pDevmode->dmOrientation == DMORIENT_PORTRAIT ? QPageLayout::Portrait : QPageLayout::Landscape);
                m_printer->setPageLayout(plt);
                GlobalUnlock(dlg.hDevMode);
            }

            if (QWin32PrintEngine *wpe = dynamic_cast<QWin32PrintEngine*>(m_printer->printEngine()))
                wpe->setGlobalDevMode(dlg.hDevNames, dlg.hDevMode);

            exit_code = QDialog::DialogCode::Accepted;
            break;
        }
        default:
            break;
        }

        if (dlg.hDevMode)
            GlobalFree(dlg.hDevMode);
        if (dlg.hDevNames)
            GlobalFree(dlg.hDevNames);
        //    if (dlg.hDC)
        //        DeleteDC(dlg.hDC);
    } else {
#ifndef __OS_WIN_XP
        if (dialog_was_changed)   // Restore print dialog type
            resetLegacyPrintDialog();
#endif
        if (hDevMode)
            GlobalFree(hDevMode);
        if (hDevNames)
            GlobalFree(hDevNames);

        const wchar_t *err = _com_error(hr).ErrorMessage();
        CMessage::error(m_parent, QObject::tr("Unable to open print dialog:<br>%1").arg(QString::fromStdWString(err)));
    }
    GlobalFree(page_ranges);

    WindowHelper::toggleLayoutDirection(parent_hwnd);
    return exit_code;
}

PrintRange PrintDialog::printRange()
{
    return m_print_range;
}

PrintOptions PrintDialog::enabledOptions()
{
    return m_options;
}

PrintOptions PrintDialog::options()
{
    return m_options;
}

QVector<PageRanges> PrintDialog::getPageRanges()
{
    return m_page_ranges;
}

int PrintDialog::fromPage()
{
    return m_printer->fromPage();
}

int PrintDialog::toPage()
{
    return m_printer->toPage();
}

void PrintDialog::setFromTo(int from, int to)
{
    from < 1 && (from = 1); to < 1 && (to = 1);
    if (m_pages_count < from || m_pages_count < to) {
        CMessage::warning(m_parent, QObject::tr("Specified range %1-%2 exceeds document limits: maximum number of pages is %3")
                                        .arg(QString::number(from), QString::number(to), QString::number(m_pages_count)));
    }
    from > m_pages_count && (from = m_pages_count);
    to > m_pages_count && (to = m_pages_count);
    m_printer->setFromTo(from > to ? to : from, from > to ? from : to);
    if (!m_page_ranges.isEmpty())
        m_page_ranges.clear();
    m_page_ranges.append(PageRanges(from, to));
}

void PrintDialog::accept()
{

}
