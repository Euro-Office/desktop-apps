
#include "cascapplicationmanagerwrapper.h"
#include "cascapplicationmanagerwrapperintf.h"

#include <QMutexLocker>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <qtcomp/qdesktopwidget.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QScreen>
#include <algorithm>
#include <functional>

#include "defines.h"
#include "version.h"
#include "components/cfiledialog.h"
#include "utils.h"
#include "common/Types.h"
#include "common/File.h"
#include "ctabundockevent.h"
#include "clangater.h"
#include "components/cmessage.h"
#include "ceditortools.h"
#include "cfilechecker.h"
#include "OfficeFileFormats.h"
#include "cproviders.h"
#ifndef __OS_WIN_XP
# include "components/cnotification.h"
#endif

#ifdef _WIN32
# include <io.h>
# include <VersionHelpers.h>
# include "platform_win/singleapplication.h"
# include "platform_win/association.h"
#else
# include <unistd.h>
# include "platform_linux/singleapplication.h"
# ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
#  include "platform_linux/cdialogcertificateinfo.h"
# endif
#endif

#define APP_CAST(app) \
    CAscApplicationManagerWrapper & app = static_cast<CAscApplicationManagerWrapper &>(AscAppManager::getInstance());

#define SKIP_EVENTS_QUEUE(callback) QTimer::singleShot(0, callback)

using namespace NSEditorApi;
using namespace std;
using namespace std::placeholders;


bool CAscApplicationManagerWrapper::m_rtlEnabled = false;

std::wstring get_file_name_from_open_deeplink(const std::wstring& link)
{
    size_t pos1 = link.find(L"|n|");
    if ( pos1 != std::wstring::npos ) {
        size_t pos2 = link.find(L"|", pos1 += 3);

        if ( pos2 != std::wstring::npos )
            return link.substr(pos1, pos2 - pos1);
    }

    return L"";
}

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper(CAscApplicationManagerWrapper const&)
{

}

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper(CAscApplicationManagerWrapper_Private * ptrprivate)
    : QObject()
    , QAscApplicationManager()
    , CCefEventsTransformer(nullptr)
    , m_queueToClose(new CWindowsQueue<sWinTag>)
    , m_private(ptrprivate)
{
    qRegisterMetaType<sWinTag>("sWinTag");
#if defined(_WIN32) && !defined(QT_VERSION_6)
    qRegisterMetaType<std::vector<std::wstring>>("std::vector<std::wstring>");
#endif
    m_private->init();
    CAscApplicationManager::SetEventListener(this);

    QObject::connect(this, &CAscApplicationManagerWrapper::coreEvent, this, &CAscApplicationManagerWrapper::onCoreEvent);
    QObject::connect(CExistanceController::getInstance(), &CExistanceController::checked, this, &CAscApplicationManagerWrapper::onFileChecked);
    QObject::connect(&commonEvents(), &CEventDriver::onEditorClosed, this, &CAscApplicationManagerWrapper::onEditorWidgetClosed);

    std::function<void(sWinTag)> callback_ = [&](sWinTag t){
        QMetaObject::invokeMethod(this, "onQueueCloseWindow", Qt::QueuedConnection, Q_ARG(sWinTag, t));
    };
    m_queueToClose->setcallback(callback_);

    m_themes = std::make_shared<CThemes>();
}

CAscApplicationManagerWrapper::~CAscApplicationManagerWrapper()
{
#ifndef _CAN_SCALE_IMMEDIATELY
    if (!m_private->uiscaling.empty()) {
        setUserSettings(L"system-scale", m_private->uiscaling != L"0" ? L"0" : L"1");
        setUserSettings(L"force-scale", m_private->uiscaling == L"0" ? L"default" : m_private->uiscaling);
    }
#endif
    delete m_queueToClose, m_queueToClose = nullptr;

    if ( m_pMainWindow ) {
        m_pMainWindow->deleteLater();
    }
#if defined (_UPDMODULE)
    // Start update installation
    if (m_pUpdateManager)
        m_pUpdateManager->handleAppClose();
#endif
    if (m_private.get()->m_needRestart)
        m_private.get()->restartApp();
}

void CAscApplicationManagerWrapper::StartSaveDialog(const std::wstring& sName, unsigned int nId)
{
    CAscSaveDialog * data = new CAscSaveDialog;
    data->put_FilePath(sName);
    data->put_IdDownload(nId);

    CAscCefMenuEvent * event = new CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG);
    event->m_pData = data;

    OnEvent(event);
}

std::wstring CAscApplicationManagerWrapper::GetExternalSchemeName()
{
    std::wstring scheme = CAscApplicationManager::GetExternalSchemeName();
    return !scheme.empty() ? scheme.back() != L':' ? scheme + L":" : scheme : L"";
}

void CAscApplicationManagerWrapper::setHasFrameFeature(CCefView *cef, const wstring &param, int sid)
{
    if (param.find(L"framesize") != std::wstring::npos) {
        if (CCefViewWidgetImpl * _impl = cef->GetWidgetImpl()) {
            QJsonParseError err;
            const QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdWString(param).toUtf8(), &err);
            if ( err.error == QJsonParseError::NoError ) {
                const QJsonObject obj = doc.object()["framesize"].toObject();
                int _frame_w = obj["width"].toInt(),
                    _frame_h = obj["height"].toInt();

                QCefView * view = static_cast<QCefView *>(_impl);
                const QSize s = view->size() / Utils::getScreenDpiRatioByWidget(view);
                std::wstring feature = L"{\"hasframe\":";
                feature += ( abs(s.width() - _frame_w) > 1 || abs(s.height() - _frame_h) > 1 ) ? L"true}" : L"false}";
                if ( m_receivers.find(sid) != m_receivers.end() )
                    m_receivers[sid]->onWebAppsFeatures(sid, feature);
                else m_pMainWindow->onWebAppsFeatures(sid, feature);
            }
        }
    }
}

void CAscApplicationManagerWrapper::OnEvent(CAscCefMenuEvent * event)
{
    if ( event->m_nType == ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND ) {
        CAscExecCommand * pData = reinterpret_cast<CAscExecCommand *>(event->m_pData);

        if ( pData->get_Command().compare(L"portal:open") == 0 ) {
            event->m_nType = ASC_MENU_EVENT_TYPE_CEF_PORTAL_OPEN;
        }
    }

    QMetaObject::invokeMethod(this, "onCoreEvent", Qt::QueuedConnection, Q_ARG(void*, event));
}

void CAscApplicationManagerWrapper::onCoreEvent(void * e)
{
    CAscCefMenuEvent * _event = static_cast<CAscCefMenuEvent *>(e);

    if ( m_private->processEvent(_event) || processCommonEvent(_event) )  {
        RELEASEINTERFACE(_event);
        return;
    }

#if 0
    if ( _event->m_nType == ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN ) {
        CAscLocalFileOpen * pData = (CAscLocalFileOpen*)_event->m_pData;
        QString inpath = QString().fromStdWString(pData->get_Directory());

        _window = mainWindowFromViewId(_event->get_SenderId());
#ifdef _WIN32
        CFileDialogWrapper dlg(_window->hWnd);
#else
        CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

        QString _path = !inpath.isEmpty() && QDir(inpath).exists() ?
                            inpath : Utils::lastPath(LOCAL_PATH_OPEN);

        if (!(_path = dlg.modalOpen(_path)).isEmpty()) {
            Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_path).absolutePath());

            _event->Release();
            _event = new CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN);
            _event->put_SenderId( _uid );

            CAscLocalOpenFileRecent_Recover * pData = new CAscLocalOpenFileRecent_Recover;
            pData->put_Path( _path.toStdWString() );

            _event->m_pData = pData;
        }
    }
#endif

    if ( m_pMainWindow && m_pMainWindow->holdView(_event->get_SenderId()) ) {
        CCefEventsTransformer::OnEvent(m_pMainWindow, _event);
    } else {
/**/
        map<int, CCefEventsGate *>::const_iterator it = m_receivers.find(_event->get_SenderId());
        if ( it != m_receivers.cend() ) {
            CCefEventsTransformer::OnEvent(it->second, _event);
            return;
        }

        RELEASEINTERFACE(_event);
    }
}

bool CAscApplicationManagerWrapper::processCommonEvent(NSEditorApi::CAscCefMenuEvent * event)
{
    switch ( event->m_nType ) {
    case ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK: {
        event->AddRef();

        SKIP_EVENTS_QUEUE([event] () {
            CAscOnOpenExternalLink * const pData = static_cast<CAscOnOpenExternalLink *>(event->m_pData);
            Utils::openUrl( QString::fromStdWString(pData->get_Url()) );

            event->Release();
        });
        return true; }

    case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND: {
        CAscExecCommand * const pData = static_cast<CAscExecCommand * const>(event->m_pData);
        std::wstring const & cmd = pData->get_Command();

        if ( !(cmd.find(L"webapps:entry") == std::wstring::npos) ) {
            int sid = event->get_SenderId();
            CCefView * ptr = GetViewById(event->get_SenderId());
            if ( ptr ) {
#ifdef __OS_WIN_XP
                sendCommandTo(ptr, L"window:features", Utils::stringifyJson(QJsonObject{{"lockthemes", true}}).toStdWString());
#else
                // TODO: unlock for ver 6.4 because bug 50589
                // TODO: unlock for back compatibility with ver 6.4 on portals
//                sendCommandTo(ptr, L"uitheme:changed", themes().current().id());
#endif

                if ( !((pData->get_Param()).find(L"fillform") == std::wstring::npos) ) {
                    if ( m_receivers.find(sid) != m_receivers.end() )
                        m_receivers[sid]->onWebAppsFeatures(sid,L"{\"uitype\":\"fillform\"}");
                }

                auto * editor = editorWindowFromViewId(event->get_SenderId());
                if ( editor && editor->isCustomWindowStyle() ) {
                    QJsonObject json{{"skiptoparea", TOOLBTN_HEIGHT},{"singlewindow",true}};
                    sendCommandTo(ptr, L"window:features", Utils::stringifyJson(json).toStdWString());
                }

                if ( InputArgs::contains(L"--system-title-bar") ) {
                    QJsonObject json{{"element","body"},
                                        {"action", "merge"},
                                        {"style","#title-doc-name{display:none}"}};
                    sendCommandTo(ptr, L"style:change", Utils::stringifyJson(json).toStdWString());
                }
            }
            return true;
        } else
        if ( cmd.compare(L"provider:list") == 0 ) {
            CProviders::instance().init(QString::fromStdWString(pData->get_Param()));
        } else
        if ( cmd.compare(L"portal:login") == 0 ) {
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:login", pData->get_Param());
            if ( m_pMainWindow ) {
                m_pMainWindow->onPortalLogin(event->get_SenderId(), pData->get_Param());
            }

            return true;
        } else
        if ( cmd.compare(L"portal:logout") == 0 ) {
            const wstring& wjson = pData->get_Param();

            QJsonParseError jerror;
            QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(wjson).toUtf8(), &jerror);

            if( jerror.error == QJsonParseError::NoError ) {
                QJsonObject objRoot = jdoc.object();
                QString _portal = objRoot["domain"].toString();

                if ( !_portal.isEmpty() ) {
                    m_closeCount = logoutCount(_portal.toStdWString());

                    if ( objRoot.contains("extra") && objRoot["extra"].isArray() ) {
                        QJsonArray a = objRoot["extra"].toArray();
                        for (auto&& v: a) {
                            m_closeCount += logoutCount(v.toString().toStdWString());
                        }
                    }

                    if ( m_closeCount > 0 ) {
                        m_closeTarget = wjson;
                        broadcastEvent(event);
                    } else {
                        Logout(wjson);
                    }
                }
            }

//            RELEASEINTERFACE(event);
            return true;
        } else
        if ( cmd.find(L"app:onready") != std::wstring::npos ) {
            GET_REGISTRY_SYSTEM(reg_system)
            GET_REGISTRY_USER(reg_user)
            if (reg_system.value("lockPortals", false).toBool() || reg_user.value("lockPortals", false).toBool()
#ifdef Q_OS_WIN
                    || Utils::getWinVersion() <= Utils::WinVer::WinVista
#endif
            )
                sendCommandTo(SEND_TO_ALL_START_PAGE, "panel:hide", "connect");
        } else
        if ( cmd.compare(0, 8, L"settings") == 0 ) {
            if ( cmd.rfind(L"apply") != wstring::npos ) {
                applySettings(pData->get_Param());
            } else
            if ( cmd.rfind(L"get") != wstring::npos ) {
                sendSettings(pData->get_Param());
            } else
            if ( cmd.rfind(L"check") != wstring::npos ) {
                checkSettings(pData->get_Param());
            }

//            RELEASEINTERFACE(event);
            return true;
        } else
#ifdef _UPDMODULE
        if ( !(cmd.find(L"updates:action") == std::wstring::npos) ) {   // params: check, download, install, abort
            if (m_pUpdateManager) {
                CNotification::instance().clear();
                const QString params = QString::fromStdWString(pData->get_Param());
                if (params == "check") {
                    m_pUpdateManager->checkUpdates(true);
                } else
                if (params == "download") {
                    m_pUpdateManager->loadUpdates();
                } else
                if (params == "install") {
                    m_pUpdateManager->installUpdates();
                } else
                if (params == "abort") {
                    m_pUpdateManager->cancelLoading();
                }
                return true;
            }
        } else
#endif
        if ( cmd.compare(L"title:button") == 0 ) {
            map<int, CCefEventsGate *>::const_iterator it = m_receivers.find(event->get_SenderId());
            if ( it != m_receivers.cend() ) {
                QMetaObject::invokeMethod(it->second, "onWebTitleChanged", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()), Q_ARG(std::wstring, pData->get_Param()));
                return true;
            }
        } else
        if ( !(cmd.find(L"go:folder") == std::wstring::npos) ) {
            if ( pData->get_Param() == L"offline" ) {}
            else {
                if ( !m_pMainWindow || !m_pMainWindow->isVisible() )
                    gotoMainWindow();

                mainWindow()->onFileLocation(-1, QString::fromStdWString(pData->get_Param()));
#ifdef Q_OS_LINUX
                mainWindow()->bringToTop();
#endif
                return true;
            }
        } else
        if ( !(cmd.find(L"uitheme:changed") == std::wstring::npos) ) {
            applyTheme( themes().parseThemeName(pData->get_Param()) );
            return true;
        } else
        if ( !(cmd.find(L"uitheme:add") == std::wstring::npos) ) {
            QString file_path = CEditorTools::getlocaltheme(event->get_SenderId());

            if ( !file_path.isEmpty() ) {
                QJsonObject json_obj = Utils::parseJsonFile(file_path);
                if ( !json_obj.isEmpty() ) {
                    if ( json_obj.contains("id") ) {
                        if ( m_themes->checkDestinationThemeFileExist(file_path) ) {
                            int res = CMessage::showMessage(mainWindow(),
                                                            QObject::tr("File %1 is already loaded. Replace it?").arg(QFileInfo(file_path).fileName()),
                                                            MsgType::MSG_CONFIRM, MsgBtns::mbYesDefNo);
                            if ( res == MODAL_RESULT_NO )
                                return true;
                        }

                        if ( !themes().validate(json_obj) ) {
                            qDebug() << "theme source is broken";
                            CMessage::error(mainWindow(), "Selected theme isn't valid");
                        } else {
                            if ( themes().addLocalTheme(json_obj, file_path) ) {
                                QJsonArray local_themes_array = themes().localThemesToJson();
                                if ( !local_themes_array.isEmpty() ) {
                                    EditorJSVariables::setVariable("localthemes", local_themes_array);
                                    EditorJSVariables::apply();
                                }

                                QJsonArray new_local_themes;
                                new_local_themes.append(json_obj);
                                sendCommandToAllEditors(L"uitheme:added",
                                                        QString(QJsonDocument(new_local_themes).toJson(QJsonDocument::Compact)).toStdWString());
                            }
                        }
                    } else {
                        qDebug() << "theme source is broken";
                        CMessage::error(mainWindow(), "This file doesn't contain theme");
                    }
                } else {
                    qDebug() << "theme file is not valid";
                    CMessage::error(mainWindow(), "This theme file is not valid");
                }
            }

            return true;
        } else
        if ( !(cmd.find(L"files:check") == std::wstring::npos) ) {
            CExistanceController::check(QString::fromStdWString(pData->get_Param()));
            return true;
        } else
        if ( !(cmd.find(L"recent:forget") == std::wstring::npos) ) {
            RemoveRecentByViewId(event->get_SenderId());
            return true;
        } else
        if ( !(cmd.find(L"system:changed") == std::wstring::npos) ) {
            QRegularExpression re(":\\s?\"(dark|light)");
            QRegularExpressionMatch match = re.match(QString::fromStdWString(pData->get_Param()));
            if ( match.hasMatch() ) {
                bool is_dark = match.captured(1) == "dark";

                if ( m_themes->isSystemSchemeDark() != is_dark ) {
                    m_themes->onSystemDarkColorScheme(is_dark);

                    QJsonObject jparams{{"theme", QJsonObject{{"system", match.captured(1)}}}};
                    QString params = Utils::stringifyJson(jparams);
                    for (auto i: GetViewsId()) {
                        sendCommandTo(GetViewById(i), L"renderervars:changed", params.toStdWString());
                    }

#ifndef Q_OS_WIN
//                    for (auto i: GetViewsId()) {
//                        sendCommandTo(GetViewById(i), cmd, pData->get_Param());
//                    }
#endif

                    if ( themes().current().isSystem() && themes().current().isDark() != is_dark )
                        applyTheme(themes().current().id());
                }
            }


            return true;
        } else
        if ( !(cmd.find(L"open:template") == std::wstring::npos) ) {
            if ( pData->get_Param() == L"external" ) {
                static QByteArray _json_to_open;
                if ( _json_to_open.isEmpty() ) {
                    QString _templates_url{QString::fromStdWString(InputArgs::argument_value(L"--templates-url"))};
                    if ( _templates_url.isEmpty() )
                        _templates_url = "https://templates.onlyoffice.com/?desktop=true";

                    QJsonObject _json_obj{
                        {"portal", _templates_url},
                        {"entrypage", ""},
                        {"title", "Templates"}
                    };

                    _json_to_open = QJsonDocument(_json_obj).toJson(QJsonDocument::Compact);
                }

                mainWindow()->onPortalOpen(_json_to_open);
            }

            return true;
        } else
        if ( !(cmd.find(L"quickaccess:changed") == std::wstring::npos) ) {
            int sid = event->get_SenderId();
            map<int, CCefEventsGate *>::const_iterator it = m_receivers.find(sid);
            if ( it != m_receivers.cend() ) {
                std::wstring param = L"{\"quickaccesschanged\":" + pData->get_Param() + L"}";
                QMetaObject::invokeMethod(it->second, "onWebTitleChanged", Qt::QueuedConnection, Q_ARG(int, sid), Q_ARG(std::wstring, param));
            }
        } else
        if ( !(cmd.find(L"recent:pinned") == std::wstring::npos) ) {
            QJsonParseError jerror;
            QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(pData->get_Param()).toUtf8(), &jerror);

            if( jerror.error == QJsonParseError::NoError ) {
                QJsonObject objRoot = jdoc.object();
                SetRecentPin(objRoot["id"].toInt(), objRoot["pinned"].toBool(false));
            }
        }

        break; }

    case ASC_MENU_EVENT_TYPE_SSO_TOKEN: {
//        CAscSSOToken * pData = (CAscSSOToken *)_event->m_pData;
        return true; }

    case ASC_MENU_EVENT_TYPE_REPORTER_CREATE: {
        CPresenterWindow * reporterWindow = createReporterWindow(event->m_pData, event->get_SenderId());
#ifdef __linux
        reporterWindow->show(false);
#else
        reporterWindow->show(false);
        //reporterWindow->toggleBorderless(false);
#endif

//        RELEASEINTERFACE(event);
        break; }

    case ASC_MENU_EVENT_TYPE_REPORTER_END: {
        // close editor window
        CAscTypeId * pData = static_cast<CAscTypeId *>(event->m_pData);

        if ( !m_winsReporter.empty() && m_winsReporter.find(pData->get_Id()) != m_winsReporter.end() ) {
            AscAppManager::getInstance().DestroyCefView(pData->get_Id());
        }

//        RELEASEINTERFACE(event);
        break; }

    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO:
    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM: return true;

    case ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE: {
        event->AddRef();
        this->Apply(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_WINDOW_SHOW_CERTIFICATE: {
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
        CAscX509CertificateData * pData = reinterpret_cast<CAscX509CertificateData *>(event->m_pData);

        CDialogCertificateInfo _dialog(mainWindow(), pData->get_Data());
        _dialog.exec();
#endif
        return true;
    }
    case ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE: {
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
        m_private->selectSSLSertificate(event->get_SenderId());
#endif
        return true; }
    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE: {
        CAscDocumentOnSaveData * pData = reinterpret_cast<CAscDocumentOnSaveData *>(event->m_pData);

        if (pData->get_IsCancel()) {
            AscAppManager::cancelClose();
        }
        break; }
    case ASC_MENU_EVENT_TYPE_CEF_DESTROYWINDOW: {
        --m_countViews;

        if ( !m_winsReporter.empty() ) {
            std::map<int, CPresenterWindow *>::const_iterator switer = m_winsReporter.find(event->get_SenderId());

            if (switer != m_winsReporter.end() ) {
                CPresenterWindow * reporterWindow = switer->second;

                GET_REGISTRY_USER(reg_user)
                reg_user.setValue("repPosition", reporterWindow->normalGeometry());

                reporterWindow->geometry();
                delete reporterWindow, reporterWindow = nullptr;
                m_winsReporter.erase(switer);

                return true;
            }
        }

        if ( m_closeCount && --m_closeCount == 0 && !m_closeTarget.empty() ) {
            if ( m_closeTarget.find(L"http") != wstring::npos ) {
                Logout(m_closeTarget);
                m_closeTarget.clear();
            }
        } else
        if ( m_countViews == 1 && mainWindow() && mainWindow()->isAboutToClose() ) {        // if only start page exists
            emit aboutToQuit();
//            DestroyCefView(-1);
        }

        break;
    }

    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PLUGINS: {
        CAscSystemExternalPlugins * pData = static_cast<CAscSystemExternalPlugins *>(event->m_pData);
        QJsonObject _json_obj;
        QJsonParseError jerror;
        QJsonDocument jdoc;

        for (const CAscSystemExternalPlugins::CItem& item: pData->get_Items()) {
            _json_obj["name"] = QString::fromStdWString(item.name);
            _json_obj["id"] = QString::fromStdWString(item.id);
            _json_obj["url"] = QString::fromStdWString(item.url);

            if ( !item.nameLocale.empty() ) {
                jdoc = QJsonDocument::fromJson(QString::fromStdWString(item.nameLocale).toUtf8(), &jerror);

                if( jerror.error == QJsonParseError::NoError ) {
                    _json_obj["nameLocale"] = jdoc.object();
                }
            }

            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "panel:external", Utils::stringifyJson(_json_obj));
        }

        return true; }

    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PROCESS: {
        CAscExternalProcess * pData = (CAscExternalProcess *)event->m_pData;
        QStringList arguments;
        const vector<wstring>& srcArgs = pData->get_Arguments();

        for (auto & wstr: srcArgs)
            arguments.append(QString::fromStdWString(wstr));

        if (pData->get_Detached())
            QProcess::startDetached(QString::fromStdWString(pData->get_Program()), arguments, QString::fromStdWString(pData->get_WorkingDirectory()));
        else QProcess::execute(QString::fromStdWString(pData->get_Program()), arguments);

//        RELEASEINTERFACE(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG:{
        CAscSaveDialog * pData = (CAscSaveDialog *)event->m_pData;
        SKIP_EVENTS_QUEUE(std::bind(&CAscApplicationManagerWrapper::onDownloadSaveDialog, this, pData->get_FilePath(), pData->get_IdDownload()));
        return true;}

    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD: {
        CMainWindow * mw = mainWindow();
        if ( mw ) mw->onDocumentDownload(event->m_pData);
        return true;}

    case ASC_MENU_EVENT_TYPE_CEF_CHECK_KEYBOARD:
        CheckKeyboard();
        return true;

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE: {
        static_cast<CAscLocalOpenFileDialog *>(event->m_pData)->put_Filter(L"image");}

    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG: {
        event->AddRef();
        SKIP_EVENTS_QUEUE([event]{
            CEditorTools::getlocalfile(event);
            event->Release();
        });

        return true;}

    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_START:
    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_END:
    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_PLAYER_COMMAND: {
        CCefView * _cef = GetViewById(event->get_SenderId());
        if ( _cef ) {
            CCefViewWidgetImpl * _impl = _cef->GetWidgetImpl();

            if ( _impl ) {
                QCefView* pQCefView = static_cast<QCefView *>(_impl);

                if (ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_PLAYER_COMMAND == event->m_nType)
                    pQCefView->OnMediaPlayerCommand(static_cast<CAscExternalMediaPlayerCommand*>(event->m_pData));
                else if (ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_START == event->m_nType)
                    pQCefView->OnMediaStart(static_cast<CAscExternalMedia *>(event->m_pData));
                else
                    pQCefView->OnMediaEnd();
            }
        }

        return true;
    }

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN: {
        CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)event->m_pData;
        vector<wstring>& files = pData->get_Files();

        CEditorWindow * _editor;
        for (size_t i(files.size()); i --> 0;) {
            _editor = editorWindowFromUrl(QString::fromStdWString(files[i]));

            if ( _editor ) {
                files.erase(files.begin() + i);
/*#ifdef Q_OS_WIN
                SetForegroundWindow(_editor->handle());
#else*/
                _editor->activateWindow();
//#endif
            }
        }
    }

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN:
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN: {
        CAscLocalOpenFileRecent_Recover * data = (CAscLocalOpenFileRecent_Recover *)event->m_pData;
        CCefView * view = GetViewByRecentId(data->get_Id());
        if ( view ) {
            CEditorWindow * editor = editorWindowFromViewId(view->GetId());

            if ( editor ) {
/*#ifdef Q_OS_WIN
                SetForegroundWindow(editor->handle());
#else*/
                editor->activateWindow();
//#endif
                return true;
            }
        }

        break;}

//    case ASC_MENU_EVENT_TYPE_CEF_CREATETAB: {
//        break;}

    case ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE: {
        CCefView * pView = GetViewById(event->get_SenderId());
        if (NULL != pView && pView->GetType() == cvwtEditor) {
            CAscTabEditorType& data = *static_cast<CAscTabEditorType *>(event->m_pData);
            ((CCefViewEditor *)pView)->SetEditorType(AscEditorType(data.get_Type()));
        }
        break;
    }

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE: {
        CEditorTools::processLocalFileSaveAs(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENDIRECTORY_DIALOG: {
        CAscLocalOpenDirectoryDialog * data = static_cast<CAscLocalOpenDirectoryDialog *>(event->m_pData);
        std::wstring path = CEditorTools::getFolder(data->get_Path(), event->get_SenderId());

        data->put_Path(path);
        event->AddRef();

        AscAppManager::getInstance().Apply(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END: {
        AscAppManager::printData().init(event->get_SenderId(), (CAscPrintEnd *)event->m_pData);
        return false;
    }

    case ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN: {
        CAscKeyboardDown * data = static_cast<CAscKeyboardDown *>(event->m_pData);

        switch ( data->get_KeyCode() ) {
        case VK_F4:
            if ( data->get_IsAlt() ) {
                CEditorWindow * editor = editorWindowFromViewId(event->get_SenderId());
                if ( editor ) {
                    editor->closeWindow();
                    return true;
                } else
                if ( mainWindow()->holdView(event->get_SenderId()) ) {
                    mainWindow()->close();
                    return true;
                }
            }
            break;
        case 0x4f:  // Key_O
            if (data->get_IsCtrl()) {
                m_private->sendOpenFolderEvent(event->get_SenderId());
                return true;
            }
            break;
        default:
            break;
        }

        break;
    }

    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER:
    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE: {
        static int fs_view_id = -1;

        if ( !m_winsReporter.empty() &&
                m_winsReporter.find(event->m_nSenderId) != m_winsReporter.end() )
        {
            break;
        }

        if ( event->m_nType == ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER ) {
            if (  fs_view_id < 0 ) {
                fs_view_id = event->m_nSenderId;
            } else {
                int view_id = event->m_nSenderId;
                QTimer::singleShot(0, [view_id]{
                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                    pCommand->put_Command(L"editor:stopDemonstration");
                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                    pEvent->m_pData = pCommand;
                    AscAppManager::getInstance().GetViewById(view_id)->Apply(pEvent);
                });

                return true;
            }
        } else {
            if ( event->m_nSenderId == fs_view_id )
                fs_view_id = -1;
        }

        break;
    }

    default: break;
    }

    return false;
}

void CAscApplicationManagerWrapper::broadcastEvent(NSEditorApi::CAscCefMenuEvent * event)
{
    if ( m_pMainWindow ) {
        ADDREFINTERFACE(event);
        CCefEventsTransformer::OnEvent(m_pMainWindow, event);
    }

    for (const auto& it: m_receivers) {
        ADDREFINTERFACE(event);
        CCefEventsTransformer::OnEvent(it.second, event);
    }

//    RELEASEINTERFACE(event);
}

CAscApplicationManagerWrapper & CAscApplicationManagerWrapper::getInstance()
{
    static CAscApplicationManagerWrapper _manager;
    return _manager;
}

CAscApplicationManager * CAscApplicationManagerWrapper::createInstance()
{
    return new CAscApplicationManagerWrapper;
}

CMainWindow * CAscApplicationManagerWrapper::prepareMainWindow(const QRect& r)
{
    APP_CAST(_app);
    GET_REGISTRY_USER(reg_user);

    QRect _start_rect{r};
    if ( r.isEmpty() )
        _start_rect = reg_user.value("position").toRect();

    QPointer<QCefView> _startPanel = AscAppManager::createViewer(nullptr, CWindowBase::expectedContentSize(_start_rect));
    _startPanel->Create(&_app, cvwtSimple);
    _startPanel->setObjectName("startPanel");
	QColor c = m_themes->current().color(CTheme::ColorRole::ecrWindowBackground);
    _startPanel->SetBackgroundCefColor(uchar(c.red()), uchar(c.green()), uchar(c.blue()));
    //_startPanel->resize(_start_rect.width(), _start_rect.height());

    CMainWindow * _window = static_cast<CMainWindow*>(new CMainWindowImpl(_start_rect));
    _window->attachStartPanel(_startPanel);

    QString data_path;
#if defined(QT_DEBUG)
    data_path = reg_user.value("startpage").value<QString>();
#endif

    if ( data_path.isEmpty() )
        data_path = qApp->applicationDirPath() + "/index.html";

    QString additional = "?waitingloader=yes&lang=" + CLangater::getCurrentLangCode();
    QString _portal = reg_user.value("portal").value<QString>();
    if (!_portal.isEmpty()) {
        QString arg_portal = (additional.isEmpty() ? "?portal=" : "&portal=") + _portal;
        additional.append(arg_portal);
    }

#if defined(__OS_WIN_XP)
    additional.append("&osver=winxp");
#endif

    std::wstring start_path = ("file:///" + data_path + additional).toStdWString();
    _startPanel->GetCefView()->load(start_path);


    QObject::connect(_window, &CMainWindow::aboutToClose, this, &CAscApplicationManagerWrapper::onMainWindowClose);

    return _window;
}

void CAscApplicationManagerWrapper::handleInputCmd(const std::vector<wstring>& vargs)
{
    APP_CAST(_app);
    GET_REGISTRY_USER(reg_user);

    auto check_param = [] (const wstring& line, const wstring& param) {
        return line.find(param) == 0;
    };

    auto check_params = [] (const wstring& line, const std::vector<std::wstring>& params) {
        for (size_t i(0); i < params.size(); ++i) {
            if (line.find(params[i]) == 0)
                return int(i);
        }

        return -1;
    };

    QRect _start_rect = reg_user.value("position").toRect();

    const wstring prefix{L"--"};
    std::vector<std::wstring> arg_check_list{L"--review",L"--view",L"--edit"};
    std::vector<COpenOptions> list_failed;
//    bool open_in_new_window = std::find(vargs.begin(), vargs.end(), L"--force-use-tab") == std::end(vargs);
    bool open_in_new_window = _app.m_private->preferOpenEditorWindow() || (std::find(vargs.begin(), vargs.end(), L"--force-use-window") != std::end(vargs));
    std::vector<std::wstring> open_scheme{L"http://",L"https://"};
    std::wstring app_scheme = _app.GetExternalSchemeName();
    if ( !app_scheme.empty() ) {
        open_scheme.push_back(app_scheme);
    }
    std::wstring app_action = app_scheme + L"//action";
    std::wstring app_action_open = app_scheme + L"//open";
    std::wstring app_action_plugin = app_scheme + L"//action|install-plugin";
    std::vector<std::wstring> vec_window_actions;

    for (const auto& arg: vargs) {
        COpenOptions open_opts;
        open_opts.name = QCoreApplication::translate("CAscTabWidget", "Document");
        open_opts.srctype = AscEditorType::etUndefined;

        const size_t p = arg.find(prefix);
        if ( p == 0 ) {
            auto i = check_params(arg, arg_check_list);
            if ( !(i < 0) ) {
                auto c = arg_check_list[size_t(i)].size();
                if ( !(c < 0) )
                    open_opts.wurl = arg.substr(++c);

                open_in_new_window = true;
                open_opts.mode = i == 0 ? COpenOptions::eOpenMode::review :
                                    i == 1 ? COpenOptions::eOpenMode::view : COpenOptions::eOpenMode::edit;
            }else
            if ( check_param(arg, L"--new" ) ) {
                open_opts.srctype = etNewFile;
                open_opts.format = arg.rfind(L"cell") != wstring::npos ? AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX :
                                    arg.rfind(L"slide") != wstring::npos ? AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX :
                                    // arg.rfind(L"draw") != wstring::npos ? AVS_OFFICESTUDIO_FILE_DRAW_VSDX :
                                    arg.rfind(L"form") != wstring::npos ? AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF :
                            /*if ( line.rfind(L"word") != wstring::npos )*/ AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;

                open_opts.name = AscAppManager::newFileName(open_opts.format);
            } else continue;

//            if ( check_param(arg, L"single-window") )
//                in_new_window = true;
        } else
        if ( arg.rfind(app_action, 0) == 0 ) {
            // correct url from browsers
            std::wstring argScheme = arg;
            Utils::replaceAll(argScheme, L"%7C", L"|");
            if (argScheme[argScheme.length() - 1] == '/')
                argScheme.pop_back();

            if ( argScheme.rfind(app_action_plugin, 0) == 0 ) {
                std::wstring _plugin_name = argScheme.substr(app_action_plugin.size() + 1);
                if ( !_plugin_name.empty() ) {
                    _app.InstallPluginFromStore(_plugin_name);
                }
            } else {
                vec_window_actions.push_back(argScheme);
            }

            continue;
        } else
        if ( arg.rfind(app_action_open, 0) == 0 ) {
            std::wstring deep_link = arg;
            Utils::replaceAll(deep_link, L"%7C", L"|");
            open_opts.wurl = deep_link;
            open_opts.name = QString::fromStdWString(get_file_name_from_open_deeplink(deep_link));
        } else {
            open_opts.wurl = arg;
        }

        if (open_opts.srctype == AscEditorType::etUndefined) {
            QString str_url = QString::fromStdWString(open_opts.wurl);
            if ( CFileInspector::isLocalFile(str_url) ) {
#ifdef _WIN32
                str_url = Utils::replaceBackslash(str_url);
                open_opts.wurl = str_url.toStdWString();
#else
                QUrl url = QUrl::fromUserInput(str_url);
                if (!url.isValid()) {
                    QFileInfo info(str_url);
                    if (info.isFile())
                        url = QUrl::fromUserInput(info.absoluteFilePath());
                }
                if (url.isValid()) {
                    str_url = url.toLocalFile();
                    open_opts.wurl = str_url.toStdWString();
                }
#endif
            }
            if ( _app.m_private->bringEditorToFront(str_url) ) {
                continue;
            } else
            if ( CFileInspector::isLocalFile(str_url) ) {
                open_opts.srctype = etLocalFile;
#ifdef Q_OS_WIN
                int _error = _waccess(open_opts.wurl.c_str(), 0);
                auto _c_pos = open_opts.wurl.rfind(L"\\");
                if ( _c_pos == std::wstring::npos )
                    _c_pos = open_opts.wurl.rfind(L"/");
#else
                int _error = access(U_TO_UTF8(open_opts.wurl).c_str(), F_OK);
                auto _c_pos = open_opts.wurl.rfind(QString(QDir::separator()).toStdWString());
#endif
                if ( _error == 0 ) {
                    _error = CCefViewEditor::GetFileFormat(open_opts.wurl) == 0 ? EBADF : 0;
                }

                if ( _error == 0 ) {
                    if ( _c_pos != std::wstring::npos )
                        open_opts.name = QString::fromStdWString(open_opts.wurl.substr(++_c_pos));

//                    open_opts.srctype = etLocalFile;
                } else
                if ( _error == EBADF && !open_in_new_window ) {
                    list_failed.push_back({open_opts});
                    continue;
                } else {
                    /* file doesn't exists */
                    open_opts.srctype = etNewFile;
                    open_opts.format = open_opts.format = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
                    open_opts.name = AscAppManager::newFileName(open_opts.format);
                }
            } else
            if ( check_params(open_opts.wurl, open_scheme) < 0 )
                continue;
        }

        if ( open_in_new_window ) {
            bool isMaximized = false;
            _app.m_private->editorWindowGeometry(_start_rect, isMaximized, open_opts);
            open_opts.panel_size = CWindowBase::expectedContentSize(_start_rect, true);
            open_opts.parent_widget = COpenOptions::eWidgetType::window;
            if (CEditorWindow * editor_win = CEditorWindow::create(_start_rect, open_opts)) {
                editor_win->show(isMaximized);
                editor_win->bringToTop();

                _app.m_vecEditors.push_back(size_t(editor_win));
                if ( editor_win->isCustomWindowStyle() )
                    sendCommandTo(editor_win->mainView()->cef(), L"window:features",
                                  Utils::stringifyJson(QJsonObject{{"skiptoparea", TOOLBTN_HEIGHT},{"singlewindow",true}}).toStdWString());
            }
        } else {
            if ( !_app.m_pMainWindow ) {
                _app.m_pMainWindow = _app.prepareMainWindow(_start_rect);
                _app.m_pMainWindow->show(reg_user.value("maximized", WindowHelper::defaultWindowMaximizeState()).toBool());
            } else
            if (!_app.m_pMainWindow->isVisible() && !_app.m_pMainWindow->isSlideshowMode())
                _app.m_pMainWindow->show(_app.m_pMainWindow->windowState().testFlag(Qt::WindowMaximized));

            open_opts.panel_size = _app.m_pMainWindow->contentSize();
            open_opts.parent_widget = COpenOptions::eWidgetType::tab;
            if (CTabPanel * panel = CEditorTools::createEditorPanel(open_opts, _app.m_pMainWindow)) {
                _app.mainWindow()->attachEditor(panel);

                if (!_app.m_pMainWindow->isSlideshowMode()) {
                    QTimer::singleShot(100, &_app, [&]{
                        _app.mainWindow()->bringToTop();
                    });
                }
            }
        }
    }

    if ( !list_failed.empty() && !open_in_new_window ) {
        if ( !_app.m_pMainWindow ) {
            _app.m_pMainWindow = _app.prepareMainWindow(_start_rect);
            _app.m_pMainWindow->show(reg_user.value("maximized", WindowHelper::defaultWindowMaximizeState()).toBool());
        }

        for ( auto & o : list_failed ) {
            COpenOptions opts{o};
            opts.url = QString::fromStdWString(opts.wurl);

            _app.m_pMainWindow->doOpenLocalFile(opts);
        }
    }

    if ( !vec_window_actions.empty() ) {
        QTimer::singleShot(0, &_app, [vec_window_actions]{
            CAscApplicationManagerWrapper::getInstance().handleDeeplinkActions(vec_window_actions);
        });
    }
}

void CAscApplicationManagerWrapper::handleDeeplinkActions(const std::vector<std::wstring>& actions)
{
    std::wstring app_scheme = GetExternalSchemeName();
    if ( !app_scheme.empty() ) {
        const std::wstring app_action_panel = app_scheme + L"//action|panel|";

        for (const auto& a: actions) {
            if ( a.rfind(app_action_panel, 0) == 0 ) {
                std::wstring _action = a.substr(app_action_panel.size() - 6);

                gotoMainWindow();
                m_pMainWindow->handleWindowAction(_action);
            }
        }
    }
}

void CAscApplicationManagerWrapper::onDocumentReady(int uid)
{
#ifndef __OS_WIN_XP
    static bool runOnce = false;
    if (!runOnce) {
        runOnce = true;
        m_private->m_notificationSupported = CNotification::instance().init();
    }
#endif

#ifdef _UPDMODULE
    if (!m_pUpdateManager) {
        m_pUpdateManager = new CUpdateManager(this);
        m_pUpdateManager->setServiceLang();
        m_pUpdateManager->launchIntervalStartTimer();
    }
    if (uid < 0) {
        QTimer::singleShot(50, this, [=]() {
            m_pUpdateManager->refreshStartPage();
        });
    }
#endif

#ifdef _WIN32
    Association::instance().chekForAssociations(uid);
#endif

    if (uid > -1) {
        if (printData().printerCapabilitiesReady())
            AscAppManager::sendCommandTo(GetViewById(uid), L"printer:config", printData().getPrinterCapabilitiesJson().toStdWString());

        CAscTabData *data = nullptr;
        if (CEditorWindow *editor = editorWindowFromViewId(uid)) {
            data = editor->mainView()->data();
        } else
            if (mainWindow() && mainWindow()->holdView(uid)) {
                int indx = mainWindow()->tabWidget()->tabIndexByView(uid);
                if (indx > -1)
                    data = mainWindow()->tabWidget()->panel(indx)->data();
            }

        if (data && data->isLocal() && !data->url().empty()) {
            Utils::addToRecent(data->url());
        }
    }

    static bool check_printers = false;
    if (!check_printers) {
        check_printers = true;

        printData().queryPrinterCapabilitiesAsync([=](const QString &json) {
            // qDebug().noquote() << json;
            if (mainWindow()) {
                CAscTabWidget *tabs = mainWindow()->tabWidget();
                for (int i = 0; i < tabs->count(); i++) {
                    if (tabs->panel(i)->isReady())
                        AscAppManager::sendCommandTo(tabs->panel(i)->cef(), L"printer:config", json.toStdWString());
                }
            }
            foreach (auto ptr, m_vecEditors) {
                CEditorWindow *e = reinterpret_cast<CEditorWindow*>(ptr);
                if (e->mainView()->isReady())
                    AscAppManager::sendCommandTo(e->mainView()->cef(), L"printer:config", json.toStdWString());
            }
        });
    }
}

void CAscApplicationManagerWrapper::startApp()
{
    APP_CAST(_app);
    GET_REGISTRY_USER(reg_user)

//    QRect _start_rect = reg_user.value("position").toRect();
    bool _is_maximized = reg_user.value("maximized", WindowHelper::defaultWindowMaximizeState()).toBool();

#if 0
    CMainWindow * _window = createMainWindow(_start_rect);

#ifdef __linux
    _window->show();
    if ( _is_maximized )
        _window->slot_windowChangeState(Qt::WindowMaximized);
#else
    _window->show(_is_maximized);
//    _window->toggleBorderless(_is_maximized);

    if ( _is_maximized ) {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(_window->hWnd, &wp)) {
            wp.rcNormalPosition = {_start_rect.x(), _start_rect.y(), _start_rect.right(), _start_rect.bottom()};

            SetWindowPlacement(_window->hWnd, &wp);
        }
    }
#endif

    QStringList * _files = Utils::getInputFiles(g_cmdArgs);
    if ( _files ) {
        _window->doOpenLocalFiles(*_files);
        if ( getInstance().m_private->allowedCreateLocalFile() ) {
            QRegularExpression re("^--new:(word|cell|slide)");
            QStringListIterator i(*_files);
            while (i.hasNext()) {
                QString n = i.next();
                if ( n.startsWith("--new:") ) {
                    QRegularExpressionMatch match = re.match(n);
                    if ( match.hasMatch() ) {
                        int _format = AVS_OFFICESTUDIO_FILE_UNKNOWN;
                        if ( match.captured(1) == "word" ) _format = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX; else
                        if ( match.captured(1) == "cell" ) _format = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX; else
                        if ( match.captured(1) == "slide" ) _format = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;

                        _window->createLocalFile(AscAppManager::newFileName(_format), _format);
                    }
                }
            }
        }
    }
#endif

    std::vector<std::wstring> in_args{InputArgs::arguments()};
    bool open_in_new_window = std::find(in_args.begin(), in_args.end(), L"--force-use-window") != std::end(in_args);
    bool files_in_args = std::find_if(in_args.begin(), in_args.end(),
                                     [](const std::wstring& arg){
                                            return (arg.rfind(L"--review", 0) != std::string::npos) || (arg.rfind(L"--view", 0) != std::string::npos) ||
                                                        (arg.rfind(L"--edit", 0) != std::string::npos) || arg.rfind(L"--", 0 == std::string::npos);
                                        }) != std::end(in_args);
    if ( !files_in_args && open_in_new_window ) {
        in_args.push_back(L"--new:word");
    }

    handleInputCmd(in_args);
    if ( _app.m_vecEditors.empty() && !_app.m_pMainWindow ) {
        _app.m_pMainWindow = _app.prepareMainWindow();
        _app.m_pMainWindow->show(_is_maximized);
    }

    QObject::connect(CExistanceController::getInstance(), &CExistanceController::checked, [] (const QString& name, int uid, bool exists) {
        if ( !exists ) {
            QJsonObject _json_obj{{QString::number(uid), exists}};
            QString json = QJsonDocument(_json_obj).toJson(QJsonDocument::Compact);

            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"files:checked", json.toStdWString());
        }
    });
}

void CAscApplicationManagerWrapper::initializeApp()
{
    APP_CAST(_app);
    _app.m_private->initializeApp();

    if ( AscAppManager::IsUseSystemScaling() ) {
        AscAppManager::setUserSettings(L"force-scale", L"default");
        AscAppManager::setUserSettings(L"system-scale", L"1");
    }

    SingleApplication * app = static_cast<SingleApplication *>(QCoreApplication::instance());
    connect(app, &SingleApplication::receivedMessage, [](const QString &args) {
        std::vector<std::wstring> vec_inargs;
        foreach (auto arg, args.split(";")) {
            if ( !arg.isEmpty() )
                vec_inargs.push_back(arg.toStdWString());
        }
        if ( !vec_inargs.empty() )
            handleInputCmd(vec_inargs);
        else
            gotoMainWindow();
    });

    /* prevent drawing of focus rectangle on a button */
//    QApplication::setStyle(new CStyleTweaks);

    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    reg_user.setFallbacksEnabled(false);

    if ( InputArgs::contains(L"--system-title-bar") )
        reg_user.setValue("titlebar", "system");
    else
    if ( InputArgs::contains(L"--custom-title-bar") || !reg_user.contains("titlebar") )
        reg_user.setValue("titlebar", "custom");

    // read installation time and clean cash folders if expired
    if ( reg_system.contains("timestamp") ) {
        QString user_data_path = Utils::getUserPath() + APP_DATA_PATH;

        QDateTime time_install, time_clear;
        time_install.setMSecsSinceEpoch(reg_system.value("timestamp", 0).toULongLong());

        bool clean = true;
        if ( reg_user.contains("timestamp") ) {
            time_clear.setMSecsSinceEpoch(reg_user.value("timestamp", 0).toULongLong());

            clean = time_install > time_clear;
        }

        if ( clean ) {
            reg_user.setValue("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
            QDir(user_data_path + "/fonts").removeRecursively();
        }
    }

    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1, ":styles/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1_25, ":styles@1.25x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1_5, ":styles@1.5x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1_75, ":styles@1.75x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_2, ":styles@2x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_2_25, ":styles@2.25x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_2_5, ":styles@2.5x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_2_75, ":styles@2.75x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_3, ":styles@3x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_3_5, ":styles@3.5x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_4, ":styles@4x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_4_5, ":styles@4.5x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_5, ":styles@5x/styles.qss");

    _app.m_private->applyStylesheets();

    // TODO: merge stylesheets and apply for the whole app
    QString css{Utils::readStylesheets(":styles/styles.qss")};
#ifdef __linux__
    css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
    qApp->setStyleSheet(css.arg(GetColorQValueByRole(ecrWindowBackground),
                                GetColorQValueByRole(ecrTextNormal),
                                GetColorQValueByRole(ecrButtonHoverBackground),
                                GetColorQValueByRole(ecrButtonPressedBackground),
                                GetColorQValueByRole(ecrButtonBackground),
                                GetColorQValueByRole(ecrTabDivider),
                                GetColorQValueByRole(ecrButtonBackgroundActive),
                                GetColorQValueByRole(ecrToolTipText))
                           .arg(GetColorQValueByRole(ecrToolTipBorder),
                                GetColorQValueByRole(ecrToolTipBackground),
                                GetColorQValueByRole(ecrMenuBackground),
                                GetColorQValueByRole(ecrMenuBorder),
                                GetColorQValueByRole(ecrMenuItemHoverBackground),
                                GetColorQValueByRole(ecrMenuText),
                                GetColorQValueByRole(ecrMenuTextItemHover),
                                GetColorQValueByRole(ecrMenuTextItemDisabled),
                                GetColorQValueByRole(ecrMenuSeparator)));

    // Font
    QFont mainFont = QApplication::font();
    mainFont.setStyleStrategy( QFont::PreferAntialias );
    QApplication::setFont( mainFont );

    EditorJSVariables::init();

    wstring wparams{InputArgs::webapps_params()};
    if ( !wparams.empty() ) wparams += L"&";
    wparams += QString("lang=%1&username=%3&location=%2").arg(CLangater::getCurrentLangCode(), Utils::systemLocationCode()).toStdWString();
    wstring user_name = QString(QUrl::toPercentEncoding(QString::fromStdWString(Utils::appUserName()))).toStdWString();

    wparams.replace(wparams.find(L"%3"), 2, user_name);
    InputArgs::set_webapps_params(wparams);

    AscAppManager::getInstance().InitAdditionalEditorParams(wparams);
//    AscAppManager::getInstance().applyTheme(themes().current().id(), true);

    QJsonArray local_themes_array = themes().localThemesToJson();
    if ( !local_themes_array.isEmpty() )
        EditorJSVariables::setVariable("localthemes", local_themes_array);

#if !defined(__OS_WIN_XP)
    bool _is_rtl = InputArgs::contains(L"--text-direction") ? InputArgs::argument_value(L"--text-direction") == L"rtl" :
                       CLangater::isRtlLanguage(CLangater::getCurrentLangCode());
#else
    const bool _is_rtl = false;
#endif
    AscAppManager::setRtlEnabled(_is_rtl);
    EditorJSVariables::setVariable("rtl", _is_rtl ? "yes" : "no");

    EditorJSVariables::setVariable("lang", CLangater::getCurrentLangCode());

    std::vector<std::pair<std::string, std::string>> layouts = _app.GetKeyboardLayoutList();
    QJsonObject kbLangs;
    for (const auto &lut : layouts) {
        kbLangs.insert(QString::fromStdString(lut.first), QString::fromStdString(lut.second));
    }
    QJsonObject _ls_ = {{"langs", kbLangs}};
    EditorJSVariables::applyVariable("keyboard", _ls_);
    EditorJSVariables::applyVariable("theme", {
                                        {"type", _app.m_themes->current().stype()},
                                        {"id", QString::fromStdWString(_app.m_themes->current().id())},
                                        {"addlocal", "on"}
// #ifndef Q_OS_LINUX
                                        ,{"system", _app.m_themes->isSystemSchemeDark() ? "dark" : "light"}
// #else
//                                         ,{"system", "disabled"}
// #endif
                                     });

    AscAppManager::getInstance().m_oSettings.macroses_support = reg_system.value("macrosDisabled", true).toBool();
    AscAppManager::getInstance().m_oSettings.plugins_support = reg_system.value("pluginsDisabled", true).toBool();
}

CPresenterWindow * CAscApplicationManagerWrapper::createReporterWindow(void * data, int parentid)
{
//    QMutexLocker locker( &m_oMutex );

    CAscReporterCreate * pData = (CAscReporterCreate *)data;
    CAscReporterData * pCreateData = reinterpret_cast<CAscReporterData *>(pData->get_Data());
    pData->put_Data(NULL);

    QString _doc_name;
    QWindow *wnd = nullptr;
    if ( m_pMainWindow && m_pMainWindow->holdView(parentid) ) {
        _doc_name = m_pMainWindow->documentName(parentid);
        wnd = m_pMainWindow->windowHandle();
    } else {
        CEditorWindow * _window = editorWindowFromViewId(parentid);

        if ( _window ) {
            _doc_name = _window->documentName();
            wnd = _window->windowHandle();
        }
    }

    QScreen *scr = (wnd && wnd->screen()) ? wnd->screen() : QApplication::primaryScreen();
    QRect _scr_rect = scr->availableGeometry();
    int _scr_dpi_ratio = Utils::getScreenDpiRatio(_scr_rect.topLeft());

    GET_REGISTRY_USER(reg_user)

    QRect _default_rect{QPoint(0,0), QSize(1000,700) * _scr_dpi_ratio};
    QRect _saved_rect = reg_user.value("repPosition", _default_rect).toRect();
    if ( _scr_rect.width() < _saved_rect.width() )
        _saved_rect.setWidth(_scr_rect.width() - 100 * _scr_dpi_ratio);

    if ( _scr_rect.height() < _saved_rect.height() )
        _saved_rect.setHeight(_scr_rect.height() - 100 * _scr_dpi_ratio);

    QRect _window_rect{QPoint(0,0), _saved_rect.size()};
    _window_rect.moveCenter(_scr_rect.center());

    QCefView * pView = createViewer(nullptr, CWindowBase::expectedContentSize(_window_rect));
    pView->CreateReporter(this, pCreateData);

    CPresenterWindow * reporterWindow = new CPresenterWindow(_window_rect, tr("Presenter View") + " - " + _doc_name, pView);
    m_winsReporter[pView->GetCefView()->GetId()] = reporterWindow;

//    QTimer::singleShot(5000, [=]{
//        ::SetForegroundWindow((HWND)_window->handle());
//        ::FlashWindow((HWND)_window->handle(), TRUE);
//    });

    return reporterWindow;
}

void CAscApplicationManagerWrapper::gotoMainWindow(size_t src)
{
    APP_CAST(_app)

    if ( !_app.m_pMainWindow ) {
        GET_REGISTRY_USER(reg_user)

        QRect _start_rect;
        if ( src ) {
            const CEditorWindow & _editor = *reinterpret_cast<CEditorWindow *>(src);
            _start_rect = _editor.geometry().translated(QPoint(50,50) * _editor.scaling());
        }

        _app.m_pMainWindow = _app.prepareMainWindow(_start_rect);
        _app.m_pMainWindow->show(reg_user.value("maximized", WindowHelper::defaultWindowMaximizeState()).toBool());
    }

    if ( !_app.m_pMainWindow->isVisible() ) {
        if (_app.m_pMainWindow->isSlideshowMode())
            _app.m_pMainWindow->onFullScreen(-1, false);
        _app.m_pMainWindow->show(mainWindow()->isMaximized());
    }

//    _app.m_pMainWindow->bringToTop();
    QTimer::singleShot(0, &_app, [](){
        AscAppManager::mainWindow()->bringToTop();
    });
}

void CAscApplicationManagerWrapper::closeAppWindows()
{
    APP_CAST(_app)

    vector<size_t>::const_iterator it = _app.m_vecEditors.begin();
    while ( it != _app.m_vecEditors.end() ) {
        _app.closeQueue().enter(sWinTag{CLOSE_QUEUE_WIN_TYPE_EDITOR, size_t(*it)});
        it++;
    }

    if ( _app.m_pMainWindow && _app.m_pMainWindow->isVisible() ) {
        _app.closeQueue().enter(sWinTag{CLOSE_QUEUE_WIN_TYPE_MAIN, size_t(_app.m_pMainWindow)});
    }
}

void CAscApplicationManagerWrapper::launchAppClose()
{
    if ( canAppClose() ) {
        if ( m_countViews > 1 ) {
            /* close all editors windows */
            if ( !m_vecEditors.empty() ) {
                vector<size_t>::const_iterator it = m_vecEditors.begin();
                if ( it != m_vecEditors.end() ) {
                    CEditorWindow * _editor = reinterpret_cast<CEditorWindow *>(*it);

                    if ( _editor && _editor->closeWindow() == MODAL_RESULT_CANCEL )
                        AscAppManager::cancelClose();
                }
            } else {
                if ( AscAppManager::mainWindow()->tabCloseRequest() == MODAL_RESULT_CANCEL )
                    AscAppManager::cancelClose();
            }
        } else {
            emit aboutToQuit();
            QTimer::singleShot(0, this, [=]() {
                DestroyCefView(-1);
            });
        }
    } else {
        cancelClose();
    }
}

void CAscApplicationManagerWrapper::closeEditorWindow(const size_t p)
{
    APP_CAST(_app)

    if ( p ) {
#if defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ < 9
        vector<size_t>::iterator
#else
        vector<size_t>::const_iterator
#endif
        it = _app.m_vecEditors.begin();
        while ( it != _app.m_vecEditors.end() ) {
            if ( *it == p /*&& !_app.m_vecEditors.empty()*/ ) {
                CEditorWindow * _w = reinterpret_cast<CEditorWindow *>(*it);

                AscAppManager::unbindReceiver(static_cast<const CCefEventsGate *>(_w->receiver()));

                delete _w, _w = nullptr;

                it = _app.m_vecEditors.erase(it);
                break;
            } else ++it;
        }

        if ( _app.m_vecEditors.empty() ) {
            if ( !_app.m_pMainWindow || !_app.m_pMainWindow->isVisible() ) {
                if ( _app.m_closeTarget.empty() ) {
                    QTimer::singleShot(0, &_app, &CAscApplicationManagerWrapper::launchAppClose);
                }
            }
        }
    }
}

void CAscApplicationManagerWrapper::pinWindowToTab(CEditorWindow *editor, bool by_position)
{
    CTabPanel * tabpanel = editor->releaseEditorView();

    QJsonObject json_opts{{"widgetType","tab"}, {"captionHeight",0}};
    tabpanel->cef()->SetParentWidgetInfo(Utils::stringifyJson(json_opts).toStdWString());

    if (by_position) {
        CAscApplicationManagerWrapper::mainWindow()->attachEditor(tabpanel, QCursor::pos());
    } else {
        CAscApplicationManagerWrapper::mainWindow()->attachEditor(tabpanel, -1);
    }
    CAscApplicationManagerWrapper::closeEditorWindow(size_t(editor));

    AscAppManager::sendCommandTo(tabpanel->cef(), L"window:features",
              Utils::stringifyJson(QJsonObject{{"skiptoparea", 0},{"singlewindow",false}}).toStdWString());
    CAscApplicationManagerWrapper::mainWindow()->bringToTop();

    QTimer::singleShot(100, []{
        CAscApplicationManagerWrapper::mainWindow()->focus();});
}

CMainWindow * CAscApplicationManagerWrapper::mainWindowFromViewId(int uid) const
{
    return m_pMainWindow && m_pMainWindow->holdView(uid) ? m_pMainWindow : nullptr;
}

CEditorWindow * CAscApplicationManagerWrapper::editorWindowFromViewId(int uid) const
{
    CEditorWindow * _window = nullptr;

    for (auto const& w : m_vecEditors) {
        _window = reinterpret_cast<CEditorWindow *>(w);

        if ( _window->holdView(uid) )
            return _window;
    }

    return nullptr;
}

CEditorWindow * CAscApplicationManagerWrapper::editorWindowFromUrl(const QString& url) const
{
    CEditorWindow * _window = nullptr;

    for (auto const& w : m_vecEditors) {
        _window = reinterpret_cast<CEditorWindow *>(w);

        if ( _window->holdView(url.toStdWString()) )
            return _window;
    }

    return nullptr;
}

ParentHandle CAscApplicationManagerWrapper::windowHandleFromId(int id)
{
    APP_CAST(_app);

    CMainWindow * w = _app.mainWindowFromViewId(id);
    if ( w ) return w->handle();
    else {
        CEditorWindow * e = _app.editorWindowFromViewId(id);
        if ( e ) return e->handle();
    }

    return nullptr;
}

namespace Drop {
    const int drop_timeout = 300;
    auto callback_to_attach(const CEditorWindow * editor) -> void {
        if ( editor ) {
            AscAppManager::pinWindowToTab(const_cast<CEditorWindow*>(editor));
        }
    }


    size_t drop_handle;
    auto validate_drop(size_t handle, const QPoint& pt) -> void {
        CMainWindow * main_window = CAscApplicationManagerWrapper::mainWindow();
        if ( main_window && main_window->isVisible() && !main_window->isMinimized() ) {
            drop_handle = handle;

            static QPoint last_cursor_pos;
            static QTimer * drop_timer = nullptr;
            if ( !drop_timer ) {
                drop_timer = new QTimer;
                QObject::connect(qApp, &QCoreApplication::aboutToQuit, drop_timer, &QTimer::deleteLater);
                QObject::connect(drop_timer, &QTimer::timeout, []{
                    CMainWindow * main_window = CAscApplicationManagerWrapper::mainWindow();
                    QPoint current_cursor = QCursor::pos();
                    if ( main_window->canPinTabAtPoint(current_cursor) ) {
                        if ( current_cursor == last_cursor_pos ) {
                            drop_timer->stop();

                            if ( WindowHelper::isLeftButtonPressed() )
                                callback_to_attach(CAscApplicationManagerWrapper::editorWindowFromHandle(drop_handle) );
                        } else {
                            last_cursor_pos = current_cursor;
                        }
                    } else {
                        drop_timer->stop();
                    }
                });
            }

            if ( main_window->canPinTabAtPoint(pt) ) {
                if ( !drop_timer->isActive() )
                    drop_timer->start(drop_timeout);

                last_cursor_pos = QCursor::pos();
            } else
            if ( drop_timer->isActive() )
                drop_timer->stop();
        }
    }
}

const CEditorWindow * CAscApplicationManagerWrapper::editorWindowFromHandle(size_t handle)
{
    APP_CAST(_app)

    for (auto const& w : _app.m_vecEditors) {
        CEditorWindow * e = reinterpret_cast<CEditorWindow *>(w);

        if ( (size_t)e->handle() == handle ) {
            return e;
        }
    }

    return nullptr;
}


void CAscApplicationManagerWrapper::editorWindowMoving(const size_t h, const QPoint& pt)
{
#if 1
    Drop::validate_drop(h,pt);
#else
    APP_CAST(_app)

    if ( _app.m_vecWindows.size() > 0 ) {
        CMainWindow * _main_window = reinterpret_cast<CMainWindow *>(_app.m_vecWindows.at(0));

        if ( _main_window && _main_window->pointInTabs(pt) ) {
            CEditorWindow * editor_win = nullptr;
            for (auto const& w : _app.m_vecEditors) {
                CEditorWindow * _e = reinterpret_cast<CEditorWindow *>(w);

                if ( (size_t)_e->handle() == h ) {
                    editor_win = _e;
                    break;
                }
            }

            if ( editor_win ) {
                SKIP_EVENTS_QUEUE([=]{
                    _main_window->attachEditor(tabpanel);

                    closeEditorWindow(size_t(editor_win));
                });
            }
        }
    }
#endif
}

CMainWindow * CAscApplicationManagerWrapper::mainWindow()
{
    APP_CAST(_app);
    return _app.m_pMainWindow;
}

void CAscApplicationManagerWrapper::sendCommandTo(QCefView * target, const QString& cmd, const QString& args)
{
    sendCommandTo(target ? target->GetCefView() : nullptr, cmd.toStdWString(), args.toStdWString() );
}

void CAscApplicationManagerWrapper::sendCommandTo(CCefView * target, const wstring& cmd, const wstring& args)
{
    CAscExecCommandJS * pCommand = new CAscExecCommandJS;
    pCommand->put_Command(cmd);
    if ( !args.empty() )
        pCommand->put_Param(args);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    if ( target ) {
        if ( target->GetType() == cvwtEditor )
            pCommand->put_FrameName(L"frameEditor");

        target->Apply(pEvent);
    } else AscAppManager::getInstance().SetEventToAllMainWindows(pEvent);
}

void CAscApplicationManagerWrapper::sendCommandToAllEditors(const std::wstring& cmd, const std::wstring& args)
{
    APP_CAST(_app);

    CCefView * target;
    for ( auto i : _app.GetViewsId() ) {
        target = _app.GetViewById(i);

//        if ( target->GetType() == cvwtEditor ) {
            sendCommandTo(target, cmd, args);
//        }
    }
}

void CAscApplicationManagerWrapper::sendEvent(int type, void * data)
{
    CAscMenuEvent * pEvent = new CAscMenuEvent(type);
    pEvent->m_pData = static_cast<IMenuEventDataBase *>(data);

    AscAppManager::getInstance().Apply(pEvent);

//    delete pEvent;
}

QString CAscApplicationManagerWrapper::getWindowStylesheets(double dpi)
{
    CScalingFactor f = dpi > 4.5 ? CScalingFactor::SCALING_FACTOR_5 :
                       dpi > 4.0 ? CScalingFactor::SCALING_FACTOR_4_5 :
                       dpi > 3.5 ? CScalingFactor::SCALING_FACTOR_4 :
                       dpi > 3.0 ? CScalingFactor::SCALING_FACTOR_3_5 :
                       dpi > 2.75 ? CScalingFactor::SCALING_FACTOR_3 :
                       dpi > 2.5 ? CScalingFactor::SCALING_FACTOR_2_75 :
                       dpi > 2.25 ? CScalingFactor::SCALING_FACTOR_2_5 :
                       dpi > 2.0 ? CScalingFactor::SCALING_FACTOR_2_25 :
                       dpi > 1.75 ? CScalingFactor::SCALING_FACTOR_2 :
                       dpi > 1.5 ? CScalingFactor::SCALING_FACTOR_1_75 :
                       dpi > 1.25 ? CScalingFactor::SCALING_FACTOR_1_5 :
                       dpi > 1 ? CScalingFactor::SCALING_FACTOR_1_25 : CScalingFactor::SCALING_FACTOR_1;
    return getWindowStylesheets(f);
}

QString CAscApplicationManagerWrapper::getWindowStylesheets(CScalingFactor factor)
{
    APP_CAST(_app);

    QString _out = Utils::readStylesheets(_app.m_mapStyles[CScalingFactor::SCALING_FACTOR_1]);
    _out = _out.arg(GetColorQValueByRole(ecrWindowBackground),
                    GetColorQValueByRole(ecrTextNormal),
                    GetColorQValueByRole(ecrButtonHoverBackground),
                    GetColorQValueByRole(ecrButtonPressedBackground),
                    GetColorQValueByRole(ecrButtonBackground),
                    GetColorQValueByRole(ecrTabDivider),
                    GetColorQValueByRole(ecrButtonBackgroundActive),
                    GetColorQValueByRole(ecrToolTipText),
                    GetColorQValueByRole(ecrToolTipBorder))
               .arg(GetColorQValueByRole(ecrToolTipBackground),
                    GetColorQValueByRole(ecrMenuBackground),
                    GetColorQValueByRole(ecrMenuBorder),
                    GetColorQValueByRole(ecrMenuItemHoverBackground),
                    GetColorQValueByRole(ecrMenuText),
                    GetColorQValueByRole(ecrMenuTextItemHover),
                    GetColorQValueByRole(ecrMenuTextItemDisabled),
                    GetColorQValueByRole(ecrMenuSeparator));
//    _out.append(Utils::readStylesheets(":/themes/theme-contrast-dark.qss"));
    if ( factor != CScalingFactor::SCALING_FACTOR_1 )
        _out.append(Utils::readStylesheets(_app.m_mapStyles[factor]));

    return _out;
}

bool CAscApplicationManagerWrapper::event(QEvent *event)
{
    if ( event->type() == CTabUndockEvent::type() ) {
        CMainWindow * _main_window = mainWindow();
        if ( _main_window ) {
            CTabUndockEvent * e = static_cast<CTabUndockEvent *>(event);

            CTabPanel * _editor = nullptr;
            if ( e->panel() ) {
                _editor = static_cast<CTabPanel *>(e->panel());
            } else
            if ( !(e->index() < 0) ) {
                _editor = static_cast<CTabPanel *>(_main_window->editor(e->index()));
            }

            if ( _editor ) {
//                _editor->setParent(nullptr);
                e->accept();
//                SKIP_EVENTS_QUEUE([=]{
                    if ( _main_window ) {
                        QJsonObject json_opts{{"widgetType","window"}, {"captionHeight",TOOLBTN_HEIGHT}};
                        _editor->cef()->SetParentWidgetInfo(Utils::stringifyJson(json_opts).toStdWString());

                        QRect rect = _main_window->windowState().testFlag(Qt::WindowMaximized) ?
                                     _main_window->normalGeometry() : _main_window->windowRect();

                        CEditorWindow * editor_win = new CEditorWindow(rect.translated(QPoint(50,50)), _editor);
                        editor_win->undock(_main_window->isMaximized());

                        m_vecEditors.push_back( size_t(editor_win) );
                        if ( editor_win->isCustomWindowStyle() )
                            sendCommandTo(_editor->cef(), L"window:features",
                                    Utils::stringifyJson(QJsonObject{{"skiptoparea", TOOLBTN_HEIGHT},{"singlewindow",true}}).toStdWString());
                    }
//                });
            }
        }

        return true;
    }

    return QObject::event(event);
}

void CAscApplicationManagerWrapper::setRtlEnabled(bool state)
{
    if (m_rtlEnabled != state) {
        m_rtlEnabled = state;
        Qt::LayoutDirection direct = m_rtlEnabled ? Qt::RightToLeft : Qt::LeftToRight;
        APP_CAST(_app);
        if (_app.m_pMainWindow)
            _app.m_pMainWindow->setLayoutDirection(direct);
        for (auto const &r : _app.m_winsReporter)
            r.second->setLayoutDirection(direct);
        for (auto const &e : _app.m_vecEditors) {
            CEditorWindow *editor = reinterpret_cast<CEditorWindow*>(e);
            editor->setLayoutDirection(direct);
        }
    }
}

bool CAscApplicationManagerWrapper::isRtlEnabled()
{
    return m_rtlEnabled;
}

bool CAscApplicationManagerWrapper::notificationSupported()
{
    APP_CAST(_app);
    return _app.m_private->m_notificationSupported;
}

bool CAscApplicationManagerWrapper::applySettings(const wstring& wstrjson)
{
    QJsonParseError jerror;
    QByteArray stringdata = QString::fromStdWString(wstrjson).toUtf8();
    QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        GET_REGISTRY_USER(_reg_user)
        _reg_user.setValue("appdata", stringdata.toBase64());

        QJsonObject objRoot = jdoc.object();

        QString _user_newname = objRoot["username"].toString();
        if ( _user_newname.isEmpty() )
            _user_newname = QString::fromStdWString(Utils::systemUserName());

        QString _lang_id = CLangater::getCurrentLangCode();
        if ( objRoot.contains("langid") ) {
            QString l = objRoot.value("langid").toString();
            if ( _lang_id != l ) {
                bool direction_changed = (CLangater::isRtlLanguage(_lang_id) != CLangater::isRtlLanguage(l));
                _lang_id = l;

                _reg_user.setValue("locale", _lang_id);

                QJsonObject _json_obj{{"lang", _lang_id}};
                AscAppManager::getInstance().UpdatePlugins(Utils::stringifyJson(_json_obj).toStdWString());
                if (!direction_changed)
                    CLangater::reloadTranslations(_lang_id);
#ifdef _UPDMODULE
                if (m_pUpdateManager) {
                    m_pUpdateManager->setServiceLang(_lang_id);
                    m_pUpdateManager->refreshStartPage();
                }
#endif
            }
        }

        if ( objRoot.contains("uiscaling") ) {
#ifndef _CAN_SCALE_IMMEDIATELY
            m_private->uiscaling = Scaling::scalingToFactor(objRoot["uiscaling"].toString());
#else
            const wstring sets = Scaling::scalingToFactor(objRoot["uiscaling"].toString());

            setUserSettings(L"system-scale", sets != L"0" ? L"0" : L"1");
            setUserSettings(L"force-scale", sets == L"0" ? L"default" : sets);
            m_pMainWindow->updateScaling();

            CEditorWindow * _editor = nullptr;
            foreach ( auto const& e, m_vecEditors ) {
                _editor = reinterpret_cast<CEditorWindow *>(e);
                _editor->updateScaling();
            }

            for (auto const& r : m_winsReporter) {
                r.second->updateScaling();
            }
#endif
        }

        if ( objRoot.contains("spellcheckdetect") ) {
            setUserSettings(L"spell-check-input-mode", objRoot["spellcheckdetect"].toString() == "off" ? L"0" : L"default");
        }

        wstring params = QString("lang=%1&username=%3&location=%2")
                            .arg(_lang_id, Utils::systemLocationCode(), QUrl::toPercentEncoding(_user_newname)).toStdWString();

        if ( objRoot["docopenmode"].toString() == "view" ) {
            params.append(L"&mode=view");
        }


        InputArgs::set_webapps_params(params);
        AscAppManager::getInstance().InitAdditionalEditorParams( params );

        if ( objRoot.contains("uitheme") ) {
            applyTheme(objRoot["uitheme"].toString().toStdWString());
        }

        if ( objRoot.contains("editorwindowmode") ) {
            m_private->m_openEditorWindow = objRoot["editorwindowmode"].toBool();
            _reg_user.setValue("editorWindowMode", m_private->m_openEditorWindow);
        }

        if ( objRoot.contains("usegpu") ) {
            bool use_gpu = objRoot["usegpu"].toBool(true);
            setUserSettings(L"disable-gpu", use_gpu ? L"0" : L"1");
        }

#ifdef _UPDMODULE
        if ( objRoot.contains("autoupdatemode") ) {
            if (m_pUpdateManager)
                m_pUpdateManager->setNewUpdateSetting(objRoot["autoupdatemode"].toString());
        }
#endif
        if (objRoot.contains("restart") && objRoot["restart"].toBool()) {
            QTimer::singleShot(500, this, [=]() {
                if (MODAL_RESULT_YES == CMessage::showMessage(mainWindow(), tr("You must restart the application for the settings to take effect."),
                                                              MsgType::MSG_INFO, MsgBtns::mbYesDefNo)) {
                    m_private.get()->m_needRestart = true;
                    AscAppManager::closeAppWindows();
                }
            });
        }
    } else {
        /* parse settings error */
    }

    return true;
}

void CAscApplicationManagerWrapper::sendSettings(const wstring& opts)
{
    wstring _send_cmd, _send_opts;
    if ( opts == L"username" ) {
        _send_cmd = L"settings:username";
        _send_opts = Utils::systemUserName();
    } else
    if ( opts == L"has:opened" ) {
        _send_cmd = L"settings:hasopened";

        if ( m_pMainWindow && m_pMainWindow->editorsCount() ) {
            _send_opts = L"has";
        }
    }

    if ( !_send_opts.empty() )
        QTimer::singleShot(0, [_send_cmd, _send_opts] {
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, _send_cmd, _send_opts);
        });
}

void CAscApplicationManagerWrapper::checkSettings(const wstring& opts)
{
    QJsonParseError jerror;
    QByteArray stringdata = QString::fromStdWString(opts).toUtf8();
    QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        QJsonObject root = jdoc.object();

        if ( root.contains("langid") ) {
            QString _curr_lang = CLangater::getCurrentLangCode(),
                    _new_lang = root.value("langid").toString();
            if ( _curr_lang != _new_lang ) {
                bool direction_changed = CLangater::isRtlLanguage(_curr_lang) != CLangater::isRtlLanguage(_new_lang);

                QTimer::singleShot(0, this, [direction_changed] {
                    AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"settings:lang",
                                direction_changed ? L"restart:true":L"restart:false");
                });
            }
        }
    }
}

void CAscApplicationManagerWrapper::applyTheme(const wstring& theme, bool force)
{
    APP_CAST(_app);

    if ( !_app.m_themes->isThemeCurrent(theme) ) {
        const std::wstring old_theme = _app.m_themes->current().id();
        _app.m_themes->setCurrentTheme(theme);

        std::wstring params{InputArgs::change_webapps_param(L"&uitheme=" + old_theme, L"&uitheme=" + theme)};
        AscAppManager::getInstance().InitAdditionalEditorParams(params);

        EditorJSVariables::applyVariable("theme", {
                                            {"type", _app.m_themes->current().stype()},
                                            {"id", QString::fromStdWString(_app.m_themes->current().id())}
// #ifndef Q_OS_LINUX
                                            ,{"system", _app.m_themes->isSystemSchemeDark() ? "dark" : "light"}
// #else
//                                             ,{"system", "disabled"}
// #endif
                                         });

        // TODO: remove
        if ( mainWindow() ) mainWindow()->applyTheme(theme);

        const std::wstring actual_id{_app.m_themes->themeActualId(theme)};
        for ( auto const& r : m_winsReporter ) {
            r.second->applyTheme(actual_id);
        }


        CEditorWindow * _editor = nullptr;
        for ( auto const& e : m_vecEditors ) {
            _editor = reinterpret_cast<CEditorWindow *>(e);
            _editor->applyTheme(theme);
        }

        // QJsonObject _json_obj{{"theme", _app.m_themes->current().json()}};
        QJsonObject _json_obj{{"theme", QString::fromStdWString(actual_id)}};       // for bug 78050. send only theme id
        AscAppManager::getInstance().UpdatePlugins(Utils::stringifyJson(_json_obj).toStdWString());
        AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"uitheme:changed", theme);
    }
}

CThemes& CAscApplicationManagerWrapper::themes()
{
    return *(AscAppManager::getInstance().m_themes);
}

CPrintData& CAscApplicationManagerWrapper::printData()
{
    return *(AscAppManager::getInstance().m_private->m_printData);
}

bool CAscApplicationManagerWrapper::canAppClose()
{
    APP_CAST(_app);

    if ( !_app.m_vecEditors.empty() ) {
        bool _has_opened_editors = std::find_if(_app.m_vecEditors.begin(), _app.m_vecEditors.end(),
                [](size_t h){
                    CEditorWindow * _e = reinterpret_cast<CEditorWindow *>(h);
                    return _e && !_e->closed();
                }) != _app.m_vecEditors.end();

        if ( _has_opened_editors ) {
            mainWindow()->bringToTop();

//            CMessage mess(mainWindow()->handle(), CMessageOpts::moButtons::mbYesNo);
//            mess.setButtons({"Yes", "No", "Hide main window"});
//            switch (mess.warning(tr("Close all editors windows?"))) {
//            case MODAL_RESULT_CUSTOM + 0: return true;
//            case MODAL_RESULT_CUSTOM + 2:
                mainWindow()->hide();
                return false;
//            default: return false;
//            }
        }
    }

    return true;
}

bool CAscApplicationManagerWrapper::hasUnsavedChanges()
{
    APP_CAST(_app);
    if (_app.mainWindow()) {
        CAscTabWidget *tabs = _app.mainWindow()->tabWidget();
        for (int i = 0; i < tabs->count(); i++) {
            if (tabs->modifiedByIndex(i))
                return true;
        }
    }

    foreach (auto ptr, _app.m_vecEditors) {
        CEditorWindow *e = reinterpret_cast<CEditorWindow*>(ptr);
        if (e->modified())
            return true;
    }
    return false;
}

QCefView * CAscApplicationManagerWrapper::createViewer(QWidget * parent, const QSize& size)
{
    APP_CAST(_app);

    ++_app.m_countViews;
    return _app.m_private->createView(parent, size);
}

void CAscApplicationManagerWrapper::destroyViewer(int id)
{
    APP_CAST(_app);
    _app.DestroyCefView(id);
}

void CAscApplicationManagerWrapper::destroyViewer(QCefView * v)
{
    destroyViewer(v->GetCefView()->GetId());
}

uint CAscApplicationManagerWrapper::logoutCount(const wstring& portal) const
{
    uint _count = m_pMainWindow ? m_pMainWindow->editorsCount(portal) : 0;

    CEditorWindow * _editor;
    for (auto const& e : m_vecEditors ) {
        _editor = reinterpret_cast<CEditorWindow *>(e);
        if ( _editor->holdView(portal) )
            ++_count;
    }

    return _count;
}

void CAscApplicationManagerWrapper::Logout(const wstring& wjson)
{
    if ( !wjson.empty() ) {
        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();

            const wstring& portal = objRoot["domain"].toString().toStdWString();
            sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:logout", portal);

            std::vector<std::wstring> _portals{portal};

            if ( objRoot.contains("extra") && objRoot["extra"].isArray() ) {
                QJsonArray a = objRoot["extra"].toArray();
                for (auto&& v: a) {
                    _portals.push_back(v.toString().toStdWString());
                }
            }

            for (auto& v: _portals) {
                CAscApplicationManager::Logout(v);

                int index = mainWindow()->tabWidget()->tabIndexByUrl(v);
                if ( !(index < 0) ) {
                    if ( objRoot.contains("onsuccess") &&
                            objRoot["onsuccess"].toString() == "reload" )
                    {
                        mainWindow()->tabWidget()->panel(index)->cef()->reload();
                    } else mainWindow()->tabWidget()->closeEditorByIndex(index);
                }
            }
        }
    }
}

void CAscApplicationManagerWrapper::bindReceiver(int view_id, CCefEventsGate * const receiver)
{
    APP_CAST(_app);
    _app.m_receivers[view_id] = receiver;
}

void CAscApplicationManagerWrapper::unbindReceiver(int view_id)
{
    APP_CAST(_app);
    _app.m_receivers.erase(view_id);
}

void CAscApplicationManagerWrapper::unbindReceiver(const CCefEventsGate * receiver)
{
    APP_CAST(_app);

    map<int, CCefEventsGate *>::const_iterator it = _app.m_receivers.begin();
    while ( it != _app.m_receivers.cend() ) {
        if ( it->second == receiver ) {
            it = _app.m_receivers.erase(it);
            break;
        } else ++it;
    }

}

void CAscApplicationManagerWrapper::onDownloadSaveDialog(const std::wstring& name, uint id)
{
    QWidget * parent = WindowHelper::activeWindow();
    if ( parent ) {
        static bool saveInProcess = false;
        if ( !saveInProcess ) {
            saveInProcess = true;

            if ( name.size() ) {
                QString savePath = Utils::lastPath(LOCAL_PATH_SAVE);
                QString fullPath = savePath + "/" + QString().fromStdWString(name);
                CFileDialogWrapper dlg(parent);

                if ( dlg.modalSaveAs(fullPath) ) {
                    Utils::keepLastPath(LOCAL_PATH_SAVE, QFileInfo(fullPath).absoluteDir().absolutePath());
                }

                AscAppManager::getInstance().EndSaveDialog(fullPath.toStdWString(), id);
            }

            saveInProcess = false;
        }
    }
}

void CAscApplicationManagerWrapper::cancelClose()
{
    APP_CAST(_app);

    if ( _app.m_closeTarget.find(L"http") != wstring::npos ) {
        _app.sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:logout:cancel", _app.m_closeTarget);
    }

    _app.m_closeCount = 0;
    _app.m_closeTarget.clear();
    if ( _app.mainWindow() )
        _app.mainWindow()->cancelClose();

    getInstance().closeQueue().cancel();
}

CWindowsQueue<sWinTag>& CAscApplicationManagerWrapper::closeQueue()
{
    return *m_queueToClose;
}

CEventDriver& CAscApplicationManagerWrapper::commonEvents()
{
    return m_eventDriver;
}

void CAscApplicationManagerWrapper::onQueueCloseWindow(const sWinTag& t)
{
    if ( t.type == CLOSE_QUEUE_WIN_TYPE_MAIN ) {
        AscAppManager::getInstance().closeQueue().leave(sWinTag{CLOSE_QUEUE_WIN_TYPE_MAIN, size_t(mainWindow())});

        if ( !AscAppManager::getInstance().mainWindow()->isAboutToClose() )
            AscAppManager::getInstance().mainWindow()->close();
    } else {
        CEditorWindow * _e = reinterpret_cast<CEditorWindow *>(t.handle);
        int res = _e->closeWindow();
        if ( res == MODAL_RESULT_CANCEL ) {
            AscAppManager::getInstance().closeQueue().cancel();
        } else {
            AscAppManager::getInstance().closeQueue().leave(t);
        }
    }
}

QString CAscApplicationManagerWrapper::newFileName(int format)
{
    static int docx_count = 0,
                 xlsx_count = 0,
                 pptx_count = 0,
                 pdf_count = 0;

    switch ( format ) {
    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX:
    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:        return tr("Document%1.docx").arg(++docx_count);
    case AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF:
    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF: {
        QString docname = tr("Document%1.docx").arg(++pdf_count);
        return docname.replace("docx", "pdf");
    }
    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX:
    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX:     return tr("Book%1.xlsx").arg(++xlsx_count);
    case AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX:
    case AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX:    return tr("Presentation%1.pptx").arg(++pptx_count);
    default:                                         return "Document.asc";
    }
}

QString CAscApplicationManagerWrapper::newFileName(const std::wstring& format)
{
    int _f = format == L"word" ? AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX :
                 format == L"cell" ? AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX :
                 format == L"form" ? AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF :
                 // format == L"draw" ? AVS_OFFICESTUDIO_FILE_DRAW_VSDX :
                 format == L"slide" ? AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX : AVS_OFFICESTUDIO_FILE_UNKNOWN;

    return newFileName(_f);
}

wstring CAscApplicationManagerWrapper::userSettings(const wstring& name)
{
    unique_ptr<CUserSettings> pSettings{AscAppManager::getInstance().GetUserSettings()};
    return pSettings->Get(name);
//    delete pSettings;
}

void CAscApplicationManagerWrapper::setUserSettings(const wstring& name, const wstring& value)
{
    unique_ptr<CUserSettings> pSettings{AscAppManager::getInstance().GetUserSettings()};
    pSettings->Set(name, value);
}

void CAscApplicationManagerWrapper::onFileChecked(const QString& name, int uid, bool exists)
{
    Q_UNUSED(name)

    if ( !exists ) {
        QJsonObject _json_obj{{QString::number(uid), exists}};
        QString json = QJsonDocument(_json_obj).toJson(QJsonDocument::Compact);

        sendCommandTo(SEND_TO_ALL_START_PAGE, "files:checked", json);
    }
}

void CAscApplicationManagerWrapper::onEditorWidgetClosed()
{
    if ( m_countViews == 0 ) {
        DestroyCefView(-1);
    }
}

void CAscApplicationManagerWrapper::onMainWindowClose()
{
    if ( !m_vecEditors.empty() ) {
        mainWindow()->hide();
    } else {
        launchAppClose();
    }

}

void CAscApplicationManagerWrapper::addStylesheets(CScalingFactor f, const std::string& path)
{
    if ( m_mapStyles.find(f) == m_mapStyles.end() ) {
        m_mapStyles[f] = {path};
    } else m_mapStyles[f].push_back(path);

}
