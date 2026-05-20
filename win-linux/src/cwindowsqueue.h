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

#ifndef CWINDOWSQUEUE_H
#define CWINDOWSQUEUE_H

#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <utility>
#include <vector>
#include <atomic>


#define THREAD_WAIT_INTERVAL 10

template<typename T>
class CWindowsQueue
{
    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
    std::vector<T> m_wintoclose;
    std::atomic_bool m_queueCanceled{false};

    std::function<void(T)> m_callback;
public:
    CWindowsQueue()
    {}

    ~CWindowsQueue()
    {
        for (std::thread& t: m_threads)
        {
            if ( t.joinable() )
                t.join();
        }
    }

    void start_queue() {
        if ( !m_wintoclose.empty() ) {
            trigger_callback(*m_wintoclose.begin());
        }
    }

    void enter(const T& iter)
    {
        m_wintoclose.push_back(iter);
        if ( !(m_wintoclose.size() > 1) ) {
            m_queueCanceled.store(false);

            std::function<void()> start_func_(std::bind(&CWindowsQueue::start_queue, this));

            std::thread([start_func_]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_WAIT_INTERVAL));
                start_func_();
            }).detach();
        }
    }

    void leave(T iter)
    {
        m_threads.push_back(std::thread(&CWindowsQueue::leave_thread_func, this, iter));
    }

    void cancel()
    {
        m_threads.push_back(std::thread(&CWindowsQueue::cancel_thread_func, this));
    }

    void setcallback(std::function<void(T)>& fn)
    {
        m_callback = fn;
    }


private:
    void leave_thread_func(T iter)
    {
        std::lock_guard<std::mutex> lock{m_mutex};

        const auto& it = std::find_if(m_wintoclose.begin(), m_wintoclose.end(), [&](T i){ return i == iter; });

        if ( it != m_wintoclose.end() )
            m_wintoclose.erase(it);

        if ( !m_wintoclose.empty() && !m_queueCanceled.load() ) {
            trigger_callback(*m_wintoclose.begin());
        }
    }

    void cancel_thread_func()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_wintoclose.clear();
        m_queueCanceled.store(true);
    }

    void trigger_callback(const T& iter)
    {
        if ( m_callback )
            m_callback(iter);
    }
};

#endif // CWINDOWSQUEUE_H
