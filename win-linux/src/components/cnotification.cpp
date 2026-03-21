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

#ifdef _WIN32
# include <QCoreApplication>
# include <QDir>
# include "utils.h"
# define APP_LAUNCH_NAME "\\" REG_APP_NAME ".exe"
# define APP_SHORTCUT_NAME "\\" APP_REG_NAME ".lnk"
#else
# include <libnotify/notify.h>
# include <gio/gio.h>
# include <glib.h>
# include <unordered_map>
# include "utils.h"
# include <QDBusConnection>
# include <QDBusInterface>
# include <QDBusMessage>
# include <QDBusVariant>
#endif
#include <QTextDocumentFragment>
#include "components/cnotification.h"
#include "defines.h"

#define NOTIF_TIMEOUT_MS 10000
#ifdef __linux__
# define toTStr(qstr)     qstr.toStdString()
#else
# define toTStr(qstr)     qstr.toStdWString()
#endif

#define TEXT_CANCEL      toTStr(QObject::tr("Cancel"))
#define TEXT_YES         toTStr(QObject::tr("Yes"))
#define TEXT_NO          toTStr(QObject::tr("No"))
#define TEXT_OK          toTStr(QObject::tr("OK"))
#define TEXT_SKIP        toTStr(QObject::tr("Skip"))
#define TEXT_BUY         toTStr(QObject::tr("Buy Now"))
#define TEXT_ACTIVATE    toTStr(QObject::tr("Activate"))
#define TEXT_CONTINUE    toTStr(QObject::tr("Continue"))
#define TEXT_SKIPVER     toTStr(QObject::tr("Skip"))
#define TEXT_REMIND      toTStr(QObject::tr("Later"))
#define TEXT_INSTALL     toTStr(QObject::tr("Install"))
#define TEXT_INSLATER    toTStr(QObject::tr("Later"))
#define TEXT_RESTART     toTStr(QObject::tr("Restart"))
#define TEXT_SAVEANDINS  toTStr(QObject::tr("Install"))
#define TEXT_DOWNLOAD    toTStr(QObject::tr("Download"))

#ifdef __linux__
# define addAction(action, label) notify_notification_add_action(ntf, action, label, NOTIFY_ACTION_CALLBACK(action_callback), (void*)&pimpl->ntfMap, NULL);

typedef std::unordered_map<NotifyNotification*, FnVoidInt> NtfMap;

static void action_callback(NotifyNotification *ntf, char *action, void *data)
{
    if (!data)
        return;
    int res = strcmp(action, "cancel") == 0 ?   MODAL_RESULT_CANCEL :
              strcmp(action, "yes") == 0 ?      MODAL_RESULT_YES :
              strcmp(action, "no") == 0 ?       MODAL_RESULT_NO :
              strcmp(action, "ok") == 0 ?       MODAL_RESULT_OK :
              strcmp(action, "skip") == 0 ?     MODAL_RESULT_SKIP :
              strcmp(action, "buy") == 0 ?      MODAL_RESULT_BUY :
              strcmp(action, "activate") == 0 ? MODAL_RESULT_ACTIVATE :
              strcmp(action, "continue") == 0 ? MODAL_RESULT_CONTINUE :
              strcmp(action, "inslater") == 0 ? MODAL_RESULT_INSLATER :
              strcmp(action, "restart") == 0 ?  MODAL_RESULT_RESTART :
              strcmp(action, "skipver") == 0 ?  MODAL_RESULT_SKIPVER :
              strcmp(action, "remind") == 0 ?   MODAL_RESULT_REMIND :
              strcmp(action, "install") == 0 ?  MODAL_RESULT_INSTALL :
              strcmp(action, "saveins") == 0 ?  MODAL_RESULT_INSTALL :
              strcmp(action, "download") == 0 ? MODAL_RESULT_DOWNLOAD : -1;

    NtfMap *ntfMap = (NtfMap*)data;
    if (ntfMap->find(ntf) != ntfMap->end()) {
        if (FnVoidInt callback = ntfMap->at(ntf))
            callback(res);
    }
}

static void on_close(NotifyNotification *ntf, void *data)
{
    if (!data)
        return;
    NtfMap *ntfMap = (NtfMap*)data;
    auto it = ntfMap->find(ntf);
    if (it != ntfMap->end()) {
        g_object_unref(ntf);
        ntfMap->erase(it);
    }
}

static bool isNotificationsEnabled()
{
    if (WindowHelper::getEnvInfo() == WindowHelper::GNOME) {
        GSettings *stn = g_settings_new("org.gnome.desktop.notifications");
        GVariant *var = g_settings_get_value(stn, "show-banners");
        gboolean res = false;
        g_variant_get(var, "b", &res);
        g_object_unref(var);
        g_object_unref(stn);
        return res;
    } else
    if (WindowHelper::getEnvInfo() == WindowHelper::CINNAMON) {
        GSettings *stn = g_settings_new("org.cinnamon.desktop.notifications");
        GVariant *var = g_settings_get_value(stn, "display-notifications");
        gboolean res = false;
        g_variant_get(var, "b", &res);
        g_object_unref(var);
        g_object_unref(stn);
        return res;
    } else
    if (WindowHelper::getEnvInfo() == WindowHelper::KDE) {
        QDBusConnection conn = QDBusConnection::sessionBus();
        if (conn.isConnected()) {
            QDBusInterface itf("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.DBus.Properties", conn);
            if (itf.isValid()) {
                QDBusMessage msg = itf.call("Get", "org.freedesktop.Notifications", "Inhibited");
                if (msg.type() == QDBusMessage::ReplyMessage && msg.arguments().size() > 0) {
                    QVariant var = msg.arguments().at(0);
                    if (var.canConvert<QDBusVariant>()) {
                        QVariant res = var.value<QDBusVariant>().variant();
                        if (res.type() == QVariant::Bool && !res.toBool())
                            return true;
                    }
                }
            }
        }
    } else
    if (WindowHelper::getEnvInfo() == WindowHelper::XFCE) {
        QDBusConnection conn = QDBusConnection::sessionBus();
        if (conn.isConnected()) {
            QDBusInterface itf("org.xfce.Xfconf", "/org/xfce/Xfconf", "org.xfce.Xfconf", conn);
            if (itf.isValid()) {
                QDBusMessage msg = itf.call("GetProperty",  "xfce4-notifyd", "/do-not-disturb");
                if (msg.type() == QDBusMessage::ErrorMessage)
                    return true; // By default the property is not defined and notifications are enabled
                if (msg.type() == QDBusMessage::ReplyMessage && msg.arguments().size() > 0) {
                    QVariant var = msg.arguments().at(0);
                    if (var.canConvert<QDBusVariant>()) {
                        QVariant res = var.value<QDBusVariant>().variant();
                        if (res.type() == QVariant::Bool && !res.toBool())
                            return true;
                    }
                }
            }
        }
    }
    return false;
}
#else
# include "platform_win/wintoastlib.h"

using namespace WinToastLib;

class ToastHandler : public IWinToastHandler {
public:
    ToastHandler(MsgBtns _dlgBtns, FnVoidInt _callback) : dlgBtns(_dlgBtns), callback(_callback)
    {}
    virtual void toastActivated() const final // The user clicked in this toast
    {}
    virtual void toastActivated(int actionIndex) const final // The user clicked on button #actionIndex
    {
        int res = -1;
        switch (actionIndex) {
        case 0: {
            switch (dlgBtns) {
            case MsgBtns::mbOk:
            case MsgBtns::mbOkCancel:
            case MsgBtns::mbOkDefCancel:
                res = MODAL_RESULT_OK;
                break;
            case MsgBtns::mbYesNo:
            case MsgBtns::mbYesDefNo:
            case MsgBtns::mbYesNoCancel:
            case MsgBtns::mbYesDefNoCancel:
            case MsgBtns::mbYesDefSkipNo:
                res = MODAL_RESULT_YES;
                break;
            case MsgBtns::mbBuy:
                res = MODAL_RESULT_BUY;
                break;
            case MsgBtns::mbActivateDefContinue:
                res = MODAL_RESULT_ACTIVATE;
                break;
            case MsgBtns::mbContinue:
                res = MODAL_RESULT_CONTINUE;
                break;
            case MsgBtns::mbInslaterRestart:
                res = MODAL_RESULT_INSLATER;
                break;
            case MsgBtns::mbSkipRemindInstall:
            case MsgBtns::mbSkipRemindSaveandinstall:
            case MsgBtns::mbSkipRemindDownload:
                res = MODAL_RESULT_SKIPVER;
                break;
            default:
                break;
            }
            break;
        }
        case 1: {
            switch (dlgBtns) {
            case MsgBtns::mbOkCancel:
            case MsgBtns::mbOkDefCancel:
                res = MODAL_RESULT_CANCEL;
                break;
            case MsgBtns::mbYesNo:
            case MsgBtns::mbYesDefNo:
            case MsgBtns::mbYesNoCancel:
            case MsgBtns::mbYesDefNoCancel:
                res = MODAL_RESULT_NO;
                break;
            case MsgBtns::mbYesDefSkipNo:
                res = MODAL_RESULT_SKIP;
                break;
            case MsgBtns::mbActivateDefContinue:
                res = MODAL_RESULT_CONTINUE;
                break;
            case MsgBtns::mbInslaterRestart:
                res = MODAL_RESULT_RESTART;
                break;
            case MsgBtns::mbSkipRemindInstall:
            case MsgBtns::mbSkipRemindSaveandinstall:
            case MsgBtns::mbSkipRemindDownload:
                res = MODAL_RESULT_REMIND;
                break;
            default:
                break;
            }
            break;
        }
        case 2: {
            switch (dlgBtns) {
            case MsgBtns::mbYesNoCancel:
            case MsgBtns::mbYesDefNoCancel:
                res = MODAL_RESULT_CANCEL;
                break;
            case MsgBtns::mbYesDefSkipNo:
                res = MODAL_RESULT_NO;
                break;
            case MsgBtns::mbSkipRemindDownload:
                res = MODAL_RESULT_DOWNLOAD;
                break;
            case MsgBtns::mbSkipRemindInstall:
            case MsgBtns::mbSkipRemindSaveandinstall:
                res = MODAL_RESULT_INSTALL;
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        if (callback)
            callback(res);
    }
    virtual void toastActivated(const char* response) const final
    {}
    virtual void toastFailed() const final // Error showing current toast
    {
        if (callback)
            callback(NOTIF_FAILED);
    }
    virtual void toastDismissed(WinToastDismissalReason state) const final
    {
        switch (state) {
        case UserCanceled: // The user dismissed this toast
            break;
        case ApplicationHidden: // The application hide the toast using ToastNotifier.hide()
            break;
        case TimedOut: // The toast has timed out
            break;
        default: // Toast not activated
            break;
        }
    }

private:
    MsgBtns dlgBtns;
    FnVoidInt callback;
};
#endif

class CNotification::CNotificationPrivate
{
public:
#ifdef __linux__
    ~CNotificationPrivate()
    {
        clear();
    }
    void clear()
    {
        for (auto it = ntfMap.begin(); it != ntfMap.end();) {
            NotifyNotification *ntf = it->first;
            it = ntfMap.erase(it);
            notify_notification_close(ntf, NULL);
            g_object_unref(ntf);
        }
    }

    NtfMap ntfMap;
#endif
    int isInit = -1;
};


CNotification& CNotification::instance()
{
    static CNotification inst;
    return inst;
}

CNotification::CNotification() :
    pimpl(new CNotificationPrivate)
{}

CNotification::~CNotification()
{
    delete pimpl, pimpl = nullptr;
#ifdef __linux__
    if (notify_is_initted())
        notify_uninit();
#endif
}

bool CNotification::init()
{
    if (pimpl->isInit != -1)
        return pimpl->isInit;

#ifdef __linux__
    pimpl->isInit = notify_init(WINDOW_TITLE);
#else
    if (Utils::getWinVersion() < Utils::WinVer::Win10) {
        pimpl->isInit = 0;
        return false;
    }
    WinToast::instance()->setAppName(TEXT(WINDOW_TITLE));
    WinToast::instance()->setAppUserModelId(TEXT(APP_USER_MODEL_ID));
    if (IsPackage(Portable)) {
        WinToast::instance()->setShortcutPolicy(WinToastLib::WinToast::SHORTCUT_POLICY_IGNORE);
    } else {
        WinToast::instance()->setShortcutPolicy(WinToastLib::WinToast::SHORTCUT_POLICY_REQUIRE_NO_CREATE);
        PWSTR progPath = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, nullptr, &progPath))) {
            std::wstring shortcutPath(progPath);
            shortcutPath.append(L"\\");
            shortcutPath.append(TEXT(APP_REG_NAME));
            shortcutPath.append(TEXT(APP_SHORTCUT_NAME));
            WinToast::instance()->setShortcutPath(shortcutPath);
        }
        CoTaskMemFree(progPath);
    }
    pimpl->isInit = WinToast::instance()->initialize();
#endif
    return pimpl->isInit;
}

void CNotification::clear()
{
    if (pimpl->isInit == 1)
#ifdef __linux__
        pimpl->clear();
#else
        WinToast::instance()->clear();
#endif
}

bool CNotification::show(const QString &msg, const QString &content, MsgBtns dlgBtns, const FnVoidInt &callback)
{
#ifdef __linux__
    if (!isNotificationsEnabled())
        return false;
    QString lpText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    NotifyNotification *ntf = notify_notification_new(lpText.toLocal8Bit().data(), content.toLocal8Bit().data(), NULL);
    g_signal_connect(G_OBJECT(ntf), "closed", G_CALLBACK(on_close), (void*)&pimpl->ntfMap);
    notify_notification_set_urgency(ntf, NotifyUrgency::NOTIFY_URGENCY_NORMAL);
    notify_notification_set_timeout(ntf, NOTIF_TIMEOUT_MS);
    pimpl->ntfMap[ntf] = callback;

    if (callback) {
        switch (dlgBtns) {
        case MsgBtns::mbYesNo:
        case MsgBtns::mbYesDefNo:
            addAction("yes", TEXT_YES.c_str());
            addAction("no", TEXT_NO.c_str());
            break;
        case MsgBtns::mbYesNoCancel:
        case MsgBtns::mbYesDefNoCancel:
            addAction("yes", TEXT_YES.c_str());
            addAction("no", TEXT_NO.c_str());
            addAction("cancel", TEXT_CANCEL.c_str());
            break;
        case MsgBtns::mbOkCancel:
        case MsgBtns::mbOkDefCancel:
            addAction("ok", TEXT_OK.c_str());
            addAction("cancel", TEXT_CANCEL.c_str());
            break;
        case MsgBtns::mbYesDefSkipNo:
            addAction("yes", TEXT_YES.c_str());
            addAction("skip", TEXT_SKIP.c_str());
            addAction("no", TEXT_NO.c_str());
            break;
        case MsgBtns::mbBuy:
            addAction("buy", TEXT_BUY.c_str());
            break;
        case MsgBtns::mbActivateDefContinue:
            addAction("activate", TEXT_ACTIVATE.c_str());
            addAction("continue", TEXT_CONTINUE.c_str());
            break;
        case MsgBtns::mbContinue:
            addAction("continue", TEXT_CONTINUE.c_str());
            break;
        case MsgBtns::mbInslaterRestart:
            addAction("inslater", TEXT_INSLATER.c_str());
            addAction("restart", TEXT_RESTART.c_str());
            break;
        case MsgBtns::mbSkipRemindInstall:
            addAction("skipver", TEXT_SKIPVER.c_str());
            addAction("remind", TEXT_REMIND.c_str());
            addAction("install", TEXT_INSTALL.c_str());
            break;
        case MsgBtns::mbSkipRemindSaveandinstall:
            addAction("skipver", TEXT_SKIPVER.c_str());
            addAction("remind", TEXT_REMIND.c_str());
            addAction("saveins", TEXT_SAVEANDINS.c_str());
            break;
        case MsgBtns::mbSkipRemindDownload:
            addAction("skipver", TEXT_SKIPVER.c_str());
            addAction("remind", TEXT_REMIND.c_str());
            addAction("download", TEXT_DOWNLOAD.c_str());
            break;
        default:
            break;
        }
    }
    return notify_notification_show(ntf, NULL);
#else
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    std::wstring lpContent = content.toStdWString();

    WinToastTemplate tmpl(WinToastTemplate::WinToastTemplateType::ImageAndText02);
    tmpl.setTextField(lpText, WinToastTemplate::FirstLine);
    tmpl.setTextField(lpContent, WinToastTemplate::SecondLine);
    const QString appIcon = QCoreApplication::applicationDirPath() + "/app.ico";
    if (QFileInfo::exists(appIcon))
        tmpl.setImagePath(appIcon.toStdWString());
    tmpl.setAudioPath(WinToastTemplate::AudioSystemFile::Mail);
    tmpl.setDuration(WinToastTemplate::Duration::System);
    tmpl.setExpiration(NOTIF_TIMEOUT_MS);

    switch (dlgBtns) {
    case MsgBtns::mbYesNo:
    case MsgBtns::mbYesDefNo:
        tmpl.addAction(TEXT_YES);
        tmpl.addAction(TEXT_NO);
        break;
    case MsgBtns::mbYesNoCancel:
    case MsgBtns::mbYesDefNoCancel:
        tmpl.addAction(TEXT_YES);
        tmpl.addAction(TEXT_NO);
        tmpl.addAction(TEXT_CANCEL);
        break;
    case MsgBtns::mbOkCancel:
    case MsgBtns::mbOkDefCancel:
        tmpl.addAction(TEXT_OK);
        tmpl.addAction(TEXT_CANCEL);
        break;
    case MsgBtns::mbYesDefSkipNo:
        tmpl.addAction(TEXT_YES);
        tmpl.addAction(TEXT_SKIP);
        tmpl.addAction(TEXT_NO);
        break;
    case MsgBtns::mbBuy:
        tmpl.addAction(TEXT_BUY);
        break;
    case MsgBtns::mbActivateDefContinue:
        tmpl.addAction(TEXT_ACTIVATE);
        tmpl.addAction(TEXT_CONTINUE);
        break;
    case MsgBtns::mbContinue:
        tmpl.addAction(TEXT_CONTINUE);
        break;
    case MsgBtns::mbInslaterRestart:
        tmpl.addAction(TEXT_INSLATER);
        tmpl.addAction(TEXT_RESTART);
        break;
    case MsgBtns::mbSkipRemindInstall:
        tmpl.addAction(TEXT_SKIPVER);
        tmpl.addAction(TEXT_REMIND);
        tmpl.addAction(TEXT_INSTALL);
        break;
    case MsgBtns::mbSkipRemindSaveandinstall:
        tmpl.addAction(TEXT_SKIPVER);
        tmpl.addAction(TEXT_REMIND);
        tmpl.addAction(TEXT_SAVEANDINS);
        break;
    case MsgBtns::mbSkipRemindDownload:
        tmpl.addAction(TEXT_SKIPVER);
        tmpl.addAction(TEXT_REMIND);
        tmpl.addAction(TEXT_DOWNLOAD);
        break;
    default:
        break;
    }
    return WinToast::instance()->showToast(tmpl, new ToastHandler(dlgBtns, callback)) > -1;
#endif
}
