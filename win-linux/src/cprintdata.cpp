/*
 * (c) Copyright Ascensio System SIA 2010-2022
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

#include "cprintdata.h"
#include "utils.h"
#include "defines.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <thread>
#include <cmath>
#ifdef __linux__
# include <cups/cups.h>
# include <cups/ppd.h>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
# define SKIP_EMPTY Qt::SkipEmptyParts
#else
# define SKIP_EMPTY QString::SkipEmptyParts
#endif

#ifdef _WIN32
static QString getDriverName(LPWSTR printerName)
{
    QString name;
    HANDLE hPrinter = nullptr;
    if (!OpenPrinter(printerName, &hPrinter, nullptr)) {
        return name;
    }
    DWORD needed = 0;
    GetPrinterDriver(hPrinter, nullptr, 1, nullptr, 0, &needed);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        ClosePrinter(hPrinter);
        return name;
    }
    std::vector<BYTE> buf(needed);
    if (!GetPrinterDriver(hPrinter, nullptr, 1, buf.data(), needed, &needed)) {
        ClosePrinter(hPrinter);
        return name;
    }
    DRIVER_INFO_1 *info = reinterpret_cast<DRIVER_INFO_1*>(buf.data());
    if (info->pName)
        name =  QString::fromWCharArray(info->pName);
    ClosePrinter(hPrinter);
    return name;
}
#endif

static bool jsonArrayContainsDriverName(const QJsonArray &array, const QString &name, QJsonObject &printerObject)
{
    if (name.isEmpty())
        return false;
    for (const QJsonValue &value : array) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            if (obj.contains("driver") && obj.value("driver").toString() == name) {
                printerObject = obj;
                return true;
            }
        }
    }
    return false;
}

static QString getFirstPrinterName(const QJsonObject &json)
{
    if (json.contains("printers")) {
        QJsonArray jarr = json["printers"].toArray();
        if (!jarr.isEmpty()) {
            QJsonObject jobj = jarr.at(0).toObject();
            return jobj["name"].toString();
        }
    }
    return QString();
}

static QVector<PageRanges> parsePageRanges(const QString& input, int pages_count)
{
    QVector<PageRanges> result;
    if (pages_count <= 0 || input.trimmed().isEmpty())
        return result;

    const QStringList parts = input.split(',', SKIP_EMPTY);
    for (const QString& part : parts) {
        QString token = part.trimmed();
        if (token.isEmpty())
            continue;

        int firstDash = token.indexOf('-');
        int lastDash = token.lastIndexOf('-');
        if (firstDash == -1) {
            // Single page
            bool ok;
            int page = token.toInt(&ok);
            if (ok && page >= 1 && page <= pages_count) {
                result.append(PageRanges(page, page));
            }
        } else
        if (firstDash == lastDash) {
            // Exactly one hyphen
            QString left = token.left(firstDash).trimmed();
            QString right = token.mid(firstDash + 1).trimmed();
            if (!left.isEmpty() && !right.isEmpty()) {
                bool ok1, ok2;
                int fromPage = left.toInt(&ok1);
                int toPage = right.toInt(&ok2);
                if (ok1 && ok2) {
                    if (fromPage > toPage)
                        std::swap(fromPage, toPage);
                    if (fromPage > pages_count || toPage < 1)
                        continue;

                    fromPage = std::max<int>(fromPage, 1);
                    toPage = std::min<int>(toPage, pages_count);
                    if (fromPage <= toPage)
                        result.append(PageRanges(fromPage, toPage));
                }
            }
        }
    }
    return result;
}

class CPrintData::CPrintDataPrivate : public QObject
{
    Q_OBJECT
public:
    QPrinterInfo printer_info;
    QPrintDialog::PrintRange print_range{QPrintDialog::PrintRange::AllPages};
    QPageLayout::Orientation page_orientation{QPageLayout::Portrait};
    QPrinter::DuplexMode duplex_mode{QPrinter::DuplexMode::DuplexNone};
    bool is_quick = false;
    bool use_system_dialog = true;
    bool color_mode = true;
    int page_from = 0,
        page_to = 0;
    int pages_count = -1,
        current_page = 0;
    int paper_width = 0,
        paper_height = 0;
    QString size_preset;
    std::wstring app_data_path;
    QVector<PageRanges> page_ranges;
    QJsonObject printers_capabilities_json;
    int sender_id = -1;
    int copies_count = 1;
    FnVoidStr m_query_callback = nullptr;

    auto parseJsonOptions(const std::wstring& json) -> bool {
        QJsonObject jsonOptions = Utils::parseJsonString(json);
        if ( jsonOptions.contains("nativeOptions") ) {
            QJsonObject native = jsonOptions["nativeOptions"].toObject();
            if ( native.contains("quickPrint") && native["quickPrint"].toBool() ) {
                is_quick = true;
                print_range = QPrintDialog::AllPages;
                return true;
            }

            if ( native.contains("printer") ) {
                QString printerName = native["printer"].toString();
                if ( !printerName.isEmpty() ) {
                    QPrinterInfo info{QPrinterInfo::printerInfo(printerName)};
                    if ( !info.isNull() )
                        printer_info = info;
                }
            }

            use_system_dialog = native.contains("usesystemdialog") ? native["usesystemdialog"].toBool() : true;
            color_mode = native.contains("colorMode") ? native["colorMode"].toBool() : true;

            if ( native.contains("pages") ) {
                QString range = native["pages"].toString();

                if ( range == "all" ) print_range = QPrintDialog::AllPages; else
                if ( range == "current" ) print_range = QPrintDialog::CurrentPage;
                else {
                    const QVector<PageRanges> ranges = parsePageRanges(range, pages_count);
                    if ( !ranges.empty() ) {
                        print_range = QPrintDialog::PageRange;
                        page_ranges = ranges;

                        page_from = ranges[0].fromPage;
                        page_to = ranges[0].toPage;

                    } else print_range = QPrintDialog::AllPages;
                }
            }

            if ( native.contains("paperOrientation") ) {
                page_orientation = native["paperOrientation"].toString() == "landscape" ? QPageLayout::Landscape : QPageLayout::Portrait;
            }

            if ( native.contains("paperSize") ) {
                QJsonObject size = native["paperSize"].toObject();

                paper_width = std::ceil(size["w"].toDouble(0));
                paper_height = std::ceil(size["h"].toDouble(0));
                if (page_orientation == QPageLayout::Landscape) {
                    std::swap(paper_width, paper_height);
                }
                size_preset = size["preset"].toString();
            }

            if ( native.contains("copies") ) {
                copies_count = native["copies"].toInt(1);
            }

            if ( native.contains("sides") ) {
                QString side = native["sides"].toString();
                if ( side == "both-long" ) {
                    duplex_mode = QPrinter::DuplexMode::DuplexLongSide;
                } else
                if ( side == "both-short" ) {
                    duplex_mode = QPrinter::DuplexMode::DuplexShortSide;
                } else {
//                    "one";
                    duplex_mode = QPrinter::DuplexMode::DuplexNone;
                }
            }

            return true;
        }

        return false;
    }

    auto init(NSEditorApi::CAscPrintEnd * data) -> void
    {
        is_quick = false;

        QPageSize def_size{QPageSize::A4};
        QSizeF paper_size = def_size.size(QPageSize::Millimeter);
        paper_width = paper_size.width();
        paper_height = paper_size.height();
        size_preset = def_size.name();

        pages_count = data->get_PagesCount();
        current_page = data->get_CurrentPage();
        print_range = QPrintDialog::AllPages;
        page_from = 1;
        page_to = pages_count;
        page_ranges.clear();
        page_ranges.push_back(PageRanges(page_from, page_to));

        parseJsonOptions(data->get_Options());
    }

    auto getPrintersCapabilitiesJson() const -> QJsonObject
    {
        bool needUpdateCache = false;
        QJsonArray printersArray, cachedPrintersArray;
        std::wstring user_data_path = app_data_path;
        const QString printers_cache = QString::fromStdWString(user_data_path.append(L"/printers.cache"));
        if (QFile::exists(printers_cache)) {
            QJsonObject cache = Utils::parseJsonFile(printers_cache);
            if (!cache.isEmpty() && cache.contains("printers")) {
                cachedPrintersArray = cache["printers"].toArray();
            }
        }

#ifdef _WIN32
        DWORD need = 0, ret = 0;
        EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, nullptr, 0, &need, &ret);
        std::vector<BYTE> buf(need);
        if (EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, buf.data(), need, &need, &ret)) {
            PRINTER_INFO_4 *printers = reinterpret_cast<PRINTER_INFO_4*>(buf.data());
            for (DWORD i = 0; i < ret; ++i) {
                QJsonObject printerObject;
                const QString driverName = getDriverName(printers[i].pPrinterName);
                if (jsonArrayContainsDriverName(cachedPrintersArray, driverName, printerObject)) {
                    printerObject["name"] = QString::fromWCharArray(printers[i].pPrinterName);
                    printersArray.append(printerObject);
                    continue;
                } else {
                    if (!needUpdateCache)
                        needUpdateCache = true;
                }

                bool duplex_supported = (DeviceCapabilities(printers[i].pPrinterName, NULL, DC_DUPLEX, NULL, NULL) == 1);
                bool color_supported = (DeviceCapabilities(printers[i].pPrinterName, NULL, DC_COLORDEVICE, NULL, NULL) == 1);

                printerObject["driver"] = driverName;
                printerObject["name"] = QString::fromWCharArray(printers[i].pPrinterName);
                printerObject["duplex_supported"] = duplex_supported;
                printerObject["color_supported"] = color_supported;

                constexpr int PAPER_NAME_LENGTH = 64;
                bool paperNamesSuccess = false, paperSizeSuccess = false;
                std::vector<WCHAR> paperNames;
                std::vector<POINT> paperSize;
                int paperNamesCount = DeviceCapabilities(printers[i].pPrinterName, NULL, DC_PAPERNAMES, NULL, NULL);
                if (paperNamesCount > 0) {
                    paperNames.assign(paperNamesCount * PAPER_NAME_LENGTH, L'\0');
                    int res = DeviceCapabilities(printers[i].pPrinterName, NULL, DC_PAPERNAMES, paperNames.data(), NULL);
                    if (res == paperNamesCount)
                        paperNamesSuccess = true;
                }
                int paperSizeCount = DeviceCapabilities(printers[i].pPrinterName, NULL, DC_PAPERSIZE, NULL, NULL);
                if (paperSizeCount > 0) {
                    paperSize.assign(paperSizeCount, {0, 0});
                    int res = DeviceCapabilities(printers[i].pPrinterName, NULL, DC_PAPERSIZE, (LPWSTR)paperSize.data(), NULL);
                    if (res == paperSizeCount)
                        paperSizeSuccess = true;
                }
                if (paperNamesSuccess && paperSizeSuccess && paperNamesCount == paperSizeCount) {
                    QJsonArray paperArray;
                    for (int j = 0; j < paperNamesCount; ++j) {
                        if (paperSize[j].x > 0 && paperSize[j].y > 0) {
                            std::wstring paperName(&paperNames[j * PAPER_NAME_LENGTH], PAPER_NAME_LENGTH);
                            QJsonObject paperObj;
                            paperObj["name"] = QString::fromWCharArray(paperName.c_str());
                            paperObj["width"] = (double)paperSize[j].x/10;
                            paperObj["height"] = (double)paperSize[j].y/10;
                            paperArray.append(paperObj);
                        }
                    }
                    printerObject["paper_supported"] = paperArray;
                }
                printersArray.append(printerObject);
            }
        }
#else
        cups_dest_t *dests = nullptr;
        int num_dests = cupsGetDests(&dests);
        if (dests) {
            for (int i = 0; i < num_dests; i++) {
                cups_dest_t *dest = &dests[i];
                const char *ppd = cupsGetPPD(dest->name);
                if (!ppd)
                    continue;
                ppd_file_t *ppdF = ppdOpenFile(ppd);
                if (!ppdF) {
                    unlink(ppd);
                    continue;
                }
                QJsonObject printerObject;
                const QString driverName = ppdF->nickname ? QString::fromUtf8(ppdF->nickname) : "";
                if (jsonArrayContainsDriverName(cachedPrintersArray, driverName, printerObject)) {
                    printerObject["name"] = QString::fromUtf8(dest->name);
                    printersArray.append(printerObject);
                    ppdClose(ppdF);
                    unlink(ppd);
                    continue;
                } else {
                    if (!needUpdateCache)
                        needUpdateCache = true;
                }

                bool duplex_supported = ppdFindOption(ppdF, "Duplex");

                printerObject["driver"] = driverName;
                printerObject["name"] = QString::fromUtf8(dest->name);
                printerObject["duplex_supported"] = duplex_supported;
                printerObject["color_supported"] = false;

                ppd_option_t *option = ppdFirstOption(ppdF);
                while (option) {
                    if (strcmp(option->keyword, "ColorModel") == 0) {
                        for (int j = 0; j < option->num_choices; j++) {
                            if (strcmp(option->choices[j].choice, "Gray") != 0) {
                                printerObject["color_supported"] = true;
                                break;
                            }
                        }
                    } else
                    if (strcmp(option->keyword, "PageSize") == 0) {
                        QJsonArray paperArray;
                        for (int j = 0; j < option->num_choices; j++) {
                            if (strcmp(option->choices[j].choice, "Custom") != 0) {
                                if (ppd_size_t *size = ppdPageSize(ppdF, option->choices[j].choice)) {
                                    QJsonObject paperObj;
                                    paperObj["name"] = QString::fromUtf8(option->choices[j].choice);
                                    paperObj["width"] = qRound(25.4 * size->width / 72.0);
                                    paperObj["height"] = qRound(25.4 * size->length / 72.0);
                                    paperArray.append(paperObj);
                                }
                            }
                        }
                        printerObject["paper_supported"] = paperArray;
                    }
                    option = ppdNextOption(ppdF);
                }
                printersArray.append(printerObject);
                ppdClose(ppdF);
                unlink(ppd);
            }
            cupsFreeDests(num_dests, dests);
        }
#endif
        QJsonObject rootObject;
        rootObject["printers"] = printersArray;
        if (needUpdateCache) {
            const QByteArray json = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
            Utils::writeFile(printers_cache, json);
        }
        return rootObject;
    }

public slots:
    void onPrinterCapabilitiesReady(QJsonObject json)
    {
        printers_capabilities_json = json;
        if (m_query_callback)
            m_query_callback(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }
};

CPrintData::CPrintData()
    : m_priv(new CPrintData::CPrintDataPrivate)
{

}

CPrintData::~CPrintData()
{
    delete m_priv, m_priv = nullptr;
}

auto CPrintData::init(NSEditorApi::CAscPrintEnd * data) -> void
{
    m_priv->init(data);
}

auto CPrintData::init(int senderid, NSEditorApi::CAscPrintEnd * data) -> void
{
    m_priv->init(data);
    m_priv->sender_id = senderid;
}

auto CPrintData::printerInfo() const -> QPrinterInfo
{
    if ( m_priv->printer_info.printerName().isEmpty() ) {
        GET_REGISTRY_USER(reg_user);

        QString last_printer_name = reg_user.value("lastPrinterName").toString();
        if ( !last_printer_name.isEmpty() ) {
            QPrinterInfo info{QPrinterInfo::printerInfo(last_printer_name)};
            if ( !info.isNull() )
                return info;
        } /*else return QPrinterInfo()*/;

        return QPrinterInfo::defaultPrinter();
    }

    return m_priv->printer_info;
}

void CPrintData::setAppDataPath(const std::wstring &app_data_path)
{
    m_priv->app_data_path = app_data_path;
}

auto CPrintData::setPrinterInfo(const QPrinterInfo& info) -> void
{
    GET_REGISTRY_USER(reg_user);
    reg_user.setValue("lastPrinterName", info.printerName());

    m_priv->printer_info = info;
}

auto CPrintData::setPrinterInfo(const QPrinter& printer) -> void
{
    setPrinterInfo(QPrinterInfo::printerInfo(printer.printerName()));
}

auto CPrintData::pageSize() const -> QPageSize
{
    if ( !m_priv->size_preset.isEmpty() ) {
        if ( m_priv->size_preset == "A0" )
            return QPageSize(QPageSize::A0);
        if ( m_priv->size_preset == "A1" )
            return QPageSize(QPageSize::A1);
        if ( m_priv->size_preset == "A2" )
            return QPageSize(QPageSize::A2);
        if ( m_priv->size_preset == "A3" )
            return QPageSize(QPageSize::A3);
        if ( m_priv->size_preset == "A4" )
            return QPageSize(QPageSize::A4);
        if ( m_priv->size_preset == "A5" )
            return QPageSize(QPageSize::A5);
        if ( m_priv->size_preset == "A6" )
            return QPageSize(QPageSize::A6);
        if ( m_priv->size_preset == "B5" )
            return QPageSize(QPageSize::B5);
        if ( m_priv->size_preset == "Tabloid" )
            return QPageSize(QPageSize::Tabloid);
        if ( m_priv->size_preset == "Envelope DL" )
            return QPageSize(QPageSize::EnvelopeDL);
        if ( m_priv->size_preset == "Envelope #10" )
            return QPageSize(QPageSize::Comm10E);
        if ( m_priv->size_preset == "Super B/A3" )
            return QPageSize(QPageSize::SuperB);
        if ( m_priv->size_preset == "Tabloid Oversize" )
            return QPageSize(QPageSize::TabloidExtra);
        if ( m_priv->size_preset == "US Letter" )
            return QPageSize(QPageSize::Letter);
        if ( m_priv->size_preset == "US Legal" )
            return QPageSize(QPageSize::Legal);
        if ( m_priv->size_preset == "Envelope Choukei 3" )
            return QPageSize(QPageSize::EnvelopeChou3);
    } else
    if ( !m_priv->paper_width || !m_priv->paper_height )
        return QPageSize(QPageSize::A4);

    return QPageSize(QSize(m_priv->paper_width, m_priv->paper_height), QPageSize::Millimeter);
}

auto CPrintData::pageOrientation() const -> QPageLayout::Orientation
{
    return m_priv->page_orientation;
}

auto CPrintData::pageFrom() const -> int
{
    return m_priv->page_from;
}

auto CPrintData::pageTo() const -> int
{
    return m_priv->page_to;
}

auto CPrintData::pageRanges() const -> QVector<PageRanges>
{
    return m_priv->page_ranges;
}

auto CPrintData::printRange() const -> QPrintDialog::PrintRange
{
    return m_priv->print_range;
}

auto CPrintData::isQuickPrint() const -> bool
{
    return m_priv->is_quick;
}

bool CPrintData::useSystemDialog() const
{
    return m_priv->use_system_dialog;
}

auto CPrintData::colorMode() const -> bool
{
    return m_priv->color_mode;
}

auto CPrintData::pagesCount() const -> int
{
    return m_priv->pages_count;
}

auto CPrintData::pageCurrent() const -> int
{
    return m_priv->current_page;
}
auto CPrintData::viewId() const -> int
{
    return m_priv->sender_id;
}

auto CPrintData::copiesCount() const -> int
{
    return m_priv->copies_count;
}

auto CPrintData::duplexMode() const -> QPrinter::DuplexMode
{
    return m_priv->duplex_mode;
}

bool CPrintData::printerCapabilitiesReady() const
{
    return !m_priv->printers_capabilities_json.isEmpty();
}

QString CPrintData::getPrinterCapabilitiesJson() const
{
    if (!m_priv->printers_capabilities_json.isEmpty()) {
        QString currentPrinterName = printerInfo().printerName();
        if (currentPrinterName.isEmpty())
            currentPrinterName = getFirstPrinterName(m_priv->printers_capabilities_json);
        m_priv->printers_capabilities_json["current_printer"] = currentPrinterName;
    }
    return QJsonDocument(m_priv->printers_capabilities_json).toJson(QJsonDocument::Compact);
}

auto CPrintData::queryPrinterCapabilitiesAsync(const FnVoidStr &callback) const -> void
{
    m_priv->m_query_callback = callback;
    std::thread([=]() {
        QJsonObject json = m_priv->getPrintersCapabilitiesJson();
        QString currentPrinterName = printerInfo().printerName();
        if (currentPrinterName.isEmpty())
            currentPrinterName = getFirstPrinterName(json);
        json["current_printer"] = currentPrinterName;
        QMetaObject::invokeMethod(m_priv, "onPrinterCapabilitiesReady", Qt::QueuedConnection, Q_ARG(QJsonObject, json));
    }).detach();
}

#include "cprintdata.moc"
