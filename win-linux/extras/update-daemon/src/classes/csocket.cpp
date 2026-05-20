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

#include "csocket.h"
#include <future>
#include <sstream>

#ifdef _WIN32
# define _WINSOCK_DEPRECATED_NO_WARNINGS
# ifndef UNICODE
#  define UNICODE 1
# endif
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
# include <winsock2.h>
# include <sys/types.h>
# include <io.h>
# define sleep(ms) Sleep(ms)
# define close_socket(a) closesocket(a)
#else
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# define SD_BOTH SHUT_RDWR
# define sleep(ms) usleep(ms*1000)
# define close_socket(a) close(a)
  typedef int SOCKET;
#endif

#define INADDR "127.0.0.1\0"
#define RETRIES_DELAY_MS 4000
#define BUFFSIZE 1024

typedef struct sockaddr_in SockAddr;

static unsigned long inetAddrFromUserId()
{
#ifdef _WIN32
    return inet_addr(INADDR);
#else
    uint16_t uid = uint16_t(getuid() - 1000);
    char addr[14];
    snprintf(addr, sizeof(addr), "127.%d.%d.1", (uid >> 8) & 0xff, uid & 0xff);
    return inet_addr(addr);
#endif
}

static bool initSocket(u_short port, SOCKET &tmpd, SockAddr &addr, int &ret, std::string &error, bool use_unique_addr)
{
#ifdef _WIN32
    WSADATA wsaData = {0};
    int iResult = 0;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        error = "Init socket: WSAStartup failed!";
        return false;
    }
    tmpd = INVALID_SOCKET;
    if ((tmpd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
#else
    tmpd = -1;
    if ((tmpd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0)) < 0) {
#endif
        error = "Init socket: socket not valid!";
        return false;
    }
    memset(&addr, 0, sizeof(SockAddr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = use_unique_addr ? inetAddrFromUserId() : inet_addr(INADDR);
    addr.sin_port = htons(port);
    ret = ::bind(tmpd, (struct sockaddr*)&addr, sizeof(addr));
    return true;
}

class CSocket::CSocketPrv
{
public:
    CSocketPrv();
    ~CSocketPrv();

    bool createSocket(u_short port);
    bool connectToSocket(u_short port);
    void startReadMessages();
    void closeSocket(SOCKET &socket);
    void postError(const char*);

    SOCKET sender_fd = -1,
           receiver_fd = -1;

    FnVoidData m_received_callback = nullptr;
    FnVoidCharPtr m_error_callback = nullptr;
    std::future<void> m_future;
    int m_sender_port;
    std::atomic_bool m_run,
                     m_use_unique_addr,
                     m_socket_created;
};

CSocket::CSocketPrv::CSocketPrv()
{
    m_run = true;
    m_socket_created = false;
}

CSocket::CSocketPrv::~CSocketPrv()
{}

bool CSocket::CSocketPrv::createSocket(u_short port)
{
    int ret;
    SOCKET tmpd;
    SockAddr addr;
    std::string error;
    if (!initSocket(port, tmpd, addr, ret, error, m_use_unique_addr)) {
        postError(error.c_str());
        return false;
    }

    if (ret == 0) {
        receiver_fd = tmpd;
        return true;
    }
    close_socket(tmpd);
    postError("Could not create socket!");
    return false;
}

bool CSocket::CSocketPrv::connectToSocket(u_short port)
{
    int ret;
    SOCKET tmpd;
    SockAddr addr;
    std::string error;
    if (!initSocket(port, tmpd, addr, ret, error, m_use_unique_addr)) {
        postError(error.c_str());
        return false;
    }

    if (ret != 0) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEADDRINUSE) {
#else
        if (errno == EADDRINUSE) {
#endif
            ret = ::connect(tmpd, (struct sockaddr*)&addr, sizeof(SockAddr));
            if (ret == 0) {
                sender_fd = tmpd;
                return true;
            }
        }
    }
    close_socket(tmpd);
    postError("Could not connect to socket!");
    return false;
}

void CSocket::CSocketPrv::startReadMessages()
{
    while (m_run) {
        char rcvBuf[BUFFSIZE] = {0};
        int ret_data = recv(receiver_fd, rcvBuf, BUFFSIZE, 0); // Receive the string data
        if (ret_data != BUFFSIZE) {
            if (ret_data < 0) {
                // FAILURE
                postError("Start read messages: error while accessing socket!");
            } else {
                // Connection closed.
            }
        } else {
            // SUCCESS
            if (m_received_callback)
                m_received_callback(rcvBuf, strlen(rcvBuf));
        }
    }
    // Dropped out of daemon loop.
}

void CSocket::CSocketPrv::closeSocket(SOCKET &socket)
{
    if (socket >= 0) {
        shutdown(socket, SD_BOTH);
        close_socket(socket);
        socket = -1;
    }
}

void CSocket::CSocketPrv::postError(const char *error)
{
    if (m_error_callback)
        m_error_callback(error);
}

CSocket::CSocket(int sender_port, int receiver_port, bool retry_connect, bool use_unique_addr) :
    pimpl(new CSocketPrv)
{
    pimpl->m_use_unique_addr = use_unique_addr;
    pimpl->m_sender_port = sender_port;
    if (receiver_port <= 0)
        return;
    pimpl->m_socket_created = pimpl->createSocket(receiver_port);
    pimpl->m_future = std::async(std::launch::async, [=]() {
        while (retry_connect && pimpl->m_run && !pimpl->m_socket_created) {
            pimpl->postError("Unable to create socket, retrying after 4 seconds.");
            sleep(RETRIES_DELAY_MS);
            pimpl->m_socket_created = pimpl->createSocket(receiver_port);
        }
        if (pimpl->m_socket_created)
            pimpl->startReadMessages();
    });
}

CSocket::~CSocket()
{
    pimpl->m_run = false;
    pimpl->closeSocket(pimpl->sender_fd);
    pimpl->closeSocket(pimpl->receiver_fd);
    if (pimpl->m_future.valid())
        pimpl->m_future.wait();

    delete pimpl;
#ifdef _WIN32
    WSACleanup();
#endif
}

bool CSocket::isPrimaryInstance()
{
    return pimpl->m_socket_created;
}

bool CSocket::sendMessage(void *data, size_t size)
{
    if (!data || size > BUFFSIZE - 1)
        return false;

    if (!pimpl->connectToSocket(pimpl->m_sender_port))
        return false;

    char client_arg[BUFFSIZE] = {0};
    memcpy(client_arg, data, size);
#ifdef _WIN32
    int ret_data = send(pimpl->sender_fd, client_arg, BUFFSIZE, 0); // Send the string
#else
    int ret_data =(int)send(pimpl->sender_fd, client_arg, BUFFSIZE, MSG_CONFIRM);
#endif
    if (ret_data != BUFFSIZE) {
        if (ret_data < 0) {
            pimpl->postError("Send message error: could not send device address to daemon!");
        } else {
            pimpl->postError("Send message error: could not send device address to daemon completely!");
        }
        pimpl->closeSocket(pimpl->sender_fd);
        return false;
    }
    pimpl->closeSocket(pimpl->sender_fd);
    return true;
}

bool CSocket::sendMessage(int cmd, const tstring &param1, const tstring &param2)
{
    tstring str = std::to_tstring(cmd) + _T("|") + (param1.empty() ? _T("null") : param1) +
                  _T("|") + (param2.empty() ? _T("null") : param2);
    size_t sz = str.size() * sizeof(str.front());
    return sendMessage((void*)str.c_str(), sz);
}

void CSocket::onMessageReceived(FnVoidData callback)
{
    pimpl->m_received_callback = callback;
}

void CSocket::onError(FnVoidCharPtr callback)
{
    pimpl->m_error_callback = callback;
}

int CSocket::parseMessage(void *data, std::vector<tstring> &params)
{
    tstring str((const tchar*)data), tmp;
    tstringstream wss(str);
    while (std::getline(wss, tmp, _T('|')))
        params.push_back(tmp == _T("null") ? _T("") : std::move(tmp));
    return (int)params.size();
}
