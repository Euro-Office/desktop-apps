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

#include "association.h"
#include "utils.h"
#include "defines.h"

#include "cascapplicationmanagerwrapper.h"
#include <unordered_map>
#include <unordered_set>
#include <shlwapi.h>
#include <ShlObj.h>
#include <ctime>
#ifdef __OS_WIN_XP
# include "components/cmessage.h"
#else
# include "components/cnotification.h"
#endif

#define DLG_RESULT_NONE -2
#define DAY_TO_SEC 24*3600
#define REG_FILE_ASSOC "SOFTWARE\\" REG_GROUP_KEY "\\" REG_APP_NAME "\\Capabilities\\FileAssociations"


#ifdef __OS_WIN_XP
// static void regValue(HKEY rootKey, LPCWSTR ext, std::wstring &value)
// {
//     HKEY hKey;
//     if (RegOpenKeyEx(rootKey, L"SOFTWARE\\Classes", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
//         DWORD type = REG_SZ, cbData = 0;
//         if (SHGetValue(hKey, ext, L"", &type, NULL, &cbData) == ERROR_SUCCESS) {
//             wchar_t *pvData = (wchar_t*)malloc(cbData);
//             if (SHGetValue(hKey, ext, L"", &type, (void*)pvData, &cbData) == ERROR_SUCCESS)
//                 value = pvData;
//             free(pvData);
//         }
//         RegCloseKey(hKey);
//     }
// }
#endif

class Association::AssociationPrivate : public QObject
{
    Q_OBJECT
public:
    AssociationPrivate();
    ~AssociationPrivate();

    // bool isFirstRun();
    bool isFormatAssociated(const wchar_t*);
    void tryProposeAssociation(QWidget *parent, const std::wstring &fileExt = L"");
    void associate(const std::vector<std::wstring> &unassocFileExts);

    time_t m_lastCheck = 0;
    bool m_ignoreAssocMsg;
    std::unordered_map<std::wstring, std::wstring> m_extMap;
    class DialogSchedule;
    DialogSchedule *m_pDialogSchedule;

public slots:
    void showAssociationMessage(QWidget *parent, const std::vector<std::wstring> &unassocFileExts, bool forceModal = false, int result = DLG_RESULT_NONE);
};

class Association::AssociationPrivate::DialogSchedule : public QObject
{
public:
    DialogSchedule(AssociationPrivate *owner);
    void addToSchedule(const std::wstring &fileExt);

private:
    AssociationPrivate *m_owner;
    QTimer *m_timer;
    std::unordered_set<std::wstring> m_unique_ext;
};

Association::AssociationPrivate::DialogSchedule::DialogSchedule(AssociationPrivate *owner) :
    QObject(),
    m_owner(owner)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(3000);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, [=] {
        QWidget *wnd = WindowHelper::currentTopWindow();
        if (wnd && !m_unique_ext.empty()) {
            std::wstring fileExt;
            if (m_unique_ext.size() == 1) {
                if (*m_unique_ext.begin() != L".all")
                    fileExt = *m_unique_ext.begin();
            } else
            if (m_unique_ext.size() == 2 && m_unique_ext.find(L".all") != m_unique_ext.end()) {
                for (const auto &ext : m_unique_ext) {
                    if (ext != L".all") {
                        fileExt = ext;
                        break;
                    }
                }
            }

            QTimer::singleShot(0, this, [=]() {
                m_owner->tryProposeAssociation(wnd, fileExt);
            });
            m_unique_ext.clear();
            m_timer->stop();
        }
    });
}

void Association::AssociationPrivate::DialogSchedule::addToSchedule(const std::wstring &fileExt)
{
    m_unique_ext.insert(fileExt);
    if (!m_timer->isActive())
        m_timer->start();
}

Association::AssociationPrivate::AssociationPrivate() :
    QObject(),
    m_pDialogSchedule(new DialogSchedule(this))
{
    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    m_ignoreAssocMsg = reg_system.value("ignoreAssocMsg", false).toBool() || reg_user.value("ignoreAssocMsg", false).toBool() || IsPackage(Portable);
    if (!m_ignoreAssocMsg)
        m_lastCheck = time_t(reg_user.value("lastAssocCheck", 0).toLongLong());

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(REG_FILE_ASSOC), 0, KEY_READ | KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD cValues = 0, cbMaxValueNameLen = 0, cbMaxValueLen = 0;
        if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, &cbMaxValueNameLen, &cbMaxValueLen, NULL, NULL) == ERROR_SUCCESS) {
            if (cValues > 0 && cbMaxValueLen > 0) {
                DWORD cValueName = cbMaxValueNameLen + 1, cbData = cbMaxValueLen, pType = REG_SZ;
                WCHAR *lpData = new WCHAR[cbData/sizeof(WCHAR)];
                WCHAR *lpValueName = new WCHAR[cValueName];
                for (int i = 0; i < cValues; i++) {
                    if (RegEnumValue(hKey, i, lpValueName, &cValueName, NULL, &pType, (LPBYTE)lpData, &cbData) == ERROR_SUCCESS) {
                        if (lpData[0] != '\0')
                            m_extMap[lpValueName] = lpData;
                    }
                    cValueName = cbMaxValueNameLen + 1;
                    cbData = cbMaxValueLen;
                }
                delete[] lpValueName;
                delete[] lpData;
            }
        }
        RegCloseKey(hKey);
    }
}

Association::AssociationPrivate::~AssociationPrivate()
{
    delete m_pDialogSchedule, m_pDialogSchedule = nullptr;
}

// bool Association::AssociationPrivate::isFirstRun()
// {
//     GET_REGISTRY_USER(reg_user);
//     if (!reg_user.contains("hasRunBefore")) {
//         reg_user.setValue("hasRunBefore", true);
//         return true;
//     }
//     return false;
// }

bool Association::AssociationPrivate::isFormatAssociated(const wchar_t *fileExt)
{
    DWORD bufSize = 0;
    HRESULT hr = AssocQueryString(ASSOCF_NONE, ASSOCSTR_FRIENDLYAPPNAME, fileExt, NULL, NULL, &bufSize);
    if (hr == S_FALSE) {
        std::wstring buf(bufSize - 1, '\0');
        hr = AssocQueryString(ASSOCF_NONE, ASSOCSTR_FRIENDLYAPPNAME, fileExt, NULL, &buf[0], &bufSize);
        if (SUCCEEDED(hr))
            return buf.find(TEXT(APP_REG_NAME)) != std::wstring::npos;
    }
    return false;
}

void Association::AssociationPrivate::tryProposeAssociation(QWidget *parent, const std::wstring &fileExt)
{
    if (m_ignoreAssocMsg)
        return;

    m_lastCheck = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.setValue("lastAssocCheck", static_cast<long long>(m_lastCheck));

    std::vector<std::wstring> unassocFileExts;
    if (fileExt.empty()) {
        for (auto it = m_extMap.begin(); it != m_extMap.end(); ++it) {
            if (!isFormatAssociated(it->first.c_str()))
                unassocFileExts.push_back(it->first);
        }
    } else
    if (!isFormatAssociated(fileExt.c_str()))
        unassocFileExts.push_back(fileExt);

    if (!unassocFileExts.empty()) {
        showAssociationMessage(parent, unassocFileExts);
    }
}

void Association::AssociationPrivate::showAssociationMessage(QWidget *parent, const std::vector<std::wstring> &unassocFileExts, bool forceModal, int result)
{
    if (result == DLG_RESULT_NONE) {
        QString msg = unassocFileExts.size() == 1 ? QObject::tr("Do you want to make %1 your default application for extension: %2?")
                                                        .arg(QString(WINDOW_NAME), QString::fromStdWString(unassocFileExts[0])) :
                                                    QObject::tr("Do you want to make %1 your default application for all supported extensions?")
                                                        .arg(QString(WINDOW_NAME));
#ifndef __OS_WIN_XP
        if (!forceModal && AscAppManager::notificationSupported()) {
            if (CNotification::instance().show(QObject::tr("Set Default App"), msg,
                       MsgBtns::mbYesDefNo, [=](int res) {
                            QMetaObject::invokeMethod(this, "showAssociationMessage", Qt::QueuedConnection, Q_ARG(QWidget*, parent),
                                Q_ARG(std::vector<std::wstring>, unassocFileExts), Q_ARG(bool, res == NOTIF_FAILED), Q_ARG(int, res));
                       })) {
                return;
            }
        }
#endif
        CMessageOpts opts;
        opts.checkBoxState = &m_ignoreAssocMsg;
        opts.chekBoxText = QObject::tr("Do not show this message again");
        result = CMessage::showMessage(parent, msg, MsgType::MSG_INFO, MsgBtns::mbYesDefNo, opts);
        if (m_ignoreAssocMsg) {
            GET_REGISTRY_USER(reg_user)
            reg_user.setValue("ignoreAssocMsg", true);
        }
    }

    if (result == MODAL_RESULT_YES) {
        associate(unassocFileExts);
//         return;
//         if (Utils::getWinVersion() >= Utils::WinVer::Win10) {
//             ShellExecute(NULL, L"open", L"ms-settings:defaultapps", NULL, NULL, SW_SHOWNORMAL);
//         } else
//         if (Utils::getWinVersion() >= Utils::WinVer::Win8) {
// #ifndef __OS_WIN_XP
//             HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
//             if (SUCCEEDED(hr)) {
//                 IApplicationAssociationRegistrationUI *ar;
//                 HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI, 0, CLSCTX_INPROC_SERVER,
//                                               IID_IApplicationAssociationRegistrationUI, (void**)&ar);
//                 if (SUCCEEDED(hr)) {
//                     ar->LaunchAdvancedAssociationUI(TEXT(APP_REG_NAME));
//                     ar->Release();
//                 }
//                 CoUninitialize();
//             }
// #endif
//         } else {
//             associate(unassocFileExts);
//         }
    }
}

void Association::AssociationPrivate::associate(const std::vector<std::wstring> &unassocFileExts)
{
#ifdef __OS_WIN_XP
    for (const auto &ext : unassocFileExts) {
        if (m_extMap.find(ext) != m_extMap.end()) {
            std::wstring progId = m_extMap[ext];
            // std::wstring progId1, progId2;
            // regValue(HKEY_LOCAL_MACHINE, ext.c_str(), progId1);
            // regValue(HKEY_CURRENT_USER, ext.c_str(), progId2);
            // if ((!progId2.empty() && progId2 != progId) || (!progId1.empty() && progId1 != progId)) {
                HKEY hKey;
                std::wstring userChoise = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + ext;
                if (RegOpenKeyEx(HKEY_CURRENT_USER, userChoise.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
                    if (RegDeleteKey(hKey, L"UserChoice") == ERROR_SUCCESS) {
                        HKEY hKeyUser;
                        std::wstring path = L"SOFTWARE\\Classes\\" + ext;
                        if (RegOpenKeyEx(HKEY_CURRENT_USER, path.c_str(), 0, KEY_ALL_ACCESS, &hKeyUser) == ERROR_SUCCESS) {
                            SHSetValue(hKeyUser, L"", 0, REG_SZ, (const BYTE*)progId.c_str(), (DWORD)(progId.length() + 1) * sizeof(WCHAR));
                            RegCloseKey(hKeyUser);
                        }
                    }
                    RegCloseKey(hKey);
                }
            // }
        }
    }
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
#else
    if (Utils::getWinVersion() > Utils::WinVer::Win7) {
        std::wstring args(L"--assoc \"");
        for (const auto &ext : unassocFileExts) {
            auto it = m_extMap.find(ext);
            if (it != m_extMap.end()) {
                args.append(it->first);
                args.append(L":");
                args.append(it->second);
                args.append(L";");
            }
        }
        args.append(L"\"");

        QString appPath = qApp->applicationDirPath();
        appPath += "/" + QString(REG_APP_NAME);
        std::wstring path = appPath.toStdWString();

        SHELLEXECUTEINFO shExInfo = {0};
        shExInfo.cbSize = sizeof(shExInfo);
        shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE | SEE_MASK_FLAG_NO_UI;
        shExInfo.hwnd = NULL;
        shExInfo.lpVerb = L"open";
        shExInfo.lpFile = path.c_str();
        shExInfo.lpParameters = args.c_str();
        shExInfo.lpDirectory = NULL;
        shExInfo.nShow = SW_HIDE;
        shExInfo.hInstApp = NULL;
        if (ShellExecuteEx(&shExInfo)) {
            CloseHandle(shExInfo.hProcess);
        }
        return;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IApplicationAssociationRegistration *pAr;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, IID_IApplicationAssociationRegistration, (void**)&pAr);
        if (SUCCEEDED(hr)) {
            for (const auto &ext : unassocFileExts) {
                if (m_extMap.find(ext) != m_extMap.end())
                    pAr->SetAppAsDefault(TEXT(APP_REG_NAME), ext.c_str(), AT_FILEEXTENSION);
            }
            pAr->Release();
        }
        CoUninitialize();
    }
#endif
}

Association::Association() : pimpl(new AssociationPrivate)
{

}

Association::~Association()
{
    delete pimpl, pimpl = nullptr;
}

Association& Association::instance()
{
    static Association inst;
    return inst;
}

void Association::chekForAssociations(int uid)
{
    if (pimpl->m_ignoreAssocMsg || (time(nullptr) - pimpl->m_lastCheck) < DAY_TO_SEC)
        return;

    if (uid < 0/*Association::isFirstRun()*/) {
        pimpl->m_pDialogSchedule->addToSchedule(L".all");
    } else
    /*if (uid > -1)*/ {
        CAscTabData *data = nullptr;
        AscAppManager &app = AscAppManager::getInstance();
        if (CEditorWindow *editor = app.editorWindowFromViewId(uid)) {
            data = editor->mainView()->data();
        } else
        if (app.mainWindow() && app.mainWindow()->holdView(uid)) {
            int indx = app.mainWindow()->tabWidget()->tabIndexByView(uid);
            if (indx > -1)
                data = app.mainWindow()->tabWidget()->panel(indx)->data();
        }

        if (data) {
            std::wstring fileExt;
            if (data->isLocal() && !data->url().empty()) {
                QFileInfo inf(QString::fromStdWString(data->url()));
                fileExt = inf.completeSuffix().toStdWString();
                if (!fileExt.empty())
                    fileExt.insert(fileExt.begin(), L'.');
            }

            if (fileExt.empty()) {
                switch (data->contentType()) {
                case AscEditorType::etDocument:
                    fileExt = L".docx";
                    break;
                case AscEditorType::etPresentation:
                    fileExt = L".pptx";
                    break;
                case AscEditorType::etSpreadsheet:
                    fileExt = L".xlsx";
                    break;
                case AscEditorType::etPdf:
                    fileExt = L".pdf";
                    break;
                case AscEditorType::etDraw:
                    fileExt = L".vsdx";
                    break;
                default:
                    break;
                }
            }

            if (!fileExt.empty() && pimpl->m_extMap.find(fileExt) != pimpl->m_extMap.end())
                pimpl->m_pDialogSchedule->addToSchedule(fileExt);
        }
    }
}

#include "association.moc"
