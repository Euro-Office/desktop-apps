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

#include "cupdatemanager.h"
#include <QApplication>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <vector>
#include <sstream>
#include "utils.h"
#include "defines.h"
#include "version.h"
#include "clangater.h"
#include "clogger.h"
#include "components/cnotification.h"
#include "cascapplicationmanagerwrapper.h"
#include <QCryptographicHash>
#ifdef _WIN32
# define DAEMON_NAME L"/updatesvc.exe"
# define GetPid() GetCurrentProcessId()
#else
# include <QProcess>
# include <dirent.h>
# include <unistd.h>
# include <spawn.h>
# include <fcntl.h>
# include <elf.h>
# define DAEMON_NAME "/updatesvc"
# define GetPid() getpid()
#endif

#include <qtcomp/qdatetime.h>

#define modeToEnum(mod) ((mod == "silent") ? UpdateMode::SILENT : (mod == "ask") ? UpdateMode::ASK : UpdateMode::DISABLE)
#define packageToStr() QString(IsPackage(ISS) ? "iss" : IsPackage(MSI) ? "msi" : IsPackage(Portable) ? "portable" : "other")
#define WStrToTStr(str) QStrToTStr(QString::fromStdWString(str))
#define DAY_TO_SEC 24*3600
#define MINIMUM_INTERVAL 30
#define RESET_MESSAGE_MS 20000
#define CHECK_ON_STARTUP_MS 9000
#define CMD_ARGUMENT_UPDATES_INTERVAL L"--updates-interval"
#define SERVICE_NAME APP_TITLE " Update Service"
#define LINK_TEXT QString("<a href=\"%1\">%2</a>").arg(QString(RELEASE_NOTES), QObject::tr("Release notes"))
#define __GLOBAL_LOCK if (m_lock) {CLogger::log("Blocked in: " + FUNCTION_INFO); return;} m_lock = true; \
                          CLogger::log("Locking and further execution:" + FUNCTION_INFO);
#define __UNLOCK m_lock = false; CLogger::log("Unlocked in:" + FUNCTION_INFO);

using std::vector;

const char *SVC_TXT_ERR_UNPACKING   = QT_TRANSLATE_NOOP("CUpdateManager", "An error occurred while unpacking the archive"),
           *SVC_TXT_ERR_DNL_OUT_MEM = QT_TRANSLATE_NOOP("CUpdateManager", "Update failed: out of memory!"),
           *SVC_TXT_ERR_DNL_CONN    = QT_TRANSLATE_NOOP("CUpdateManager", "Update failed: server connection error!"),
           *SVC_TXT_ERR_DNL_URL     = QT_TRANSLATE_NOOP("CUpdateManager", "Update failed: wrong URL!"),
           *SVC_TXT_ERR_DNL_CREAT   = QT_TRANSLATE_NOOP("CUpdateManager", "Update failed: unable to create file!"),
           *SVC_TXT_ERR_DNL_INET    = QT_TRANSLATE_NOOP("CUpdateManager", "Update failed: network error!"),
           *SVC_TXT_ERR_OTHER       = QT_TRANSLATE_NOOP("CUpdateManager", "A service error has occurred!"),

           *TXT_LAST_CHECK      = QT_TRANSLATE_NOOP("CUpdateManager", "Last check performed %1"),
           *TXT_UPDATED         = QT_TRANSLATE_NOOP("CUpdateManager", "Current version is up to date"),
           *TXT_CHECKING_UPD    = QT_TRANSLATE_NOOP("CUpdateManager", "Checking for updates..."),
           *TXT_AVAILABLE_UPD   = QT_TRANSLATE_NOOP("CUpdateManager", "Update is available (version %1)"),
           *TXT_AVAILABLE_SVC   = QT_TRANSLATE_NOOP("CUpdateManager", "Service update is available (version %1)"),
           *TXT_DOWNLOADING_UPD = QT_TRANSLATE_NOOP("CUpdateManager", "Downloading new version %1 (%2%)"),
           *TXT_PREPARING_UPD   = QT_TRANSLATE_NOOP("CUpdateManager", "Preparing update..."),
           *TXT_UNZIP_UPD       = QT_TRANSLATE_NOOP("CUpdateManager", "Preparing update (%1%)"),
           *TXT_RESTART_TO_UPD  = QT_TRANSLATE_NOOP("CUpdateManager", "To finish updating, restart app"),
           *TXT_ERR_NOT_ALLOWED = QT_TRANSLATE_NOOP("CUpdateManager", "Updates are not allowed!"),
           *TXT_ERR_URL         = QT_TRANSLATE_NOOP("CUpdateManager", "Unable to check update: URL not defined."),
           *TXT_ERR_PACK_URL    = QT_TRANSLATE_NOOP("CUpdateManager", "An error occurred while loading updates: package Url is empty!"),
           *TXT_ERR_CHECK       = QT_TRANSLATE_NOOP("CUpdateManager", "An error occurred while check updates: the Update Service is not installed or is not running!"),
           *TXT_ERR_LOAD        = QT_TRANSLATE_NOOP("CUpdateManager", "An error occurred while loading updates: the Update Service is not installed or is not running!"),
           *TXT_ERR_UNZIP       = QT_TRANSLATE_NOOP("CUpdateManager", "An error occurred while unzip updates: the Update Service is not installed or is not running!"),
           *TXT_ERR_JSON        = QT_TRANSLATE_NOOP("CUpdateManager", "Error opening JSON file."),
           *TXT_ERR_MD5         = QT_TRANSLATE_NOOP("CUpdateManager", "Update package error: md5 sum does not match the original."),

           *BTN_TXT_CHECK    = QT_TRANSLATE_NOOP("CUpdateManager", "Check for updates"),
           *BTN_TXT_DOWNLOAD = QT_TRANSLATE_NOOP("CUpdateManager", "Download update"),
           *BTN_TXT_RESTART  = QT_TRANSLATE_NOOP("CUpdateManager", "Restart"),
           *BTN_TXT_CANCEL   = QT_TRANSLATE_NOOP("CUpdateManager", "Cancel");

class CUpdateManager::DialogSchedule : public QObject
{
    Q_OBJECT
public:
    DialogSchedule(QObject *owner);
public slots:
    void addToSchedule(const QString &method, const QString &text = QString());

private:
    struct Tag {
        QString method,
                text;
    };
    QObject *m_owner = nullptr;
    QTimer *m_timer = nullptr;
    QVector<Tag> m_shedule_vec;
};

CUpdateManager::DialogSchedule::DialogSchedule(QObject *owner) :
    QObject(owner),
    m_owner(owner)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, [=] {
        QWidget *wnd = WindowHelper::currentTopWindow();
        if (wnd && !m_shedule_vec.isEmpty()) {
            QByteArray method = m_shedule_vec.first().method.toLocal8Bit();
            QString text = m_shedule_vec.first().text;
            if (text.isEmpty())
                QMetaObject::invokeMethod(owner, method.data(), Qt::QueuedConnection, Q_ARG(QWidget*, wnd));
            else
                QMetaObject::invokeMethod(owner, method.data(), Qt::QueuedConnection, Q_ARG(QWidget*, wnd), Q_ARG(QString, text));
            m_shedule_vec.removeFirst();
            if (m_shedule_vec.isEmpty()) {
                m_timer->stop();
                qobject_cast<CUpdateManager*>(owner)->refreshStartPage({"", {}, nullptr, "", "false"});
            }
        }
    });
}

void CUpdateManager::DialogSchedule::addToSchedule(const QString &method, const QString &text)
{
    m_shedule_vec.push_back({method, text});
    if (!m_timer->isActive()) {
        m_timer->start();
        qobject_cast<CUpdateManager*>(m_owner)->refreshStartPage({"", {}, nullptr, "", "true"});
    }
}

auto currentArch()->QString
{
#ifdef _WIN32
# if defined(_M_ARM64)
    return "_arm64";
# elif defined(_M_X64)
    return "_x64";
# elif defined(_M_IX86)
    return "_x86";
# endif
#else
    return "_x64";
#endif
}

auto formattedTime(time_t timestamp)->QString
{
    return (timestamp != 0) ? QLocale::system().toString(QtComp::DateTime::fromTimestamp(timestamp), QLocale::ShortFormat) :
               QString("--.--.---- --:--");
}

auto getFileHash(const QString &fileName)->QString
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&file)) {
            file.close();
            return QString(hash.result().toHex()).toLower();
        }
        file.close();
    }
    return QString();
}

auto runProcess(const tstring &fileName, const tstring &args, bool runAsAdmin = false)->bool
{
#ifdef _WIN32
    SHELLEXECUTEINFO shExInfo = {0};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE | SEE_MASK_FLAG_NO_UI;
    shExInfo.hwnd = NULL;
    shExInfo.lpVerb = runAsAdmin ? L"runas" : L"open";
    shExInfo.lpFile = fileName.c_str();
    shExInfo.lpParameters = args.c_str();
    shExInfo.lpDirectory = NULL;
    shExInfo.nShow = SW_HIDE;
    shExInfo.hInstApp = NULL;
    if (ShellExecuteEx(&shExInfo)) {
        //WaitForSingleObject(shExInfo.hProcess, INFINITE);
        CloseHandle(shExInfo.hProcess);
        return true;
    }
    return false;
#else
    Q_UNUSED(runAsAdmin)
    const QStringList args_list = args.empty() ? QStringList() : QString::fromStdString(args).split(" ");
    char **_args = new char*[args_list.size() + 2];
    int i = 0;
    _args[i++] = const_cast<char*>(fileName.c_str());
    for (const auto &arg : args_list)
        _args[i++] = arg.toLocal8Bit().data();
    _args[i] = NULL;
    pid_t pid;
    posix_spawn_file_actions_t acts;
    posix_spawn_file_actions_init(&acts);
    char fd_path[64];
    snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd", getpid());
    struct dirent *entry;
    if (DIR *dir = opendir(fd_path)) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_LNK) {
                int fd = atoi(entry->d_name);
                if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
                    posix_spawn_file_actions_addclose(&acts, fd);
            }
        }
        closedir(dir);
    }
    int res = posix_spawn(&pid, fileName.c_str(), &acts, NULL, _args, environ);
    posix_spawn_file_actions_destroy(&acts);
    delete[] _args;
    return res == 0;
#endif
}

auto getFileVersion(const tstring &filePath)->QString
{
    QString ver;
#ifdef _WIN32
    DWORD handle, size = GetFileVersionInfoSize(filePath.c_str(), &handle);
    if (size > 0) {
        BYTE *data = new BYTE[size];
        if (GetFileVersionInfo(filePath.c_str(), handle, size, (LPVOID)data)) {
            UINT len = 0;
            VS_FIXEDFILEINFO *verInfo = NULL;
            if (VerQueryValue((LPCVOID)data, L"\\", (LPVOID*)&verInfo, &len)) {
                if (verInfo->dwSignature == 0xfeef04bd) {
                    ver = QString("%1.%2.%3.%4").arg(QString::number(HIWORD(verInfo->dwFileVersionMS)),
                                                     QString::number(LOWORD(verInfo->dwFileVersionMS)),
                                                     QString::number(HIWORD(verInfo->dwFileVersionLS)),
                                                     QString::number(LOWORD(verInfo->dwFileVersionLS)));
                }
            }
        }
        delete[] data;
    }
#else
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd != -1) {
        Elf64_Ehdr header;
        if (read(fd, &header, sizeof(header)) == sizeof(header)) {
            Elf64_Shdr section;
            off_t ofset = header.e_shoff + header.e_shentsize * header.e_shstrndx;
            if (lseek(fd, ofset, SEEK_SET) == ofset && read(fd, &section, sizeof(section)) == sizeof(section)) {
                char *shstrtab = new char[section.sh_size];
                if (lseek(fd, section.sh_offset, SEEK_SET) == (off_t)section.sh_offset &&
                        read(fd, shstrtab, section.sh_size) == (ssize_t)section.sh_size) {
                    for (int i = 0; i < header.e_shnum; ++i) {
                        ofset = header.e_shoff + i * header.e_shentsize;
                        if (lseek(fd, ofset, SEEK_SET) == ofset && read(fd, &section, sizeof(section)) == sizeof(section)) {
                            if (strcmp(".version_info", shstrtab + section.sh_name) == 0) {
                                if (lseek(fd, section.sh_offset, SEEK_SET) == (off_t)section.sh_offset) {
                                    char *version = new char[section.sh_size];
                                    if (read(fd, version, section.sh_size) == (ssize_t)section.sh_size)
                                        ver = QString(version);
                                    delete[] version;
                                }
                                break;
                            }
                        }
                    }
                }
                delete[] shstrtab;
            }
        }
        close(fd);
    }
#endif
    return ver;
}

struct CUpdateManager::PackageData {
    QString fileName,
            fileType,
            fileSize,
            object,
            hash,
            version;
    wstring packageUrl;
    bool    isInstallable = true;
    void clear() {
        fileName.clear();
        fileType.clear();
        fileSize.clear();
        object.clear();
        hash.clear();
        version.clear();
        packageUrl.clear();
        isInstallable = true;
    }
};

struct CUpdateManager::SavedPackageData {
    QString fileName,
            fileType,
            version;
};

CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    m_packageData(new PackageData),
    m_savedPackageData(new SavedPackageData),
    m_dialogSchedule(new DialogSchedule(this)),
    m_socket(new CSocket(SVC_PORT, APP_PORT))
{
    // =========== Set updates URL ============
    if ( Utils::updatesAllowed() ) {
        if ( InputArgs::contains(CMD_ARGUMENT_UPDATES_INTERVAL) ) {
            int interval = QString::fromStdWString(InputArgs::argument_value(CMD_ARGUMENT_UPDATES_INTERVAL)).toInt();
            if (interval >= MINIMUM_INTERVAL) {
                GET_REGISTRY_USER(reg_user)
                reg_user.beginGroup("Updates");
                reg_user.setValue("interval", interval);
                reg_user.endGroup();
            }
        }

        CLogger::log("Updates is on");
        m_pIntervalTimer = new QTimer(this);
        m_pIntervalTimer->setSingleShot(false);
        connect(m_pIntervalTimer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
        m_pIntervalStartTimer = new QTimer(this);
        m_pIntervalStartTimer->setSingleShot(true);
        m_pIntervalStartTimer->setInterval(CHECK_ON_STARTUP_MS);
        connect(m_pIntervalStartTimer, &QTimer::timeout, this, &CUpdateManager::updateNeededCheking);
        m_pLastCheckMsgTimer = new QTimer(this);
        m_pLastCheckMsgTimer->setSingleShot(true);
        m_pLastCheckMsgTimer->setInterval(RESET_MESSAGE_MS);
        connect(m_pLastCheckMsgTimer, &QTimer::timeout, this, [=]() {
            refreshStartPage({"lastcheck", {TXT_LAST_CHECK, formattedTime(m_lastCheck)}});
        });
        if (IsPackage(Portable)) {
            int pid = GetPid();
            std::string msg = std::to_string(pid);
            CSocket sock(INSTANCE_SVC_PORT, 0);
            sock.sendMessage((void*)msg.c_str(), msg.length() + 1);
            tstring args = _T("--run-as-app ") + std::to_tstring(pid);
            for (const auto &arg : InputArgs::arguments())
                args.append(_T(" ") + WStrToTStr(arg));
            runProcess(QStrToTStr(qApp->applicationDirPath()) + DAEMON_NAME, args);
        }
        init();
    } else {
        CLogger::log("Updates is off");
        refreshStartPage({"error", {TXT_ERR_NOT_ALLOWED}, BTN_TXT_CHECK, "", "true"});
    }
}

CUpdateManager::~CUpdateManager()
{
    delete m_packageData, m_packageData = nullptr;
    delete m_savedPackageData, m_savedPackageData = nullptr;
    delete m_dialogSchedule, m_dialogSchedule = nullptr;
    delete m_socket, m_socket = nullptr;
    if (IsPackage(Portable)) {
        CSocket sock(INSTANCE_SVC_PORT, 0);
        const char msg[] = "stop";
        sock.sendMessage((void*)msg, sizeof(msg));
    }
}

void CUpdateManager::init()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    m_savedPackageData->fileName = reg_user.value("file", QString()).toString();
    m_savedPackageData->fileType = reg_user.value("type", QString()).toString();
    m_savedPackageData->version = reg_user.value("version", QString()).toString();
    m_lastCheck = time_t(reg_user.value("last_check", 0).toLongLong());
    refreshStartPage({"lastcheck", {TXT_LAST_CHECK, formattedTime(m_lastCheck)}, BTN_TXT_CHECK, "check", "false"});
    m_interval = reg_user.value("interval", DAY_TO_SEC).toInt();
    if (m_interval < MINIMUM_INTERVAL)
        m_interval = MINIMUM_INTERVAL;
    reg_user.endGroup();

    m_socket->onMessageReceived([this](void *data, size_t) {
        vector<tstring> params;
        if (m_socket->parseMessage(data, params) == 3) {
            switch (std::stoi(params[0])) {
            case MSG_LoadCheckFinished:
                QMetaObject::invokeMethod(this, "onLoadCheckFinished", Qt::QueuedConnection, Q_ARG(QString, TStrToQStr(params[1])));
                break;

            case MSG_LoadUpdateFinished:
                QMetaObject::invokeMethod(this, "onLoadUpdateFinished", Qt::QueuedConnection, Q_ARG(QString, TStrToQStr(params[1])));
                break;

            case MSG_ShowStartInstallMessage: {
                refreshStartPage({"success", {TXT_RESTART_TO_UPD}, BTN_TXT_RESTART, "install", "false"});
                QMetaObject::invokeMethod(m_dialogSchedule, "addToSchedule", Qt::QueuedConnection, Q_ARG(QString, QString("showStartInstallMessage")));
                break;
            }

            case MSG_Progress:
                QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, std::stoi(params[1])));
                break;

            case MSG_UnzipProgress:
                QMetaObject::invokeMethod(this, "onUnzipProgressSlot", Qt::QueuedConnection, Q_ARG(int, std::stoi(params[1])));
                break;

            case MSG_RequestContentLenght: {
                double fileSize = std::stod(params[1])/1024/1024;
                m_packageData->fileSize = (fileSize == 0) ? "--" : QString::number(fileSize, 'f', 1);
                QMetaObject::invokeMethod(this, "onCheckFinished", Qt::QueuedConnection, Q_ARG(bool, false), Q_ARG(bool, true),
                                          Q_ARG(QString, m_packageData->version), Q_ARG(QString, ""));
                break;
            }

            case MSG_OtherError:
                QMetaObject::invokeMethod(this, "onError", Qt::QueuedConnection, Q_ARG(QString, TStrToQStr(params[1])));
                break;

            default:
                break;
            }
        }
    });
}

void CUpdateManager::criticalMsg(QWidget *parent, const QString &msg)
{
    if (!m_manualCheck) {
        __UNLOCK
        return;
    }
#ifdef _WIN32
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : NULL;
    wstring lpText = msg.toStdWString();
    MessageBoxW(parent_hwnd, lpText.c_str(), TEXT(APP_TITLE), MB_ICONERROR | MB_SERVICE_NOTIFICATION_NT3X | MB_SETFOREGROUND);
#else
    CMessage::error(parent, msg);
#endif
    __UNLOCK
}

void CUpdateManager::clearTempFiles(const QString &except)
{
    static bool lock = false;
    if (!lock) { // for one-time cleaning
        lock = true;
        m_socket->sendMessage(MSG_ClearTempFiles, QStrToTStr(QString(FILE_PREFIX)), QStrToTStr(except));
    }
    if (except.isEmpty())
        savePackageData();
}

void CUpdateManager::checkUpdates(bool manualCheck)
{
    m_pIntervalStartTimer->stop();
    m_lastCheck = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("last_check", static_cast<qlonglong>(m_lastCheck));
    reg_user.endGroup();
    if (getUpdateMode() != UpdateMode::DISABLE)
        m_pIntervalStartTimer->start();

    __GLOBAL_LOCK

    refreshStartPage({"load", {TXT_CHECKING_UPD}, BTN_TXT_CHECK, "check", "true"});
    m_manualCheck = manualCheck;
    m_packageData->clear();

#ifdef CHECK_DIRECTORY
    if (QFileInfo(qApp->applicationDirPath()).baseName() != QString(REG_APP_NAME)) {
        refreshStartPage({"error", {TXT_ERR_NOT_ALLOWED}});
        m_dialogSchedule->addToSchedule("criticalMsg", tr("This folder configuration does not allow for "
                       "updates! The folder name should be: ") + QString(REG_APP_NAME));
        return;
    }
#endif
    QString json = QString("{\"currVersion\":\"%1\",\"ignVersion\":\"%2\",\"package\":\"%3\",\"fileName\":\"%4\"}").
                   arg(QString::fromLatin1(VER_FILEVERSION_STR), ignoredVersion(), packageToStr(), m_savedPackageData->fileName);
    if (!m_socket->sendMessage(MSG_CheckUpdates, QStrToTStr(json))) {
        refreshStartPage({"error", {TXT_ERR_CHECK}, BTN_TXT_CHECK, "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while check updates: Update Service not found!"));
    }
}

void CUpdateManager::updateNeededCheking()
{
    if (m_pIntervalTimer) {
        m_pIntervalTimer->stop();
        int elapsed_time = int(time(nullptr) - m_lastCheck);
        if (elapsed_time > m_interval) {
            checkUpdates();
        } else {
            int remaining_time = 1000 * (m_interval - elapsed_time);
            m_pIntervalTimer->setInterval(remaining_time < CHECK_ON_STARTUP_MS + 1000 ? CHECK_ON_STARTUP_MS + 1000 : remaining_time);
            m_pIntervalTimer->start();
        }
    }
}

void CUpdateManager::onProgressSlot(const int percent)
{
    refreshStartPage({"", {TXT_DOWNLOADING_UPD, m_packageData->version, QString::number(percent)}});
}

void CUpdateManager::onUnzipProgressSlot(const int percent)
{
    refreshStartPage({"", {TXT_UNZIP_UPD, QString::number(percent)}});
}

void CUpdateManager::onError(const QString &error)
{
    const char *_error = SVC_TXT_ERR_OTHER;
    if (error == "SVC_TXT_ERR_UNPACKING")
        _error = SVC_TXT_ERR_UNPACKING;
    else
    if (error == "SVC_TXT_ERR_DNL_OUT_MEM")
        _error = SVC_TXT_ERR_DNL_OUT_MEM;
    else
    if (error == "SVC_TXT_ERR_DNL_CONN")
        _error = SVC_TXT_ERR_DNL_CONN;
    else
    if (error == "SVC_TXT_ERR_DNL_URL")
        _error = SVC_TXT_ERR_DNL_URL;
    else
    if (error == "SVC_TXT_ERR_DNL_CREAT")
        _error = SVC_TXT_ERR_DNL_CREAT;
    else
    if (error == "SVC_TXT_ERR_DNL_INET")
        _error = SVC_TXT_ERR_DNL_INET;
    else
    if (error == "SVC_TXT_ERR_MD5")
        _error = TXT_ERR_MD5;
    else
    if (error == "SVC_TXT_ERR_URL")
        _error = TXT_ERR_URL;

    refreshStartPage({"error", {_error}, BTN_TXT_CHECK, "check", "false"});
    __UNLOCK
//    m_dialogSchedule->addToSchedule("criticalMsg", error);
}

void CUpdateManager::savePackageData(const QString &version, const QString &fileName, const QString &fileType)
{
    m_savedPackageData->fileName = fileName;
    m_savedPackageData->fileType = fileType;
    m_savedPackageData->version = version;
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("file", fileName);
    reg_user.setValue("type", fileType);
    reg_user.setValue("version", version);
    reg_user.endGroup();
}

QString CUpdateManager::ignoredVersion()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    const QString ignored_ver = reg_user.value("ignored_ver").toString();
    reg_user.endGroup();
    return ignored_ver;
}

bool CUpdateManager::isSavedPackageValid()
{
    return (m_savedPackageData->fileName.indexOf(currentArch()) != -1
                && m_savedPackageData->fileType == m_packageData->fileType
                && m_savedPackageData->version == m_packageData->version
                && getFileHash(m_savedPackageData->fileName) == m_packageData->hash);
}

bool CUpdateManager::isVersionBHigherThanA(const QString &a, const QString &b)
{
    const QStringList old_ver = a.split('.');
    const QStringList new_ver = b.split('.');
    for (int i = 0; i < std::min(new_ver.size(), old_ver.size()); i++) {
        if (new_ver.at(i).toInt() > old_ver.at(i).toInt()) {
            return true;
        } else
        if (new_ver.at(i).toInt() < old_ver.at(i).toInt())
            break;
    }
    return false;
}

void CUpdateManager::loadUpdates()
{
    __GLOBAL_LOCK

    if (isSavedPackageValid()) {
        m_packageData->fileName = m_savedPackageData->fileName;
        if (m_packageData->fileType == "archive") {
            __UNLOCK
            unzipIfNeeded();
        } else {
            refreshStartPage({"success", {TXT_RESTART_TO_UPD}, BTN_TXT_RESTART, "install", "false"});
            m_dialogSchedule->addToSchedule("showStartInstallMessage");
        }

    } else
    if (!m_packageData->packageUrl.empty()) {
        if (!m_socket->sendMessage(MSG_LoadUpdates)) {
            refreshStartPage({"error", {TXT_ERR_LOAD}, BTN_TXT_CHECK, "check", "false"});
            __UNLOCK
//            m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while loading updates: Update Service not found!"));
        } else {
            refreshStartPage({"load", {TXT_DOWNLOADING_UPD, m_packageData->version, "0"}, BTN_TXT_CANCEL, "abort", "false"});
        }
    } else {
        refreshStartPage({"error", {TXT_ERR_PACK_URL}, BTN_TXT_CHECK, "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while loading updates: package Url is empty!"));
    }
}

void CUpdateManager::installUpdates()
{
    __UNLOCK
    m_startUpdateOnClose = true;
    m_restartAfterUpdate = true;
    AscAppManager::closeAppWindows();
}

void CUpdateManager::refreshStartPage(const Command &cmd)
{
    if (m_pLastCheckMsgTimer)
        m_pLastCheckMsgTimer->stop();
    static bool lock = true;
    QJsonObject jsn, btn_jsn;
    if (cmd.isEmpty()) {
        if (lock)
            lock = false;
        if (!m_lastCommand.icon.isEmpty())
            jsn["icon"] = m_lastCommand.icon;
        if (m_lastCommand.text.text != nullptr)
            jsn["text"] = tr(m_lastCommand.text.text, "CUpdateManager").arg(m_lastCommand.text.arg1, m_lastCommand.text.arg2);
        if (m_lastCommand.btn_text != nullptr)
            btn_jsn["text"] = tr(m_lastCommand.btn_text, "CUpdateManager");
        if (!m_lastCommand.btn_action.isEmpty())
            btn_jsn["action"] = m_lastCommand.btn_action;
        if (!m_lastCommand.btn_lock.isEmpty())
            btn_jsn["lock"] = m_lastCommand.btn_lock;
    } else {
        if (!cmd.icon.isEmpty()) {
            m_lastCommand.icon = cmd.icon;
            jsn["icon"] = cmd.icon;
        }
        if (cmd.text.text != nullptr) {
            m_lastCommand.text = cmd.text;
            jsn["text"] = tr(cmd.text.text, "CUpdateManager").arg(cmd.text.arg1, cmd.text.arg2);
        }
        if (cmd.btn_text != nullptr) {
            m_lastCommand.btn_text = cmd.btn_text;
            btn_jsn["text"] = tr(cmd.btn_text, "CUpdateManager");
        }
        if (!cmd.btn_action.isEmpty()) {
            m_lastCommand.btn_action = cmd.btn_action;
            btn_jsn["action"] = cmd.btn_action;
        }
        if (!cmd.btn_lock.isEmpty()) {
            m_lastCommand.btn_lock = cmd.btn_lock;
            btn_jsn["lock"] = cmd.btn_lock;
        }
    }
    if (lock)
        return;
    if (!btn_jsn.isEmpty())
        jsn["button"] = btn_jsn;
    if (!jsn.isEmpty())
        AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "updates:status", Utils::stringifyJson(jsn));
}

void CUpdateManager::launchIntervalStartTimer()
{
    if (m_pIntervalStartTimer && getUpdateMode() != UpdateMode::DISABLE)
        m_pIntervalStartTimer->start();
}

void CUpdateManager::setServiceLang(QString lang)
{
    if (lang.isEmpty())
        lang = CLangater::getLangName();
    lang.replace('-', '_');
    if (!m_socket->sendMessage(MSG_SetLanguage, QStrToTStr(lang)))
        CLogger::log("Cannot set service language to: " + lang);
}

void CUpdateManager::onLoadUpdateFinished(const QString &filePath)
{
    if (getFileHash(filePath) != m_packageData->hash) {
        refreshStartPage({"error", {TXT_ERR_MD5}, BTN_TXT_CHECK, "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", "Update package error: md5 sum does not match the original.");
        return;
    }
    m_packageData->fileName = filePath;
    savePackageData(m_packageData->version, filePath, m_packageData->fileType);
    if (m_packageData->fileType == "archive") {
        __UNLOCK
        unzipIfNeeded();
    } else {
        refreshStartPage({"success", {TXT_RESTART_TO_UPD}, BTN_TXT_RESTART, "install", "false"});
        m_dialogSchedule->addToSchedule("showStartInstallMessage");
    }
}

void CUpdateManager::unzipIfNeeded()
{
    __GLOBAL_LOCK

    refreshStartPage({"load", {TXT_PREPARING_UPD}, BTN_TXT_CANCEL, "abort", "true"});
    if (!m_socket->sendMessage(MSG_UnzipIfNeeded)) {
        refreshStartPage({"error", {TXT_ERR_UNZIP}, BTN_TXT_CHECK, "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while unzip updates: Update Service not found!"));
    }
}

void CUpdateManager::handleAppClose()
{
    if ( m_startUpdateOnClose ) {
#ifdef _WIN32
        if (m_packageData->fileType != "archive") {
            if (!m_socket->sendMessage(MSG_StartInstallPackage)) {
                criticalMsg(nullptr, QObject::tr("An error occurred while start install updates: Update Service not found!"));
            }
        } else {
            if (!Utils::isSessionInProgress()) {
                CLogger::log("Update skipped: session is being terminated.");
                return;
            }
#endif
            int cmd = (m_packageData->object == "app") ? MSG_StartReplacingFiles : MSG_StartReplacingService;
            if (!m_socket->sendMessage(cmd, QStrToTStr(packageToStr()), m_restartAfterUpdate ? _T("true") : _T("false"))) {
                criticalMsg(nullptr, QObject::tr("An error occurred while start replacing files: Update Service not found!"));
            }
#ifdef _WIN32
        }
#endif
    } else
        m_socket->sendMessage(MSG_StopDownload);
}

void CUpdateManager::setNewUpdateSetting(const QString& _rate)
{
    GET_REGISTRY_USER(reg_user);
    reg_user.setValue("autoUpdateMode", _rate);
    if (modeToEnum(_rate) == UpdateMode::DISABLE) {
        if (m_pIntervalStartTimer)
            m_pIntervalStartTimer->stop();
        if (m_pIntervalTimer)
            m_pIntervalTimer->stop();
    } else {
        if (m_pIntervalStartTimer)
            m_pIntervalStartTimer->start();
    }
}

void CUpdateManager::cancelLoading()
{
    refreshStartPage({"lastcheck", {TXT_LAST_CHECK, formattedTime(m_lastCheck)}, BTN_TXT_CHECK, "check", "false"});
    m_socket->sendMessage(MSG_StopDownload);
    __UNLOCK
}

void CUpdateManager::skipVersion()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("ignored_ver", m_packageData->version);
    reg_user.endGroup();
}

int CUpdateManager::getUpdateMode()
{
    GET_REGISTRY_USER(reg_user);
    return modeToEnum(reg_user.value("autoUpdateMode", "ask").toString());
}

void CUpdateManager::onLoadCheckFinished(const QString &json)
{
    m_manualCheck = true;
    if (!json.isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError) {
            QJsonObject root = doc.object();
            if (!root.isEmpty()) {
                m_packageData->object = root.value("object").toString();
                m_packageData->version = root.value("version").toString();
                m_packageData->fileType = root.value("fileType").toString();
                m_packageData->packageUrl = root.value("packageUrl").toString().toStdWString();
                m_packageData->hash = root.value("hash").toString();
                m_packageData->isInstallable = root.value("isInstallable").toBool();

                clearTempFiles(m_packageData->isInstallable && isSavedPackageValid() ? m_savedPackageData->fileName : "");
                if (m_packageData->packageUrl.empty() || !m_socket->sendMessage(MSG_RequestContentLenght)) {
                    m_packageData->fileSize = "--";
                    onCheckFinished(false, true, m_packageData->version, "");
                }
            } else {
                clearTempFiles();
                onCheckFinished(false, false, "", "");
            }
        } else {
            onCheckFinished(true, false, "", "");
        }
    } else {
        onCheckFinished(true, false, "", "");
    }
}

void CUpdateManager::onCheckFinished(bool error, bool updateExist, const QString &version, const QString &changelog)
{
    if ( !error) {
        if ( updateExist ) {
//            if (m_packageData->object == "svc") {
//                __UNLOCK
//                loadUpdates();
//                return;
//            } else
            if (!m_packageData->isInstallable) {
                refreshStartPage({"lastcheck", {m_packageData->object == "svc" ? TXT_AVAILABLE_SVC : TXT_AVAILABLE_UPD, version}, BTN_TXT_CHECK, "check", "false"});
                m_dialogSchedule->addToSchedule("showUpdateMessage");
                return;
            }
            switch (getUpdateMode()) {
            case UpdateMode::SILENT:
                __UNLOCK
                loadUpdates();
                break;
            case UpdateMode::ASK:
            case UpdateMode::DISABLE:
                if (isSavedPackageValid()) {
                    __UNLOCK
                    loadUpdates();
                } else {
                    refreshStartPage({"lastcheck", {m_packageData->object == "svc" ? TXT_AVAILABLE_SVC : TXT_AVAILABLE_UPD, version}, BTN_TXT_DOWNLOAD, "download", "false"});
                    m_dialogSchedule->addToSchedule("showUpdateMessage");
                }
                break;
            }
        } else {
            refreshStartPage({"success", {TXT_UPDATED}, BTN_TXT_CHECK, "check", "false"});
            m_pLastCheckMsgTimer->start();
            __UNLOCK;
        }
    } else {
        refreshStartPage({"error", {TXT_ERR_JSON}, BTN_TXT_CHECK, "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", changelog);
    }
}

void CUpdateManager::showUpdateMessage(QWidget *parent, bool forceModal, int result) {
    if (result == DLG_RESULT_NONE) {
        QString name = (m_packageData->object == "app") ? QString(WINDOW_NAME) : QString(SERVICE_NAME);
        QString curr_version = (m_packageData->object == "app") ? QString(VER_FILEVERSION_STR) :
                                   getFileVersion(QStrToTStr(qApp->applicationDirPath()) + DAEMON_NAME);
        QString text = m_packageData->isInstallable ? tr("Would you like to download update now?") :
                           tr("The current version does not support installing this update directly. "
                              "To install updates, you can download the required package from the official website.");
        QString title = tr("Update is available");
        if (!forceModal && AscAppManager::notificationSupported()) {
            if (CNotification::instance().show(title,
                        QString("%1\n%2: %3\n%4: %5").arg(name, tr("Current version"),
                        curr_version, tr("New version"), m_packageData->version),
                        MsgBtns::mbSkipRemindDownload, [=](int res) {
                            QMetaObject::invokeMethod(this, "showUpdateMessage", Qt::QueuedConnection, Q_ARG(QWidget*, parent), Q_ARG(bool, res == NOTIF_FAILED), Q_ARG(int, res));
                        })) {
                __UNLOCK
                return;
            }
        }

        QString content = QString("%1\n%2: %3\n%4: %5\n%6 (%7 MB)")
                              .arg(name, tr("Current version"), curr_version, tr("New version"), m_packageData->version, text, m_packageData->fileSize);
        CMessageOpts opts;
        opts.contentText = QString("%1\n").arg(content);
        if (!QString(RELEASE_NOTES).isEmpty())
            opts.linkText =  LINK_TEXT;
        result = CMessage::showMessage(parent, title, MsgType::MSG_BRAND, MsgBtns::mbSkipRemindDownload, opts);
        __UNLOCK
    }
    switch (result) {
    case MsgRes::MODAL_RESULT_DOWNLOAD:
        if (m_packageData->isInstallable)
            loadUpdates();
        else
            Utils::openUrl(DOWNLOAD_PAGE);
        break;
    case MsgRes::MODAL_RESULT_SKIPVER: {
        skipVersion();
        refreshStartPage({"success", {TXT_UPDATED}, BTN_TXT_CHECK, "check", "false"});
        m_pLastCheckMsgTimer->start();
        break;
    }
    default:
        break;
    }
}

void CUpdateManager::showStartInstallMessage(QWidget *parent, bool forceModal, int result)
{
    if (result == DLG_RESULT_NONE) {
        QString name = (m_packageData->object == "app") ? QString(WINDOW_NAME) : QString(SERVICE_NAME);
        QString curr_version = (m_packageData->object == "app") ? QString(VER_FILEVERSION_STR) :
                                   getFileVersion(QStrToTStr(qApp->applicationDirPath()) + DAEMON_NAME);
        QString title = tr("Update is ready to install");
        if (!forceModal && AscAppManager::notificationSupported()) {
            if (CNotification::instance().show(title,
                        QString("%1\n%2: %3\n%4: %5").arg(name, tr("Current version"),
                        curr_version, tr("New version"), m_packageData->version),
                        MsgBtns::mbInslaterRestart, [=](int res) {
                            QMetaObject::invokeMethod(this, "showStartInstallMessage", Qt::QueuedConnection, Q_ARG(QWidget*, parent), Q_ARG(bool, res == NOTIF_FAILED), Q_ARG(int, res));
                        })) {
                __UNLOCK
                return;
            }
        }

        QString content = QString("%1\n%2: %3\n%4: %5\n%6")
                              .arg(name, tr("Current version"), curr_version, tr("New version"), m_packageData->version,
                                   tr("To finish updating, restart the app"));
        CMessageOpts opts;
        opts.contentText = QString("%1\n").arg(content);
        if (!QString(RELEASE_NOTES).isEmpty())
            opts.linkText =  LINK_TEXT;
        result = CMessage::showMessage(parent, title, MsgType::MSG_BRAND, MsgBtns::mbInslaterRestart, opts);
         __UNLOCK
    }
    switch (result) {
    case MsgRes::MODAL_RESULT_RESTART: {
        m_startUpdateOnClose = true;
        m_restartAfterUpdate = true;
        AscAppManager::closeAppWindows();
        break;
    }
    case MsgRes::MODAL_RESULT_INSLATER: {
#ifdef _WIN32
        m_startUpdateOnClose = (m_packageData->fileType == "archive");
#else
        m_startUpdateOnClose = false;
#endif
        m_restartAfterUpdate = false;
        break;
    }
//    case WinDlg::DLG_RESULT_SKIP: {
//        skipVersion();
//        AscAppManager::sendCommandTo(0, "updates:link", "lock");
//        AscAppManager::sendCommandTo(0, "updates:checking", "{\"version\":\"no\"}");
//        break;
//    }
    default:
        m_startUpdateOnClose = false;
        m_restartAfterUpdate = false;
        break;
    }
}

#include "cupdatemanager.moc"
