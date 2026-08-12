#include <stdint.h>
#include <glib.h>
#include "gtkutils.h"
#include "gtkprintdialog.h"
#include "components/cmessage.h"
#include "cascapplicationmanagerwrapper.h"
#include <string>
#include <algorithm>
#ifdef HAVE_X11
#include <gdk/gdkx.h>
#endif
#include <gtk/gtkunixprint.h>
#include "components/cmessage.h"

#define PDF_PRINTER_NAME "Print to File"
#define LPR_PRINTER_NAME "Print to LPR"


typedef QPrinter::Unit QUnit;
typedef uint16_t WORD;


static gboolean on_entry_key_press(GtkWidget *wgt, GdkEventKey *event, gpointer) {
    guint keyval = gdk_keyval_to_unicode(event->keyval);
    if (g_unichar_isdigit(keyval) || keyval == '-' || keyval == ',' ||
            event->keyval == GDK_KEY_Delete   || event->keyval == GDK_KEY_BackSpace ||
            event->keyval == GDK_KEY_Left     || event->keyval == GDK_KEY_Up ||
            event->keyval == GDK_KEY_Right    || event->keyval == GDK_KEY_Down ||
            event->keyval == GDK_KEY_Return   || event->keyval == GDK_KEY_Escape ||
            event->keyval == GDK_KEY_KP_Enter || event->keyval == GDK_KEY_Tab) {
        return FALSE;
    }
    if (wgt) {
        gchar *text = gtk_widget_get_tooltip_text(wgt);
        GtkWidget *popover = gtk_popover_new(wgt);
        GtkWidget *label = gtk_label_new(text);
        gtk_container_add(GTK_CONTAINER(popover), label);
        g_object_set(G_OBJECT(label), "margin", 6, NULL);
        g_signal_connect(G_OBJECT(popover), "hide", G_CALLBACK(gtk_widget_destroy), NULL);
        g_signal_connect(G_OBJECT(popover), "key-press-event", G_CALLBACK(gtk_widget_destroy), NULL);
        g_signal_connect(G_OBJECT(popover), "button-press-event", G_CALLBACK(gtk_widget_destroy), NULL);
        gtk_widget_show_all(popover);
    }
    return TRUE;
}

static void get_page_ranges_entry(GtkWidget *dialog, gpointer user_data)
{
    GtkEntry **entry = (GtkEntry**)user_data;
    if (dialog && entry) {
        const gchar *entry_path = "GtkPrintUnixDialog.GtkBox.GtkBox.GtkNotebook.GtkBox.GtkBox.GtkBox.GtkGrid.GtkEntry";
        GtkWidget *widget = find_widget_by_path(dialog, entry_path);
        if (widget && GTK_IS_ENTRY(widget)) {
            *entry = GTK_ENTRY(widget);
            g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(on_entry_key_press), NULL);
        }
    }
}

static GtkPageRange *get_page_ranges(GtkEntry *entry, gint *num_ranges)
{
    const gchar *entry_text = entry ? gtk_entry_get_text(entry) : NULL;
    if (entry_text && *entry_text && num_ranges) {
        std::string ranges_str(entry_text);
        ranges_str.erase(std::remove_if(ranges_str.begin(), ranges_str.end(), ::isspace), ranges_str.end());
        if (ranges_str.empty())
            return NULL;

        int ranges_count = std::count(ranges_str.begin(), ranges_str.end(), ',') + 1;
        GtkPageRange *page_ranges = (GtkPageRange*)g_malloc(ranges_count * sizeof(GtkPageRange));
        memset(page_ranges, 0, ranges_count * sizeof(GtkPageRange));
        size_t start_pos = 0;
        size_t sep_pos = ranges_str.find_first_of(',');
        for (int i = 0; i < ranges_count; i++) {
            std::string range_str = ranges_str.substr(start_pos, sep_pos != std::string::npos ?
                    sep_pos - start_pos : std::string::npos);
            if (range_str.empty()) {
                g_free(page_ranges);
                return NULL;
            }

            int dash_count = std::count(range_str.begin(), range_str.end(), '-');
            if (dash_count == 0) {
                char *err = NULL;
                gint page = strtol(range_str.c_str(), &err, 10);
                if (err && *err) {
                    g_free(page_ranges);
                    return NULL;
                }
                page_ranges[i].start = page - 1;
                page_ranges[i].end = page - 1;
            } else
            if (dash_count == 1) {
                size_t dash_pos = range_str.find_first_of('-');
                std::string from_page_str = range_str.substr(0, dash_pos);
                std::string to_page_str = range_str.substr(dash_pos + 1);
                if (from_page_str.empty() || to_page_str.empty()) {
                    g_free(page_ranges);
                    return NULL;
                }

                char *err1 = NULL, *err2 = NULL;
                gint from_page = strtol(from_page_str.c_str(), &err1, 10);
                gint to_page = strtol(to_page_str.c_str(), &err2, 10);
                if ((err1 && *err1) || (err2 && *err2)) {
                    g_free(page_ranges);
                    return NULL;
                }
                page_ranges[i].start = from_page - 1;
                page_ranges[i].end = to_page - 1;
            } else {
                g_free(page_ranges);
                return NULL;
            }

            if (sep_pos != std::string::npos) {
                start_pos = sep_pos + 1;
                sep_pos = ranges_str.find_first_of(',', start_pos);
            }
        }
        *num_ranges = ranges_count;
        return page_ranges;
    }
    return NULL;
}

auto gtkPaperNameFromPageSize(const QSizeF &size)->QString
{
    QString gtkPaperName;
    constexpr double diff = 1.0;
    GList *paper_sizes = gtk_paper_size_get_paper_sizes(FALSE);
    for (GList *it = paper_sizes; it != nullptr; it = it->next) {
        GtkPaperSize *psize = (GtkPaperSize*)it->data;
        double width = gtk_paper_size_get_width(psize, GTK_UNIT_MM);
        double height = gtk_paper_size_get_height(psize, GTK_UNIT_MM);
        if (std::abs(size.width() - width) < diff && std::abs(size.height() - height) < diff) {
            gtkPaperName = gtk_paper_size_get_name(psize);
            break;
        }
    }
    g_list_free_full(paper_sizes, (GDestroyNotify)gtk_paper_size_free);
    return gtkPaperName;
}

GtkPrintDialog::GtkPrintDialog(QPrinter *printer, QWidget *parent) :
    m_printer(printer),
    m_parent(parent),
    m_title(QString()),
    m_options(PrintOptions()),
    m_print_range(PrintRange::AllPages),
    m_page_ranges(QVector<PageRanges>())
{
    m_print_range = (PrintRange)printer->printRange();
    if (m_printer->collateCopies())
        m_options |= PrintOption::PrintCollateCopies;
    m_page_ranges.append(PageRanges(m_printer->fromPage(), m_printer->toPage()));
    m_pages_count = m_printer->toPage();
}

GtkPrintDialog::~GtkPrintDialog()
{

}

void GtkPrintDialog::setWindowTitle(const QString &title)
{
    m_title = title;
}

void GtkPrintDialog::setEnabledOptions(PrintOptions enbl_opts)
{
    m_options = enbl_opts;
}

void GtkPrintDialog::setOptions(PrintOptions opts)
{
    m_options = opts;
}

void GtkPrintDialog::setPrintRange(PrintRange print_range)
{
    m_print_range = print_range;
}

QDialog::DialogCode GtkPrintDialog::exec()
{
    QDialog::DialogCode exit_code = QDialog::DialogCode::Rejected;
    ulong parent_xid = (m_parent) ? (ulong)m_parent->winId() : 0;

    auto qt_printer_name = m_printer->printerName();
    auto qt_resolution = m_printer->resolution();
    auto qt_orient = m_printer->pageLayout().orientation();
    auto qt_duplex = m_printer->duplex();
    auto qt_color_mode = m_printer->colorMode();
    auto qt_copy_count = m_printer->copyCount();
    auto qt_page_order = m_printer->pageOrder();
    auto qt_output_filename = m_printer->outputFileName();
    //auto qt_doc_name = m_printer->docName();
    //auto qt_full_page = m_printer->fullPage();
    //auto qt_color_count = m_printer->colorCount();
    //auto qt_supported_res = m_printer->supportedResolutions();
    //auto qt_supports_multi_copies = m_printer->supportsMultipleCopies();
    //auto qt_selection_option = m_printer->printerSelectionOption();
    //auto qt_output_format = m_printer->outputFormat();
    //auto qt_paper_source = m_printer->paperSource();

    // Qt-PrintOptions:
    // None = 0
    // PrintToFile = 1          - not applied
    // PrintSelection = 2       - not applied
    // PrintPageRange = 4       - not applied
    // PrintShowPageSize = 8    - not applied
    // PrintCollateCopies = 16
    // DontUseSheet = 32        - not applied
    // PrintCurrentPage = 64    - not applied
    WORD _capabilityes = (
        GTK_PRINT_CAPABILITY_PAGE_SET |
        GTK_PRINT_CAPABILITY_COPIES |
        //GTK_PRINT_CAPABILITY_SCALE |          - muted
        GTK_PRINT_CAPABILITY_REVERSE |
        GTK_PRINT_CAPABILITY_GENERATE_PDF
        //GTK_PRINT_CAPABILITY_GENERATE_PS |
        //GTK_PRINT_CAPABILITY_PREVIEW |        - muted
        //GTK_PRINT_CAPABILITY_NUMBER_UP |      - muted
        //GTK_PRINT_CAPABILITY_NUMBER_UP_LAYOUT - muted
    );

    if (m_options.testFlag(PrintOption::PrintCollateCopies))
        _capabilityes |= GTK_PRINT_CAPABILITY_COLLATE;
    GtkPrintCapabilities capabilityes = (GtkPrintCapabilities)_capabilityes;

    // Input settings
    GtkPrintSettings *settings;
    settings = gtk_print_settings_new();
    {
        gtk_print_settings_set_printer(settings, (qt_printer_name == "") ?
                                           PDF_PRINTER_NAME : qt_printer_name.toUtf8().data());
        gtk_print_settings_set_resolution(settings, qt_resolution);
        gtk_print_settings_set_use_color(settings, (qt_color_mode == QPrinter::Color) ? TRUE : FALSE);

        GtkPrintQuality quality_arr[4] = {
            GTK_PRINT_QUALITY_LOW,
            GTK_PRINT_QUALITY_NORMAL,
            GTK_PRINT_QUALITY_HIGH,
            GTK_PRINT_QUALITY_DRAFT
        };
        gtk_print_settings_set_quality(settings, quality_arr[2]);
        //gtk_print_settings_set_printer_lpi(settings, 1);
        //gtk_print_settings_set_paper_size(settings, psize);
        //gtk_print_settings_set_paper_width(settings, m_printer->widthMM(), unit);
        //gtk_print_settings_set_paper_height(settings, m_printer->heightMM(), unit);
        //gtk_print_settings_set_orientation(settings, ornt);

        // Qt-PrintRange:
        // AllPages = 0
        // Selection = 1
        // PageRange = 2
        // CurrentPage = 3
        GtkPrintPages pages_arr[4] = {
            GTK_PRINT_PAGES_ALL,
            GTK_PRINT_PAGES_SELECTION,
            GTK_PRINT_PAGES_RANGES,
            GTK_PRINT_PAGES_CURRENT
        };
        const int print_range = (int)m_print_range;
        GtkPrintPages print_pages = (print_range >= 0 && print_range <= 4) ?
                    pages_arr[m_print_range] : pages_arr[0];
        gtk_print_settings_set_print_pages(settings, print_pages);

        const gint input_num_ranges = 1;
        GtkPageRange input_range[input_num_ranges] = {
            {m_printer->fromPage() - 1, m_printer->toPage() - 1}
        };
        gtk_print_settings_set_page_ranges(settings, input_range, input_num_ranges);

        GtkPageSet page_set_arr[3] = {
            GTK_PAGE_SET_ALL,
            GTK_PAGE_SET_EVEN,
            GTK_PAGE_SET_ODD
        };
        gtk_print_settings_set_page_set(settings, page_set_arr[0]);
        gtk_print_settings_set_collate(settings, m_printer->collateCopies() ? TRUE : FALSE);

        // Qt-Duplex:
        // DuplexNone = 0
        // DuplexAuto = 1       - not applied
        // DuplexLongSide = 2
        // DuplexShortSide = 3
        GtkPrintDuplex duplex_arr[3] = {
            GTK_PRINT_DUPLEX_SIMPLEX,
            GTK_PRINT_DUPLEX_HORIZONTAL,
            GTK_PRINT_DUPLEX_VERTICAL
        };
        GtkPrintDuplex duplex = (qt_duplex == QPrinter::DuplexLongSide) ?  duplex_arr[1] :
                                (qt_duplex == QPrinter::DuplexShortSide) ? duplex_arr[2] :
                                                                           duplex_arr[0];
        gtk_print_settings_set_duplex(settings, duplex);
        gtk_print_settings_set_reverse(settings,
            (qt_page_order == QPrinter::LastPageFirst) ? TRUE : FALSE);

        gtk_print_settings_set_n_copies(settings, qt_copy_count);
        gtk_print_settings_set_scale(settings, 100);
        gtk_print_settings_set(settings, GTK_PRINT_SETTINGS_OUTPUT_URI, g_filename_to_uri(
                                   qt_output_filename.toUtf8().data(), NULL, NULL));
        //gtk_print_settings_set_number_up(settings, 1);
        //gtk_print_settings_set_number_up_layout(settings,);
        //gtk_print_settings_set_dither(settings, "");
        //gtk_print_settings_set_finishings(settings, "");
    }

    // Input page setup
    GtkPageSetup *page_setup;
    page_setup = gtk_page_setup_new();
    {
        GtkUnit unit = GtkUnit::GTK_UNIT_MM;
        QUnit qt_unit(QUnit::Millimeter);
        double left_in, top_in, right_in, bottom_in;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QMarginsF margins = m_printer->pageLayout().margins(QPageLayout::Millimeter);
        left_in = margins.left();
        top_in = margins.top();
        right_in = margins.right();
        bottom_in = margins.bottom();
#else
        m_printer->getPageMargins(&left_in, &top_in, &right_in, &bottom_in, qt_unit);
#endif
        gtk_page_setup_set_left_margin(page_setup, left_in, unit);
        gtk_page_setup_set_top_margin(page_setup, top_in, unit);
        gtk_page_setup_set_right_margin(page_setup, right_in, unit);
        gtk_page_setup_set_bottom_margin(page_setup, bottom_in, unit);

        QPageSize ps = m_printer->pageLayout().pageSize();
        QSizeF page_size = ps.size(QPageSize::Millimeter);
        const QString paper_name = gtkPaperNameFromPageSize(page_size);
        GtkPaperSize *psize = gtk_paper_size_new_custom(
                    paper_name.toUtf8().data(),
                    ps.name().toUtf8().data(),
                    page_size.width(),
                    page_size.height(),
                    unit);
        gtk_page_setup_set_paper_size(page_setup, psize);
        gtk_paper_size_free(psize);

        // Qt-Orient:
        // Portrait = 0
        // Landscape = 1
        GtkPageOrientation ornt_arr[4]  = {
            GTK_PAGE_ORIENTATION_PORTRAIT,
            GTK_PAGE_ORIENTATION_LANDSCAPE,
            GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT,
            GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE
        };
        const int print_ornt = (int)qt_orient;
        GtkPageOrientation ornt = (print_ornt >= 0 && print_ornt <= 4) ?
                    ornt_arr[print_ornt] : ornt_arr[0];
        gtk_page_setup_set_orientation(page_setup, ornt);
    }

    // Init dialog
    if (AscAppManager::isRtlEnabled())
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
    GtkWidget *dialog;
    dialog = gtk_print_unix_dialog_new(m_title.toUtf8().data(), NULL);   
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);

    GtkEntry *page_ranges_entry = NULL;
    g_signal_connect(G_OBJECT(dialog), "map", G_CALLBACK(get_page_ranges_entry), (gpointer)&page_ranges_entry);
    //g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    g_signal_connect(G_OBJECT(dialog), "map_event", G_CALLBACK(set_focus), NULL);
    DialogTag tag;  // unable to send parent_xid via g_signal_connect and "focus_out_event"
    memset(&tag, 0, sizeof(tag));
    tag.dialog = dialog;
    tag.parent_xid = (ulong)parent_xid;
    g_signal_connect_swapped(G_OBJECT(dialog), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);

    gtk_print_unix_dialog_set_manual_capabilities(GTK_PRINT_UNIX_DIALOG(dialog), capabilityes);
    gtk_print_unix_dialog_set_embed_page_setup(GTK_PRINT_UNIX_DIALOG(dialog), TRUE);
    gtk_print_unix_dialog_set_settings(GTK_PRINT_UNIX_DIALOG(dialog), settings);
    gtk_print_unix_dialog_set_page_setup(GTK_PRINT_UNIX_DIALOG(dialog), page_setup);
    gtk_print_unix_dialog_set_has_selection(GTK_PRINT_UNIX_DIALOG(dialog), TRUE);
    gtk_print_unix_dialog_set_support_selection(GTK_PRINT_UNIX_DIALOG(dialog),
            m_options.testFlag(PrintOption::PrintSelection) ? TRUE : FALSE);
    gtk_print_unix_dialog_set_current_page(GTK_PRINT_UNIX_DIALOG(dialog), 0);

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    g_object_unref (G_OBJECT(page_setup));
    g_object_unref(G_OBJECT(settings));
    switch (res) {
    case GTK_RESPONSE_OK: {
        exit_code = QDialog::DialogCode::Accepted;
        GtkPrinter *printer;
        printer = gtk_print_unix_dialog_get_selected_printer(GTK_PRINT_UNIX_DIALOG(dialog));
        {
            const char* name = gtk_printer_get_name(printer);
            m_printer->setPrinterName(QString::fromUtf8(
                                          strcmp(name, PDF_PRINTER_NAME) == 0 ? "" : name));
        }

        settings = gtk_print_unix_dialog_get_settings(GTK_PRINT_UNIX_DIALOG(dialog));
        {
            enum _ColorMode {_GrayScale, _Color}; // GrayScale is defined already
            gboolean use_color = gtk_print_settings_get_use_color(settings);
            m_printer->setColorMode(use_color == TRUE ?
                          QPrinter::ColorMode(_Color) : QPrinter::ColorMode(_GrayScale));

            GtkPrintPages print_pages = gtk_print_settings_get_print_pages(settings);
            PrintRange range_arr[4] = {
                PrintRange::AllPages,
                PrintRange::CurrentPage,
                PrintRange::PageRange,
                PrintRange::Selection
            };
            const int _print_pages = (int)print_pages;
            m_print_range = (_print_pages >= 0 && _print_pages <= 4) ?
                        range_arr[_print_pages] : range_arr[0];

            GtkPageSet page_set = gtk_print_settings_get_page_set(settings);
            GtkPageRange* page_ranges = NULL;
            gint num_ranges = 0;
            // Reverse range order not working in GTK3
            page_ranges = (m_print_range == PrintRange::PageRange) ? get_page_ranges(page_ranges_entry, &num_ranges) :
                                                                     gtk_print_settings_get_page_ranges(settings, &num_ranges);
            if (!m_page_ranges.isEmpty())
                m_page_ranges.clear();

            if (page_ranges) {
                int pagesCount = m_printer->toPage();
                for (gint i = 0; i < num_ranges; i++) {
                    int start = page_ranges[i].start + 1;
                    int end = page_ranges[i].end + 1;
                    start > pagesCount && (start = pagesCount);
                    end > pagesCount && (end = pagesCount);
                    bool reverse = (start > end);
                    if (page_set == GTK_PAGE_SET_ALL) {
                        if (reverse)
                            for (int page = start; page >= end; page--)
                                m_page_ranges.append(PageRanges(page, page));
                        else
                            m_page_ranges.append(PageRanges(start, end));
                    } else {
                        for (int page = start; reverse ? page >= end : page <= end; reverse ? page-- : page++) {
                            if (page % 2 == 0 && page_set == GTK_PAGE_SET_EVEN)
                                m_page_ranges.append(PageRanges(page, page));   // Filter Even pages
                            else
                            if (page % 2 != 0 && page_set == GTK_PAGE_SET_ODD)
                                m_page_ranges.append(PageRanges(page, page));   // Filter Odd pages
                        }
                    }
                    if (i == 0)
                        m_printer->setFromTo(reverse ? end : start, reverse ? start : end);
                }
                g_free(page_ranges);
            } else {
                res = GTK_RESPONSE_REJECT;
                exit_code = QDialog::DialogCode::Rejected;
            }

            gboolean collate = gtk_print_settings_get_collate(settings);
            m_printer->setCollateCopies((bool)collate);

            GtkPrintDuplex duplex = gtk_print_settings_get_duplex(settings);
            QPrinter::DuplexMode duplex_arr[4] = {
                QPrinter::DuplexNone,
                QPrinter::DuplexAuto,
                QPrinter::DuplexLongSide,
                QPrinter::DuplexShortSide
            };
            m_printer->setDuplex(duplex == GTK_PRINT_DUPLEX_HORIZONTAL ? duplex_arr[2] :
                                 duplex == GTK_PRINT_DUPLEX_VERTICAL ?   duplex_arr[3] :
                                                                         duplex_arr[0]);

            gboolean reverse = gtk_print_settings_get_reverse(settings);
            m_printer->setPageOrder(reverse == TRUE ?
                        QPrinter::LastPageFirst : QPrinter::FirstPageFirst);

            int n_copies = gtk_print_settings_get_n_copies(settings);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            m_printer->setCopyCount(n_copies);
#else
            m_printer->setNumCopies(n_copies);
#endif

            const char* output_uri = gtk_print_settings_get(settings, GTK_PRINT_SETTINGS_OUTPUT_URI);
            auto path = QUrl::fromPercentEncoding(QByteArray(output_uri)).replace("file://", "");
            m_printer->setOutputFileName(path);

            //gtk_print_settings_get_quality(settings);
            //gtk_print_settings_get_scale(settings);
            //gtk_print_settings_get_number_up(settings);
            //gtk_print_settings_get_number_up_layout(settings);
            //gtk_print_settings_get_dither(settings);
            //gtk_print_settings_get_finishings(settings);
        }
        page_setup = gtk_print_unix_dialog_get_page_setup(GTK_PRINT_UNIX_DIALOG(dialog));
        {
            QUnit qt_unit(QUnit::Millimeter);
            GtkUnit unit = GtkUnit::GTK_UNIT_MM;
            gdouble left = gtk_page_setup_get_left_margin(page_setup, unit);
            gdouble top = gtk_page_setup_get_top_margin(page_setup, unit);
            gdouble right = gtk_page_setup_get_right_margin(page_setup, unit);
            gdouble bottom = gtk_page_setup_get_bottom_margin(page_setup, unit);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            m_printer->setPageMargins(QMarginsF(left, top, right, bottom), QPageLayout::Millimeter);
#else
            m_printer->setPageMargins(left, top, right, bottom, qt_unit);
#endif

            GtkPaperSize *paper_size = gtk_page_setup_get_paper_size(page_setup);
            //const char* paper_name = gtk_paper_size_get_display_name(paper_size);
            gdouble width = gtk_paper_size_get_width(paper_size, unit);
            gdouble height = gtk_paper_size_get_height(paper_size, unit);
            QPageSize ps(QSizeF(width, height), QPageSize::Millimeter);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            m_printer->setPageSize(ps);
#else
            m_printer->setPaperName(ps.name());
            m_printer->setPaperSize(QSizeF(width, height), qt_unit);
#endif

            GtkPageOrientation orient = gtk_page_setup_get_orientation(page_setup);
            QPageLayout plt = m_printer->pageLayout();
            plt.setOrientation((orient == GtkPageOrientation::GTK_PAGE_ORIENTATION_PORTRAIT ||
                                orient == GtkPageOrientation::GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT) ? QPageLayout::Portrait : QPageLayout::Landscape);
            m_printer->setPageLayout(plt);
        }
        break;
    }
    case GTK_RESPONSE_APPLY: {  // ask for preview
        break;
    }
    default:
        break;
    } 
    gtk_widget_destroy(dialog);

    if (res == GTK_RESPONSE_REJECT)
        CMessage::error(m_parent, QObject::tr("The syntaxis for the page range is invalid.<br>"
                                              "Enter one or more page ranges, for example: 1-3,7,11."));

    return exit_code;
}

PrintRange GtkPrintDialog::printRange()
{
    return m_print_range;
}

PrintOptions GtkPrintDialog::enabledOptions()
{
    return m_options;
}

PrintOptions GtkPrintDialog::options()
{
    return m_options;
}

QVector<PageRanges> GtkPrintDialog::getPageRanges()
{
    return m_page_ranges;
}

int GtkPrintDialog::fromPage()
{
    return m_printer->fromPage();
}

int GtkPrintDialog::toPage()
{
    return m_printer->toPage();
}

void GtkPrintDialog::setFromTo(int from, int to)
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
    m_page_ranges.append(PageRanges(m_printer->fromPage(), m_printer->toPage()));
}

void GtkPrintDialog::accept()
{

}
