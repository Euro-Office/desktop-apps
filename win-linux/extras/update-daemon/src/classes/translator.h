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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <unordered_map>
#include <string>
#ifdef _WIN32
# include <tchar.h>
# define tchar wchar_t
# define tstring std::wstring
#else
# define _T(str) str
# define tchar char
# define tstring std::string
#endif

#define _TR(str) Translator::instance().tr(_T(#str))

using std::unordered_map;

typedef unordered_map<tstring, tstring> LocaleMap;
typedef unordered_map<tstring, LocaleMap> TranslationsMap;


class Translator
{
public:
    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;
    static Translator& instance();

#ifdef _WIN32
    void init(const tstring &lang, int resourceId);
#else
    void init(const tstring &lang, const char *resourcePath);
#endif
    tstring tr(const tchar*) const;
    void setLanguage(const tstring &lang);

private:
    Translator();
    ~Translator();

    TranslationsMap translMap;
    tstring langName;
    bool    is_translations_valid;

    enum TokenType {
        TOKEN_BEGIN_DOCUMENT = 0,
        TOKEN_END_DOCUMENT,
        TOKEN_BEGIN_STRING_ID,
        TOKEN_END_STRING_ID,
        TOKEN_BEGIN_LOCALE,
        TOKEN_END_LOCALE,
        TOKEN_BEGIN_VALUE,
        TOKEN_END_VALUE
    };
};

#endif // TRANSLATOR_H
