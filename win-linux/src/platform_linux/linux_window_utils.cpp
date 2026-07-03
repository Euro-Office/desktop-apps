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

#include "linux_window_utils.h"
#include "iplatformbackend.h"

void LinuxWindowUtils::moveWindow(WId window, int x, int y)
{
    IPlatformBackend::instance()->moveWindow(window, x, y);
}

bool LinuxWindowUtils::isNativeFocus(WId window)
{
    return IPlatformBackend::instance()->isNativeFocus(window);
}

void LinuxWindowUtils::setNativeFocusTo(WId window)
{
    IPlatformBackend::instance()->setNativeFocusTo(window);
}

void LinuxWindowUtils::findWindowAsync(const char *window_name, void *user_data,
                               uint timeout_ms,
                               void(*callback)(WId, void*))
{
    IPlatformBackend::instance()->findWindowAsync(window_name, user_data, timeout_ms, callback);
}

void LinuxWindowUtils::getWindowStack(std::vector<WId> &winStack)
{
    IPlatformBackend::instance()->getWindowStack(winStack);
}

void LinuxWindowUtils::setInputEnabled(WId window, bool enabled)
{
    IPlatformBackend::instance()->setInputEnabled(window, enabled);
}
