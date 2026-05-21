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

#ifndef VERSION_H
#define VERSION_H

#define VER_STRINGIFY(d)            #d
#define TO_STR(v)                   VER_STRINGIFY(v)

#ifdef VER_PRODUCT_VERSION
# define VER_FILEVERSION            VER_PRODUCT_VERSION_COMMAS
# define VER_FILEVERSION_STR        TO_STR(VER_PRODUCT_VERSION)

# define VER_PRODUCTVERSION         VER_FILEVERSION
# define VER_PRODUCTVERSION_STR     TO_STR(VER_PRODUCT_VERSION)
#else
# define VER_STR_LONG(mj,mn,b,r)    VER_STRINGIFY(mj) "." VER_STRINGIFY(mn) "." VER_STRINGIFY(b) "." VER_STRINGIFY(r) "\0"
# define VER_STR_SHORT(mj,mn)       VER_STRINGIFY(mj) "." VER_STRINGIFY(mn) "\0"

# define VER_NUM_MAJOR              5
# define VER_NUM_MINOR              3
# define VER_NUM_BUILD              95
# define VER_NUM_REVISION           508
# define VER_NUMBER                 VER_NUM_MAJOR,VER_NUM_MINOR,VER_NUM_BUILD,VER_NUM_REVISION
# define VER_STRING                 VER_STR_LONG(VER_NUM_MAJOR,VER_NUM_MINOR,VER_NUM_BUILD,VER_NUM_REVISION)
# define VER_STRING_SHORT           VER_STR_SHORT(VER_NUM_MAJOR,VER_NUM_MINOR)

# define VER_FILEVERSION            VER_NUMBER
# define VER_FILEVERSION_STR        VER_STRING

# define VER_PRODUCTVERSION         VER_FILEVERSION
# define VER_PRODUCTVERSION_STR     VER_STRING_SHORT
#endif

#define VER_COMPANYNAME_STR         "Ascensio System SIA\0"
#define VER_LEGALCOPYRIGHT_STR      "© Ascensio System SIA " TO_STR(COPYRIGHT_YEAR) ". All rights reserved.\0"
#define VER_COMPANYDOMAIN_STR       "https://github.com/Euro-Office\0"
#define ABOUT_COPYRIGHT_STR         VER_LEGALCOPYRIGHT_STR
#define VER_FILEDESCRIPTION_STR     "Euro-Office Desktop Editors\0"
#define VER_INTERNALNAME_STR        "Desktop Editors\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved\0"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "documenteditor.exe\0"
#define VER_PRODUCTNAME_STR         "Euro-Office\0"

#define VER_LANG_AND_CHARSET        "040904E4"
#define VER_LANG_ID                 0x0409
#define VER_CHARSET_ID              1252

#ifndef RC_COMPILE_FLAG
# include "version_p.h"
#endif

#endif

