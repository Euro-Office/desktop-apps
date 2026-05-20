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

#include "cascapplicationmanagerwrapper.h"
#include "singleapplication.h"
#include "csocket.h"
#include "utils.h"
#include "defines.h"
#include <QSessionManager>


static const char *GNOME_SESSION_NAME     = "org.gnome.SessionManager",
                  *GNOME_SESSION_ITF      = GNOME_SESSION_NAME,
                  *GNOME_SESSION_CLNT_ITF = "org.gnome.SessionManager.ClientPrivate",
                  *GNOME_SESSION_PATH     = "/org/gnome/SessionManager";

static GVariant* callMethod(const gchar *obj_path, const gchar *interface, const gchar *method, GVariant *args)
{
    GVariant *result = nullptr;
    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, GNOME_SESSION_NAME, obj_path, interface, nullptr, nullptr);
    if (proxy) {
        result = g_dbus_proxy_call_sync(proxy, method, args, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr);
        g_object_unref(proxy);
    }
    return result;
}

static std::string registerClient()
{
    std::string result;
    GVariant *args = g_variant_new("(ss)", DESKTOP_FILE_NAME, "");
    g_variant_ref_sink(args);
    if (GVariant *ret = callMethod(GNOME_SESSION_PATH, GNOME_SESSION_ITF, "RegisterClient", args)) {
        gchar *client_id = nullptr;
        g_variant_get(ret, "(o)", &client_id);
        if (client_id) {
            result = client_id;
            g_free(client_id);
        }
        g_variant_unref(ret);
    }
    g_variant_unref(args);
    return result;
}

static void unregisterClient(const std::string &client_id)
{
    GVariant *args = g_variant_new("(o)", client_id.c_str());
    g_variant_ref_sink(args);
    if (GVariant *ret = callMethod(GNOME_SESSION_PATH, GNOME_SESSION_ITF, "UnregisterClient", args)) {
        g_variant_unref(ret);
    }
    g_variant_unref(args);
}

static void endSessionResponse(const std::string &client_id, gboolean is_ok, const std::string &reason)
{
    GVariant *args = g_variant_new("(bs)", is_ok, reason.c_str());
    g_variant_ref_sink(args);
    if (GVariant *ret = callMethod(client_id.c_str(), GNOME_SESSION_CLNT_ITF, "EndSessionResponse", args)) {
        g_variant_unref(ret);
    }
    g_variant_unref(args);
}

static void onSessionSignal(GDBusConnection*, const gchar*, const gchar*, const gchar*, const gchar *name, GVariant*, gpointer data)
{
    const char *client_id = (const char*)data;
    static bool holdApp = false;
    if (strcmp(name, "QueryEndSession") == 0) {
        holdApp = true;
        std::string msg = QObject::tr("There are unsaved documents", "SingleApplication").toStdString();
        endSessionResponse(client_id, !AscAppManager::hasUnsavedChanges(), msg);
        while (holdApp)
            Utils::processMoreEvents(1000);
    } else
    if (strcmp(name, "CancelEndSession") == 0) {
        holdApp = false;
    } else
    if (strcmp(name, "EndSession") == 0) {
        holdApp = false;
        endSessionResponse(client_id, TRUE, "");
        AscAppManager::closeAppWindows();
    } else
    if (strcmp(name, "Stop") == 0) {
        holdApp = false;
    }
}

SingleApplication::SingleApplication(int &argc, char *argv[]) :
    QApplication(argc, argv)
{
    m_socket = new CSocket(0, Utils::getInstAppPort(), false, true);
    if (m_socket->isPrimaryInstance()) {
        m_isPrimary = true;
        m_socket->onMessageReceived([=](void *buff, size_t size) {
            QString data = QString::fromLocal8Bit((const char*)buff, size);
            QMetaObject::invokeMethod(this, "invokeSignal", Qt::QueuedConnection, Q_ARG(QString, data));
        });

        if (WindowHelper::getEnvInfo() == WindowHelper::GNOME && !(m_client_id = registerClient()).empty()) {
            if ((m_conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, nullptr)) != nullptr) {
                m_subscrId  = g_dbus_connection_signal_subscribe(m_conn, GNOME_SESSION_NAME, GNOME_SESSION_CLNT_ITF, nullptr, m_client_id.c_str(), nullptr,
                                                                     G_DBUS_SIGNAL_FLAGS_NONE, onSessionSignal, (gpointer)m_client_id.c_str(), nullptr);
            }
            connect(this, &QApplication::commitDataRequest, this, [=](QSessionManager &sm) {
                if (sm.allowsInteraction()) {
                    // sm.release();
                    Utils::processMoreEvents(250);
                }
            });
        }
    }
}

SingleApplication::~SingleApplication()
{
    if (m_conn) {
        g_dbus_connection_signal_unsubscribe(m_conn, m_subscrId);
        g_object_unref(m_conn);
        unregisterClient(m_client_id);
    }

    if (m_socket)
        delete m_socket, m_socket = nullptr;
}

bool SingleApplication::isPrimary()
{
    return m_isPrimary;
}

bool SingleApplication::sendMessage(const QByteArray &msg)
{
    if (m_isPrimary)
        return false;

    CSocket socket(Utils::getInstAppPort(), 0, false, true);
    return socket.sendMessage((void*)msg.data(), msg.size());
}

void SingleApplication::invokeSignal(const QString &data)
{
    emit receivedMessage(data.toUtf8());
}
