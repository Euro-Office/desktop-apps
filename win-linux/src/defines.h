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

#ifndef DEFINES_H
#define DEFINES_H

#define rePortalName        "^https?:\\/\\/(.+)"
#define reFileExtension     "\\.(\\w{1,10})$"
#define reUserName          "([^\\.]+)\\.?([^\\.]+)?"
#define reCmdLang           "--(keep)?lang[:|=](\\w{2,5})"

#define APP_NAME "DesktopEditors"
#define APP_TITLE "ONLYOFFICE"
#ifdef __linux
# define APP_DATA_PATH "/onlyoffice/desktopeditors"
# define REG_GROUP_KEY "onlyoffice"
# define APP_MUTEX_NAME "asc:editors"
# define DESKTOP_FILE_NAME "onlyoffice-desktopeditors"
#else
# define APP_DATA_PATH "/ONLYOFFICE/DesktopEditors"
# define APP_REG_NAME  "ONLYOFFICE"
# define REG_GROUP_KEY "ONLYOFFICE"
# define REG_UNINST_KEY "ONLYOFFICE Desktop Editors"
# define APP_MUTEX_NAME "TEAMLAB"
#endif

#define WINDOW_NAME "ONLYOFFICE"
#define WINDOW_TITLE WINDOW_NAME
#define WINDOW_CLASS_NAME L"DocEditorsWindowClass"
#define WINDOW_EDITOR_CLASS_NAME L"SingleWindowClass"
#define REG_APP_NAME "DesktopEditors"
#define APP_DEFAULT_LOCALE "en-US"
#define APP_DEFAULT_SYSTEM_LOCALE 1
#define APP_USER_MODEL_ID "ASC.Documents.5"
#define APP_SIMPLE_WINDOW_TITLE "ONLYOFFICE Editor"
#define APP_PROTOCOL "oo-office"
#define FILE_PREFIX "onlyoffice_"

#define URL_SITE                "http://www.onlyoffice.com"
#define URL_SIGNUP              "https://onlyoffice.com/registration.aspx?desktop=true"

#define GET_REGISTRY_USER(variable) \
    QSettings variable(QSettings::NativeFormat, QSettings::UserScope, REG_GROUP_KEY, REG_APP_NAME);
#define GET_REGISTRY_SYSTEM(variable) \
    QSettings variable(QSettings::SystemScope, REG_GROUP_KEY, REG_APP_NAME);

#define LOCAL_PATH_OPEN         1
#define LOCAL_PATH_SAVE         2

#define ACTIONPANEL_CONNECT     255
#define ACTIONPANEL_ACTIVATE    ACTIONPANEL_CONNECT + 1

#define URL_AGPL "https://www.gnu.org/licenses/agpl-3.0.en.html"
#define DOWNLOAD_PAGE "https://www.onlyoffice.com/en/download-desktop.aspx"
#define RELEASE_NOTES "https://github.com/ONLYOFFICE/DesktopEditors/blob/master/CHANGELOG.md"

#ifdef __linux
typedef unsigned char BYTE;
#else
# define UM_INSTALL_UPDATE      WM_USER+254
#endif

#define UM_ENDMOVE (QEvent::User + 2)

#define TO_WSTR(str)            L ## str
#define WSTR(str)               TO_WSTR(str)

#ifdef __linux
# define VK_F1 0x70
# define VK_F4 0x73
# define VK_TAB 0x09
#endif

#define APP_PORT   12010
#define SVC_PORT   12011
#define INSTANCE_SVC_PORT 12012
#define INSTANCE_APP_PORT 13012

#define WARNING_LAUNCH_WITH_ADMIN_RIGHTS "App can't working correctly under admin rights."

#include "defines_p.h"

#endif // DEFINES_H

