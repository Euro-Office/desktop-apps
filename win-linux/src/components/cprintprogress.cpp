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

#ifdef __linux__
# include <gtk/gtk.h>
# include "cascapplicationmanagerwrapper.h"
# include "platform_linux/gtkutils.h"
# include <gdk/gdkx.h>
# include "defines.h"
#else
# include <shlobj.h>
# include <combaseapi.h>
#endif
#include "components/cprintprogress.h"
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include "utils.h"

#ifdef __linux__
static void on_response(GtkDialog*, gint resp_id, gpointer data) {
    switch (resp_id) {
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL: {
        bool *isRejected = (bool*)data;
        *isRejected = true;
        break;
    }
    default:
        break;
    }
}

static gboolean on_key_press(GtkWidget*, GdkEventKey *ev, gpointer) {
    if (ev->keyval == GDK_KEY_Escape || ev->keyval == GDK_KEY_space)
        return TRUE;
    return FALSE;
}
#endif

class CPrintProgress::CPrintProgressPrivate
{
#pragma push_macro("KeyPress")
#undef KeyPress
    class CDialog : public QDialog
    {
    public:
        CDialog(QWidget *parent = nullptr) : QDialog(parent)
        {}
    private:
        virtual bool event(QEvent *ev) override
        {
            if (ev->type() == QEvent::KeyPress) {
                QKeyEvent *kev = static_cast<QKeyEvent*>(ev);
                if (kev->key() == Qt::Key_Escape)
                    return true;
            }
            return QDialog::event(ev);
        }
    };
#pragma pop_macro("KeyPress")

public:
    CPrintProgressPrivate(QWidget *parent = nullptr) {
        useNativeDialog = WindowHelper::useNativeDialog();
        const QString primaryText = QObject::tr("Printing...", "CPrintProgress");
        const QString secondaryText = QObject::tr("Document is preparing", "CPrintProgress");
        const QString cancelText = QObject::tr("&Cancel", "CPrintProgress");
        if (useNativeDialog) {
#ifdef _WIN32
            parentHwnd = parent ? (HWND)parent->winId() : NULL;
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr)) {
                hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (void**)&winDlg);
                if (SUCCEEDED(hr)) {
                    winDlg->SetTitle(primaryText.toStdWString().c_str());
                    //winDlg->SetLine(1, L"Processing", FALSE, NULL);
                    winDlg->SetLine(2, secondaryText.toStdWString().c_str(), FALSE, NULL);
                } else {
                    CoUninitialize();
                }
            }
#else
            Window parent_xid = (parent) ? (Window)parent->winId() : 0L;
            if (AscAppManager::isRtlEnabled())
                gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
            GtkDialogFlags flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
            gtkDlg = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_OTHER, GTK_BUTTONS_NONE, "%s", primaryText.toLocal8Bit().data());
            gtk_window_set_type_hint(GTK_WINDOW(gtkDlg), GDK_WINDOW_TYPE_HINT_DIALOG);
            gtk_window_set_default_size(GTK_WINDOW(gtkDlg), 350, 150);
            if (GtkWidget *img = gtk_message_dialog_get_image(GTK_MESSAGE_DIALOG(gtkDlg)))
                gtk_widget_destroy(img);
            g_signal_connect(G_OBJECT(gtkDlg), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
            g_signal_connect(G_OBJECT(gtkDlg), "map_event", G_CALLBACK(set_focus), NULL);
            g_signal_connect(G_OBJECT(gtkDlg), "response", G_CALLBACK(on_response), (gpointer)&isRejected);
            g_signal_connect(G_OBJECT(gtkDlg), "key-press-event", G_CALLBACK(on_key_press), NULL);
            tag.dialog = gtkDlg; // unable to send parent_xid via g_signal_connect and "focus_out_event"
            tag.parent_xid = (ulong)parent_xid;
            g_signal_connect_swapped(G_OBJECT(gtkDlg), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);
            //gtk_window_set_title(GTK_WINDOW(dialog), APP_TITLE);
            gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(gtkDlg), "%s", secondaryText.toLocal8Bit().data());

            gtk_dialog_add_button(GTK_DIALOG(gtkDlg), cancelText.toLocal8Bit().data(), GTK_RESPONSE_CANCEL);
            gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(gtkDlg), GTK_RESPONSE_CANCEL));
            gtkProgressBar = gtk_progress_bar_new();
            gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(gtkProgressBar), 0.05);
            //GtkWidget *cont_area = gtk_dialog_get_content_area(GTK_DIALOG(gtkDlg));
            GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(gtkDlg));
            gtk_container_add(GTK_CONTAINER(msg_area), gtkProgressBar);

            gtk_widget_realize(gtkDlg);
            while (gtk_events_pending())
                gtk_main_iteration_do(FALSE);
#endif
        } else {
            auto _dpi_ratio = Utils::getScreenDpiRatioByWidget(parent);
            qtDlg = new CDialog(parent);
            qtDlg->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::MSWindowsFixedSizeDialogHint);
            qtDlg->setMinimumWidth(400*_dpi_ratio);
            qtDlg->setWindowTitle(primaryText);

            QVBoxLayout * layout = new QVBoxLayout;
            layout->setSizeConstraint(QLayout::SetMaximumSize);

            qtProgressLabel = new QLabel;
            qtProgressLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
            qtProgressLabel->setText(secondaryText);
            qtProgressLabel->setStyleSheet(QString("margin-bottom: %1px;").arg(8*_dpi_ratio));
            layout->addWidget(qtProgressLabel);

            QPushButton * btn_cancel = new QPushButton(cancelText);
            QWidget * box = new QWidget;
            box->setLayout(new QHBoxLayout);
            box->layout()->addWidget(btn_cancel);
            box->layout()->setContentsMargins(0,8*_dpi_ratio,0,0);
            layout->addWidget(box, 0, Qt::AlignCenter);

            qtDlg->setLayout(layout);
            qtDlg->setResult(QDialog::Accepted);
            QObject::connect(btn_cancel, &QPushButton::clicked, qtDlg, &QDialog::reject);
        }
    }

    ~CPrintProgressPrivate() {
        if (useNativeDialog) {
#ifdef _WIN32
            if (winDlg) {
                winDlg->StopProgressDialog();
                winDlg->Release();
                CoUninitialize();
                if (parentHwnd) {
                    SetForegroundWindow(parentHwnd);
                }
            }
#else
            if (gtkDlg && !gtk_widget_in_destruction(gtkDlg))
                gtk_widget_destroy(gtkDlg);
#endif
        } else {
            if (qtDlg)
                qtDlg->deleteLater();
        }
    }

#ifdef _WIN32
    IProgressDialog *winDlg = nullptr;
    HWND       parentHwnd = nullptr;
#else
    GtkWidget *gtkDlg = nullptr;
    GtkWidget *gtkProgressBar = nullptr;
    bool       isRejected = false;
#endif
    CDialog  *qtDlg = nullptr;
    QLabel   *qtProgressLabel = nullptr;
    bool      useNativeDialog = true;

private:
#ifdef __linux
    DialogTag tag;
#endif
};

CPrintProgress::CPrintProgress(QWidget * parent)
    : QObject(parent),
    pimpl(new CPrintProgressPrivate(parent))
{}

CPrintProgress::~CPrintProgress()
{
    delete pimpl, pimpl = nullptr;
}

void CPrintProgress::setProgress(int current, int count)
{
    QString line = tr("Document is printing: page %1 of %2").arg(QString::number(current), QString::number(count));
    if (pimpl->useNativeDialog) {
#ifdef _WIN32
        if (pimpl->winDlg) {
            pimpl->winDlg->SetLine(2, line.toStdWString().c_str(), FALSE, NULL);
            pimpl->winDlg->SetProgress(current, count);
        }
#else
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pimpl->gtkDlg), "%s", line.toLocal8Bit().data());
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pimpl->gtkProgressBar), (gdouble)current/count);
        while (gtk_events_pending())
            gtk_main_iteration_do(FALSE);
#endif
    } else {
        pimpl->qtProgressLabel->setText(line);
    }
}

void CPrintProgress::startProgress()
{
    if (pimpl->useNativeDialog) {
#ifdef _WIN32
        if (pimpl->winDlg) {
            pimpl->winDlg->StartProgressDialog(pimpl->parentHwnd, NULL, PROGDLG_NORMAL /*| PROGDLG_NOTIME | PROGDLG_AUTOTIME*/ | PROGDLG_MODAL | PROGDLG_NOMINIMIZE, NULL);

            HWND winDlgHwnd = nullptr;
            IOleWindow *oleWnd = nullptr;
            HRESULT hr = pimpl->winDlg->QueryInterface(IID_IOleWindow, (LPVOID*)&oleWnd);
            if (SUCCEEDED(hr)) {
                hr = oleWnd->GetWindow(&winDlgHwnd);
                if (FAILED(hr))
                    winDlgHwnd = nullptr;
                oleWnd->Release();
            }
            if (winDlgHwnd) {
                RECT parentRc, dlgRc;
                GetWindowRect(pimpl->parentHwnd, &parentRc);
                GetWindowRect(winDlgHwnd, &dlgRc);
                int x = parentRc.left + (parentRc.right - parentRc.left - dlgRc.right + dlgRc.left) / 2;
                int y = parentRc.top + (parentRc.bottom - parentRc.top - dlgRc.bottom + dlgRc.top) / 2;
                SetWindowPos(winDlgHwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                ShowWindow(winDlgHwnd, SW_SHOW);
            }
        }
#else
        gtk_widget_show_all(pimpl->gtkDlg);
        Utils::processMoreEvents(100);
#endif
    } else {
        pimpl->qtDlg->show();
#ifdef __linux
        Utils::processMoreEvents(100);
#endif
    }
}

bool CPrintProgress::isRejected()
{
    if (pimpl->useNativeDialog) {
#ifdef _WIN32
        return pimpl->winDlg && pimpl->winDlg->HasUserCancelled();
#else
        return pimpl->isRejected;
#endif
    } else {
        return pimpl->qtDlg->result() == QDialog::Rejected;
    }
}
