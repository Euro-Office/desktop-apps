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
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#ifndef CASCAPPLICATIONMANAGERWRAPPER_PRIVATE_H
#define CASCAPPLICATIONMANAGERWRAPPER_PRIVATE_H

#include "cascapplicationmanagerwrapper.h"
#include "qcefview_media.h"
#include "defines.h"
#include "clangater.h"
#include "cappeventfilter.h"
#include "OfficeFileFormats.h"
#include "ceditortools.h"
#include "utils.h"
#include "components/cmessage.h"
#include "cprintdata.h"
#include "clogger.h"
#include "common/File.h"
#include <QApplication>
#include <QJsonParseError>
#ifdef _WIN32
# define APP_LAUNCH_NAME "\\DesktopEditors.exe"
# define RESTART_BATCH "/apprestart.bat"
#else
# include "platform_linux/linux_window_utils.h"
# include <QProcess>
# include <unistd.h>
# include <sys/types.h>
# define APP_LAUNCH_NAME "/DesktopEditors"
# define RESTART_BATCH "/apprestart.sh"
#endif

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "platform_linux/cdialogopenssl.h"
#endif


class CAscApplicationManagerWrapper_Private
{
public:
    CAscApplicationManagerWrapper_Private(CAscApplicationManagerWrapper * manager)
        : m_appmanager(*manager)
    {
        qApp->installEventFilter(new CAppEventFilter(this));

        GET_REGISTRY_USER(reg_user);
        m_openEditorWindow = reg_user.value("editorWindowMode").toBool();
        m_printData = std::make_shared<CPrintData>();
    }

    virtual ~CAscApplicationManagerWrapper_Private() {}

    virtual void initializeApp() {
        m_printData->setAppDataPath(m_appmanager.m_oSettings.app_data_path);
    }
    virtual bool processEvent(NSEditorApi::CAscCefMenuEvent * event) {
        if ( detectDocumentOpening(*event) )
            return true;

        return false;
    }
    virtual void applyStylesheets() {}
    virtual void addStylesheets(CScalingFactor f, const std::string& s)
    {
        m_appmanager.addStylesheets(f, s);
    }

    virtual QCefView * createView(QWidget * parent, const QSize& s)
    {
        return new QCefView_Media(parent, s + QSize(1,0));
    }

    bool allowedCreateLocalFile()
    {
        return true;
    }

    virtual void init()
    {
    }

    void restartApp()
    {
        const QString fileName = QDir::tempPath() + RESTART_BATCH;
        if (QFile::exists(fileName) && !QFile::remove(fileName)) {
            CLogger::log("An error occurred while deleting: " + fileName);
            return;
        }
        QFile f(fileName);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
#ifdef _WIN32
            ts << "@chcp 65001>nul\n";
            ts << "@echo off\n";
            ts << "start \"\" \"" << QString::fromStdWString(NSFile::GetProcessDirectory()) << APP_LAUNCH_NAME << "\"\n";
            ts << "del \"%~f0\"&exit\n";
#else
            ts << "#!/bin/bash\n";
            ts << "sleep 1\n";
            QString appDir = QString::fromStdWString(NSFile::GetProcessDirectory());
            ts << "cd \"" << appDir << "\"\n";
            ts << "LD_LIBRARY_PATH=\"$PWD:$PWD/converter:$LD_LIBRARY_PATH\"";
            ts << " LD_PRELOAD=libcef.so";
            ts << " \"" << appDir << APP_LAUNCH_NAME << "\" &\n";
            ts << "rm -- \"$0\"\n";
#endif
            if (!f.flush()) {
                CLogger::log("An error occurred while writing: " + fileName);
                f.close();
                return;
            }
            f.close();
        } else {
            CLogger::log("An error occurred while creating: " + fileName);
            return;
        }
#ifdef _WIN32
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        if (!CreateProcess(NULL, const_cast<LPWSTR>(fileName.toStdWString().c_str()), NULL, NULL, FALSE,
                           CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi)) {
            CLogger::log("An error occurred while restarting the app!");
            return;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
#else
        // Use fork/exec instead of QProcess::startDetached because
        // this runs from a destructor after Qt's event loop has stopped,
        // where QProcess may not work reliably.
        pid_t pid = fork();
        if (pid == 0) {
            setsid();
            execlp("/bin/sh", "/bin/sh", fileName.toLocal8Bit().constData(), (char *)nullptr);
            _exit(1);
        } else if (pid < 0) {
            CLogger::log("An error occurred while restarting the app!");
        }
#endif
    }

//    auto createStartPanel() -> void {
//        GET_REGISTRY_USER(reg_user)

//        m_pStartPanel = AscAppManager::createViewer(nullptr, QSize());
//        m_pStartPanel->Create(&m_appmanager, cvwtSimple);
//        m_pStartPanel->setObjectName("mainPanel");

//        QString data_path;
//#if defined(QT_DEBUG)
//        data_path = reg_user.value("startpage").value<QString>();
//#endif

//        if ( data_path.isEmpty() )
//            data_path = qApp->applicationDirPath() + "/index.html";

//        QString additional = "?waitingloader=yes&lang=" + CLangater::getCurrentLangCode();
//        QString _portal = reg_user.value("portal").value<QString>();
//        if (!_portal.isEmpty()) {
//            QString arg_portal = (additional.isEmpty() ? "?portal=" : "&portal=") + _portal;
//            additional.append(arg_portal);
//        }

//        std::wstring start_path = ("file:///" + data_path + additional).toStdWString();
//        m_pStartPanel->GetCefView()->load(start_path);
//    }

    auto sendOpenFolderEvent(int id) -> void {
        if (!mainWindow() || mainWindow()->startPanelId() != id) {   // Ignore start page
            NSEditorApi::CAscCefMenuEvent * ns_event = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND);
            NSEditorApi::CAscExecCommand * pData = new NSEditorApi::CAscExecCommand;
            pData->put_Command(L"open:folder");
            ns_event->m_pData = pData;
            ns_event->m_nSenderId = id;
            m_appmanager.OnEvent(ns_event);
        }
    }

    auto handleAppKeyPress(QKeyEvent * e) -> bool
    {
        if (e->key() == Qt::Key_O && (e->modifiers() & Qt::ControlModifier)) {
            //sendOpenFolderEvent(-1);
            return true;
        }

        return false;
    }

    auto preferOpenEditorWindow() -> bool
    {
        return m_openEditorWindow;
    }

    auto detectDocumentOpening(const NSEditorApi::CAscCefMenuEvent& event) -> bool
    {
        switch ( event.m_nType ) {
        case ASC_MENU_EVENT_TYPE_CEF_CREATETAB: {
            NSEditorApi::CAscCreateTab & data = *static_cast<NSEditorApi::CAscCreateTab *>(event.m_pData);
//            data.get_Active();

            COpenOptions opts{data.get_Url()};
            //            opts.id = data.get_IdEqual();
            opts.parent_id = event.m_nSenderId;
            opts.name = QString::fromStdWString(data.get_Name());

            if ( !bringEditorToFront(QString::fromStdWString(opts.wurl)) )
                openDocument(opts);

            return true;
        }
        case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND: {
            NSEditorApi::CAscExecCommand & data = *static_cast<NSEditorApi::CAscExecCommand *>(event.m_pData);
            const std::wstring & cmd = data.get_Command();

            if ( cmd.compare(L"open:recent") == 0 ) {
                QJsonObject objRoot = Utils::parseJsonString(data.get_Param());
                if ( !objRoot.isEmpty() ) {
                    QString _path = objRoot["path"].toString();
                    if ( bringEditorToFront( _path ) )
                        return true;

                    bool _from_recovery = objRoot["recovery"].toBool(false);
                    COpenOptions opts{_path.toStdWString(), _from_recovery ? etRecoveryFile : etRecentFile, objRoot["id"].toInt()};
                    opts.format = objRoot["type"].toInt();
                    opts.parent_id = event.m_nSenderId;
                    opts.name = objRoot["name"].toString();
                    opts.cloud = objRoot["cloud"].toString();

                    static const QRegularExpression re(rePortalName);
                    QRegularExpressionMatch match = re.match(opts.url);

                    if ( !_from_recovery && !match.hasMatch() ) {
                        QFileInfo _info(opts.url);
                        if ( /*!data->get_IsRecover() &&*/ !_info.exists() ) {
                            int res = CMessage::showMessage(m_appmanager.mainWindow()->handle(),
                                                            QObject::tr("%1 doesn't exists!<br>Remove file from the list?").arg(_info.fileName().toHtmlEscaped()),
                                                            MsgType::MSG_WARN, MsgBtns::mbYesDefNo);
                            if ( res == MODAL_RESULT_YES ) {
                                AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "file:skip", QString::number(opts.id));
                            } else
                            if ( res == MODAL_RESULT_NO ) {
                                int uid = objRoot["hash"].toInt();
                                m_appmanager.onFileChecked(opts.name, uid, false);
                            }

                            return true;
                        }
                    }

                    openDocument(opts);
                }

                return true;
            } else
            if ( cmd.compare(L"recovery:update") == 0 ) {
                QJsonParseError jerror;
                QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(data.get_Param()).toUtf8(), &jerror);

                if( jerror.error == QJsonParseError::NoError ) {
                    if (jdoc.isArray()) {
                        const QJsonArray arr = jdoc.array();
                        for (const auto &val : arr) {
                            QJsonObject obj = val.toObject();
                            if (obj.contains("path")) {
                                QString path = obj["path"].toString();

                                COpenOptions opts{path.toStdWString(), etRecoveryFile, obj["id"].toInt()};
                                opts.parent_id = event.m_nSenderId;
                                opts.format = obj["type"].toInt();
                                opts.name = (QFileInfo(path)).fileName();
                                openDocument(opts);
                            }
                        }
                    }
                }

                return true;
            } else
            if ( cmd.compare(L"open:document") == 0 ) {
                const std::wstring & _url = data.get_Param();
                if ( !_url.empty() ) {
                    CCefView * _view = m_appmanager.GetViewByUrl(_url);
                    if ( _url.rfind(L"http://",0) == 0 || _url.rfind(L"https://",0) == 0 ) {
                        COpenOptions opts{_url};
                        opts.id = _view ? _view->GetId() : -1;
                    } else {
//                        /* open local file */
                    }
                }
            } else
            if ( cmd.compare(L"open:folder") == 0 ||
                    (!(cmd.find(L"editor:event") == std::wstring::npos) &&
                     !(data.get_Param().find(L"file:open") == std::wstring::npos)) )
            {
                std::wstring file_path = CEditorTools::getlocalfile(data.get_Param(), event.m_nSenderId).toStdWString();

                if ( !file_path.empty() ) {
                    QString qfile_path = QString::fromStdWString(file_path);
                    if ( bringEditorToFront(qfile_path) )
                        return true;

                    QFileInfo _info(qfile_path);
                    COpenOptions opts{_info.fileName(), etLocalFile};
                    opts.parent_id = event.m_nSenderId;
                    opts.url = qfile_path;
                    opts.wurl = file_path;

                    if ( !openDocument(opts) ) {
                        CMessage::error(m_appmanager.mainWindow()->handle(),
                                        QObject::tr("File %1 cannot be opened or doesn't exists.").arg(_info.fileName()));
                    }
                }

                return true;
            } else
            if ( cmd.compare(L"create:new") == 0 ) {
                const std::wstring & format = data.get_Param();
                const std::wstring search_tpl = L"template:";

                if ( format.rfind(search_tpl, 0) == 0 ) {
                    std::wstring type = format.substr(search_tpl.length());
                    std::wstring file_path = CEditorTools::getlocaltemplate(type, event.m_nSenderId).toStdWString();

                    if ( !file_path.empty() ) {
                        COpenOptions opts{file_path, etTemplateFile};
                        opts.name = m_appmanager.newFileName(type);
                        opts.parent_id = event.m_nSenderId;

                        openDocument(opts);
                    }
                } else if ( format.rfind(L"{\"template", 0) == 0 ) {
                    QJsonParseError jerror;
                    QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(format).toUtf8(), &jerror);

                    if( jerror.error == QJsonParseError::NoError ) {
                        QJsonObject obj = jdoc.object().value("template").toObject();
                        int _f = obj.value("type").toInt();

                        COpenOptions opts{m_appmanager.newFileName(_f), etTemplateFile, obj.value("path").toString()};
                        opts.format = _f;

                        openDocument(opts);
                    }
                } else {
                    int _f = format == L"word" ? AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX :
                                 format == L"cell" ? AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX :
                                 format == L"form" ? AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF :
                                 // format == L"draw" ? AVS_OFFICESTUDIO_FILE_DRAW_VSDX :
                                 format == L"slide" ? AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX : AVS_OFFICESTUDIO_FILE_UNKNOWN;

                    COpenOptions opts{m_appmanager.newFileName(_f), etNewFile};
                    opts.format = _f;
                    opts.parent_id = event.m_nSenderId;

                    openDocument(opts);
                }

                return true;
            }

        }
        }
        return false;
    }

    auto bringEditorToFront(int viewid) -> void
    {
        CEditorWindow * editor = m_appmanager.editorWindowFromViewId(viewid);
        if ( editor  ) {
            if (!editor->isSlideshowMode())
                editor->bringToTop();
        } else m_appmanager.mainWindow()->selectView(viewid);
    }

    auto bringEditorToFront(const QString& url) -> bool
    {
        CEditorWindow * _editor = nullptr;
        CCefView * _view = m_appmanager.GetViewByUrl(url.toStdWString());
        if ( _view ) {
            int _view_id = _view->GetId();

            if ( mainWindow() && (mainWindow()->slideshowHoldView(_view_id) || mainWindow()->holdUid(_view_id)) ) {
                if (!mainWindow()->isSlideshowMode()) {
                    mainWindow()->bringToTop();
                    mainWindow()->selectView(_view_id);
                }
                return true;
            } else
                _editor = m_appmanager.editorWindowFromViewId(_view_id);
        } else {
            QString _n_url = Utils::replaceBackslash(url);

            if ( mainWindow() && (mainWindow()->slideshowHoldUrl(_n_url, etLocalFile) || mainWindow()->holdUrl(_n_url, etLocalFile)) ) {
                if (!mainWindow()->isSlideshowMode()) {
                    mainWindow()->bringToTop();
                    mainWindow()->selectView(_n_url);
                }
                return true;
            } else {
                _editor = m_appmanager.editorWindowFromUrl(_n_url);
            }
        }

        if ( _editor ) {
            if (!_editor->isSlideshowMode())
                _editor->bringToTop();
            return true;
        }

        return false;
    }

    auto windowRectFromViewId(int viewid) -> QRect
    {
        if ( !(viewid < 0) ) {
            CEditorWindow * editor = m_appmanager.editorWindowFromViewId(viewid);
            if ( editor )
                return editor->normalGeometry();
            else
            if ( m_appmanager.mainWindow() && m_appmanager.mainWindow()->holdView(viewid) )
                return m_appmanager.mainWindow()->windowRect();
        }

        return QRect();
    }

    auto editorWindowGeometry(QRect &rc, bool &isMaximized, const COpenOptions& opts) -> void
    {        
        AscEditorType etype = AscEditorType::etUndefined;
        int format = (opts.format == 0) ? CCefViewEditor::GetFileFormat(opts.wurl) : opts.format;
        switch (format) {
        case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF:
            etype = AscEditorType::etPdf;
            break;
        default:
            etype = CEditorTools::editorTypeFromFormat(format);
            break;
        }

        if (!m_appmanager.m_vecEditors.empty()) {
#ifdef _WIN32
            if (HWND hWnd = GetTopWindow(GetDesktopWindow())) {
                do {
                    WId wid = (WId)hWnd;
#else
            std::vector<WId> winStack;
            LinuxWindowUtils::getWindowStack(winStack);
            if (winStack.empty()) {
                // Wayland fallback: use our tracked editors list
                for (auto it = m_appmanager.m_vecEditors.rbegin(); it != m_appmanager.m_vecEditors.rend(); it++) {
                    CEditorWindow *editor = reinterpret_cast<CEditorWindow*>(*it);
                    if (editor && editor->editorType() == etype) {
                        rc = editor->normalGeometry();
                        rc.adjust(50, 50, 50, 50);
                        isMaximized = editor->windowState().testFlag(Qt::WindowMaximized);
                        return;
                    }
                }
            }
            for (auto it = winStack.rbegin(); it != winStack.rend(); it++) {
                WId wid = (WId)(*it);
#endif
                    QWidget *wgt = QWidget::find(wid);
                    if (wgt && wgt->isWindow()) {
                        if (CEditorWindow *editor = qobject_cast<CEditorWindow*>(wgt)) {
                            if (editor->editorType() == etype) {
                                rc = editor->normalGeometry();
                                rc.adjust(50, 50, 50, 50);
                                isMaximized = editor->windowState().testFlag(Qt::WindowMaximized);
                                return;
                            }
                        }
                    }
#ifdef _WIN32
                } while ((hWnd = GetWindow(hWnd, GW_HWNDNEXT)) != nullptr);
            }
#else
            }
#endif
        }

        GET_REGISTRY_USER(reg_user);
        if (etype == AscEditorType::etUndefined) {
            if (!rc.isEmpty())
                rc.adjust(50,50,50,50);
            isMaximized = mainWindow() ? mainWindow()->windowState().testFlag(Qt::WindowMaximized) : reg_user.value("maximized", WindowHelper::defaultWindowMaximizeState()).toBool();
        } else {
            QString baseKey = "EditorsGeometry/" + QString::number(int(etype)) + "/";
            if (reg_user.contains(baseKey + "position"))
                rc = reg_user.value(baseKey + "position").toRect();
            else {
                if (!rc.isEmpty())
                    rc.adjust(50,50,50,50);
            }

            if (reg_user.contains(baseKey + "maximized"))
                isMaximized = reg_user.value(baseKey + "maximized").toBool();
            else {
                isMaximized = mainWindow() ? mainWindow()->windowState().testFlag(Qt::WindowMaximized) : reg_user.value("maximized", WindowHelper::defaultWindowMaximizeState()).toBool();
            }
        }
    }

    auto editorWindowFromViewId(int viewid) -> CEditorWindow *
    {
        return m_appmanager.editorWindowFromViewId(viewid);
    }

    auto openDocument(const COpenOptions& opts) -> bool
    {
        COpenOptions opts_ext{opts};
        if ( preferOpenEditorWindow() ) {
            GET_REGISTRY_USER(reg_user);
            bool isMaximized = false;
            QRect rect = /*isMaximized ? QRect() :*/ windowRectFromViewId(opts.parent_id);
            editorWindowGeometry(rect, isMaximized, opts);
            opts_ext.panel_size = CWindowBase::expectedContentSize(rect, true);
            opts_ext.parent_widget = COpenOptions::eWidgetType::window;
            if (CEditorWindow * editor_win = CEditorWindow::create(rect, opts_ext)) {
                editor_win->show(isMaximized);

                m_appmanager.m_vecEditors.push_back(size_t(editor_win));
                if ( editor_win->isCustomWindowStyle() ) {
                    m_appmanager.sendCommandTo(editor_win->mainView()->cef(), L"window:features",
                                               Utils::stringifyJson(QJsonObject{{"skiptoparea", TOOLBTN_HEIGHT},{"singlewindow",true}}).toStdWString());
                }
                return true;
            }
        } else {
            m_appmanager.gotoMainWindow(size_t(m_appmanager.editorWindowFromViewId(opts.parent_id)));
            opts_ext.panel_size = mainWindow()->contentSize();
            opts_ext.parent_widget = COpenOptions::eWidgetType::tab;
            if (CTabPanel * panel = CEditorTools::createEditorPanel(opts_ext, mainWindow())) {
                CAscTabData * panel_data = panel->data();
                QRegularExpression re("^ascdesktop:\\/\\/(?:compare|merge|template)");
                if ( re.match(QString::fromStdWString(panel_data->url())).hasMatch() ) {
                    panel_data->setIsLocal(true);
                    panel_data->setUrl("");
                }
                mainWindow()->attachEditor(panel);
                return true;
            }
        }

        return false;

    }

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
    auto selectSSLSertificate(int viewid) -> void {
        QWidget * parent = m_appmanager.editorWindowFromViewId(viewid);
        if ( !parent ) {
            parent = m_appmanager.mainWindowFromViewId(viewid);
        }

        if ( parent ) {
            CDialogOpenSsl _dialog(parent);

            NSEditorApi::CAscOpenSslData * pData = new NSEditorApi::CAscOpenSslData;
            if ( _dialog.exec() == QDialog::Accepted ) {
                _dialog.getResult(*pData);
            }

            NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE);
            pEvent->m_pData = pData;
            m_appmanager.GetViewById(viewid)->Apply(pEvent);
        }
    }
#endif

protected:
    auto mainWindow() -> CMainWindow * {
        return m_appmanager.m_pMainWindow;
    }

public:
    CAscApplicationManagerWrapper& m_appmanager;
    QPointer<QCefView> m_pStartPanel;
    bool m_openEditorWindow = false;
    bool m_needRestart = false;
    bool m_notificationSupported = false;
    std::shared_ptr<CPrintData> m_printData;
#ifndef _CAN_SCALE_IMMEDIATELY
    std::wstring uiscaling;
#endif
};

//CAscApplicationManagerWrapper::CAscApplicationManagerWrapper()
//    : CAscApplicationManagerWrapper(new CAscApplicationManagerWrapper::CAscApplicationManagerWrapper_Private(this))
//{
//}

#endif // CASCAPPLICATIONMANAGERWRAPPER_PRIVATE_H
