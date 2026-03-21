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
