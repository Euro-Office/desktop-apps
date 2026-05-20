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

#ifndef CTHEMES_H
#define CTHEMES_H

#include <QColor>
#include <QJsonArray>

#ifdef Q_OS_WIN
# include <windows.h>
#endif

#define GetCurrentTheme() \
    AscAppManager::themes().current()
#define GetActualTheme(theme) \
    AscAppManager::themes().themeActualId(theme)
#define GetColorByRole(role) \
    GetCurrentTheme().color(CTheme::ColorRole::role)
#define GetColorValueByRole(role) \
    GetCurrentTheme().value(CTheme::ColorRole::role)
#define GetColorQValueByRole(role) \
    QString::fromStdWString(GetColorValueByRole(role))

class CThemes;
class CTheme {
public:
    enum class ColorRole {
        ecrWindowBackground
        , ecrWindowBorder
        , ecrBorderControlFocus
        , ecrTextNormal
        , ecrTextPretty
        , ecrTextInverse
        , ecrLogoColor
        , ecrTabWordActive
        , ecrTabCellActive
        , ecrTabSlideActive
        , ecrTabViewerActive
        , ecrTabDrawActive
        , ecrTabSimpleActiveBackground
        , ecrTabSimpleActiveText
        , ecrTabDefaultActiveBackground
        , ecrTabDefaultActiveText
        , ecrButtonNormalOpacity
        , ecrButtonBackground
        , ecrButtonHoverBackground
        , ecrButtonPressedBackground
        , ecrButtonBackgroundActive
        , ecrDownloadWidgetBackground
        , ecrDownloadWidgetBorder
        , ecrDownloadItemHoverBackground
        , ecrDownloadGhostButtonText
        , ecrDownloadGhostButtonTextHover
        , ecrDownloadGhostButtonTextPressed
        , ecrDownloadGhostButtonTextPressedItemHover
        , ecrDownloadLabelText
        , ecrDownloadLabelTextInfo
        , ecrDownloadLabelTextInfoItemHover
        , ecrDownloadProgressBarBackground
        , ecrDownloadProgressBarBackgroundItemHover
        , ecrDownloadProgressBarChunk
        , ecrDownloadScrollBarHandle
        , ecrMenuBackground
        , ecrMenuBorder
        , ecrMenuItemHoverBackground
        , ecrMenuText
        , ecrMenuTextItemHover
        , ecrMenuTextItemDisabled
        , ecrMenuSeparator
        , ecrToolTipText
        , ecrToolTipBorder
        , ecrToolTipBackground
        , ecrTabDivider
        , ecrTabThemeType
    };

    CTheme(const CTheme &other);
    CTheme(CTheme &&other) noexcept;
    ~CTheme();

    CTheme& operator=(const CTheme&);
    CTheme& operator=(CTheme&&) noexcept;

    auto fromFile(const QString&) -> bool;
    auto fromJson(const QString&) -> bool;
    auto json() const -> QString;

    auto id() const -> std::wstring;
    auto originalId() const -> std::wstring;
    auto stype() const -> QString;
    auto color(ColorRole r) const -> QColor;
#ifdef Q_OS_WIN
    auto colorRef(ColorRole r) const -> COLORREF;
#endif
    auto value(ColorRole, const std::wstring& def = L"") const -> std::wstring;
    auto isDark() const -> bool;
    auto isSystem() const -> bool;
    auto isValid() const -> bool;

private:
    CTheme(const QString& path = QString());

    class CThemePrivate;
    CThemePrivate * m_priv = nullptr;

    friend class CThemes;
};

class CThemes
{
public:
    CThemes();
    ~CThemes();

    auto current() -> const CTheme&;
    auto defaultDark() -> const CTheme&;
    auto defaultLight() -> const CTheme&;
    auto localFromId(const QString &id) const -> CTheme;

//    auto addLocalTheme(const std::wstring&) -> bool;
    auto addLocalTheme(QJsonObject&, const QString& filepath) -> bool;
    auto setCurrentTheme(const std::wstring&) -> void;
    auto isThemeCurrent(const std::wstring& id) -> bool;
//    auto isThemeDark(const std::wstring& id) -> bool;
    auto themeActualId(const std::wstring& id) const -> std::wstring;

    static auto isColorDark(const std::wstring&) -> bool;
    static auto isColorDark(const QString&) -> bool;

    auto onSystemDarkColorScheme(bool isdark) -> void;
    auto isSystemSchemeDark() -> const bool;
    auto parseThemeName(const std::wstring&) -> std::wstring;
    auto localThemesToJson() -> QJsonArray;
    auto contains(const QString& id) -> bool;
    auto validate(const QJsonObject&) -> bool;
    auto checkDestinationThemeFileExist(const QString& srcpath) -> bool;
private:
    class CThemesPrivate;
    CThemesPrivate * m_priv = nullptr;
};

#endif // CTHEMES_H
