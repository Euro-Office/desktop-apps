
#include "cthemes.h"
#ifdef Q_OS_LINUX
# include <gtk/gtk.h>
# include <gio/gio.h>
# include <glib.h>
# include "cascapplicationmanagerwrapper.h"
#endif
#include "defines.h"
#include "utils.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColor>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QProcess>
#include <QPalette>
#include <QApplication>
#include <QJsonArray>

#define QSTRING_FROM_WSTR(s) QString::fromStdWString(s)
#define REGISTRY_THEME_KEY "UITheme"
// #define REGISTRY_THEME_KEY_7_2 "UITheme2"
#define THEME_DEFAULT_DARK_ID "theme-night"
#if !defined(__OS_WIN_XP)
# define THEME_DEFAULT_LIGHT_ID "theme-white"
#else
# define THEME_DEFAULT_LIGHT_ID "theme-classic-light"
#endif
#define THEME_ID_SYSTEM "theme-system"

namespace NSTheme {
    static const QString theme_type_dark = "dark";
    static const QString theme_type_light = "light";

    enum class ThemeType {
        ttUndef,
        ttDark,
        ttLight
    };

    static const std::map<CTheme::ColorRole, QString> map_names = {
            {CTheme::ColorRole::ecrTabWordActive, "brand-word"},
            {CTheme::ColorRole::ecrTabSlideActive, "brand-slide"},
            {CTheme::ColorRole::ecrTabCellActive, "brand-cell"},
            {CTheme::ColorRole::ecrTabViewerActive, "brand-pdf"},
            {CTheme::ColorRole::ecrTabDrawActive, "brand-draw"},

            {CTheme::ColorRole::ecrWindowBackground, "window-background"},
            {CTheme::ColorRole::ecrWindowBorder, "window-border"},

            {CTheme::ColorRole::ecrBorderControlFocus, "border-control-focus"},

            {CTheme::ColorRole::ecrTextNormal, "text-normal"},
            {CTheme::ColorRole::ecrTextPretty, "text-pretty"},
            {CTheme::ColorRole::ecrTextInverse, "text-inverse"},

            {CTheme::ColorRole::ecrButtonBackground, "tool-button-background"},
            {CTheme::ColorRole::ecrButtonHoverBackground, "tool-button-hover-background"},
            {CTheme::ColorRole::ecrButtonPressedBackground, "tool-button-pressed-background"},
            {CTheme::ColorRole::ecrButtonBackgroundActive, "tool-button-active-background"},

            {CTheme::ColorRole::ecrDownloadWidgetBackground, "download-widget-background"},
            {CTheme::ColorRole::ecrDownloadWidgetBorder, "download-widget-border"},
            {CTheme::ColorRole::ecrDownloadItemHoverBackground, "download-item-hover-background"},

            {CTheme::ColorRole::ecrDownloadGhostButtonText, "download-ghost-button-text"},
            {CTheme::ColorRole::ecrDownloadGhostButtonTextHover, "download-ghost-button-text-hover"},
            {CTheme::ColorRole::ecrDownloadGhostButtonTextPressed, "download-ghost-button-text-pressed"},
            {CTheme::ColorRole::ecrDownloadGhostButtonTextPressedItemHover, "download-ghost-button-text-pressed-item-hover"},

            {CTheme::ColorRole::ecrDownloadLabelText, "download-label-text"},
            {CTheme::ColorRole::ecrDownloadLabelTextInfo, "download-label-text-info"},
            {CTheme::ColorRole::ecrDownloadLabelTextInfoItemHover, "download-label-text-info-item-hover"},

            {CTheme::ColorRole::ecrDownloadProgressBarBackground, "download-progressbar-background"},
            {CTheme::ColorRole::ecrDownloadProgressBarBackgroundItemHover, "download-progressbar-background-item-hover"},
            {CTheme::ColorRole::ecrDownloadProgressBarChunk, "download-progressbar-chunk"},

            {CTheme::ColorRole::ecrDownloadScrollBarHandle, "download-scrollbar-handle"},

            {CTheme::ColorRole::ecrMenuBackground, "menu-background"},
            {CTheme::ColorRole::ecrMenuBorder, "menu-border"},
            {CTheme::ColorRole::ecrMenuItemHoverBackground, "menu-item-hover-background"},
            {CTheme::ColorRole::ecrMenuText, "menu-text"},
            {CTheme::ColorRole::ecrMenuTextItemHover, "menu-text-item-hover"},
            {CTheme::ColorRole::ecrMenuTextItemDisabled, "menu-text-item-disabled"},
            {CTheme::ColorRole::ecrMenuSeparator, "menu-separator"},

            {CTheme::ColorRole::ecrToolTipText, "tooltip-text"},
            {CTheme::ColorRole::ecrToolTipBorder, "tooltip-border"},
            {CTheme::ColorRole::ecrToolTipBackground, "tooltip-background"},

    //      {  "tab-active-background": "#fff",
            {CTheme::ColorRole::ecrTabSimpleActiveBackground, "tab-simple-active-background"},
            {CTheme::ColorRole::ecrTabSimpleActiveText, "tab-simple-active-text"},
            {CTheme::ColorRole::ecrTabDefaultActiveBackground, "tab-default-active-background"},
            {CTheme::ColorRole::ecrTabDefaultActiveText, "tab-default-active-text"},
            {CTheme::ColorRole::ecrTabDivider, "tab-divider"},

            {CTheme::ColorRole::ecrButtonNormalOpacity, "button-normal-opacity"},
            {CTheme::ColorRole::ecrLogoColor, "logo"},
            {CTheme::ColorRole::ecrTabThemeType, "tab-editor-theme-type"}
        };

    static const std::map<QString, QString> map_alias_names = {
            {"brand-word", "toolbar-header-document"},
            {"brand-slide", "toolbar-header-presentation"},
            {"brand-cell", "toolbar-header-spreadsheet"},
            {"brand-pdf", "toolbar-header-pdf"},
            {"brand-draw", "toolbar-header-draw"},

            {"window-background", "background-toolbar"},
            // {"window-border", ""},

            {"text-pretty", "text-toolbar-header"},

            {"tool-button-background",         "background-toolbar"},
            {"tool-button-hover-background",   "highlight-button-hover"},
            {"tool-button-pressed-background", "highlight-button-pressed-hover"},
            {"tool-button-active-background",  "background-normal"},

            {"download-widget-background",     "background-normal"},
            {"download-widget-border",         "border-regular-control"},
            {"download-item-hover-background", "highlight-button-hover"},

            // {"download-ghost-button-text",          ""},
            {"download-ghost-button-text-hover",    "text-normal"},
            // {"download-ghost-button-text-pressed",  ""},
            // {"download-ghost-button-text-pressed-item-hover", ""},

            {"download-label-text",                 "text-normal"},
            // {"download-label-text-info",            ""},
            // {"download-label-text-info-item-hover", ""},

            // {"download-progressbar-chunk",          ""},
            // {"download-progressbar-background",     ""},
            // {"download-progressbar-background-item-hover", ""},

            {"download-scrollbar-handle",  "canvas-scroll-thumb-hover"},

            {"menu-background",            "background-normal"},
            {"menu-border",                "border-regular-control"},
            {"menu-item-hover-background", "highlight-button-hover"},

            {"menu-text",            "text-normal"},
            {"menu-text-item-hover", "text-normal"},
            // {"menu-text-item-disabled", ""},

            {"menu-separator",       "border-divider"},

            {"tooltip-text",         "text-normal"},
            {"tooltip-border",       "border-regular-control"},
            {"tooltip-background",   "background-normal"},

            // {"tab-active-background",         ""}, // not used
            // {"tab-simple-active-background",  ""},
            {"tab-simple-active-text",        "text-normal"},
            // {"tab-default-active-background", ""},
            // {"tab-default-active-text",       ""}, // not used
            {"tab-divider",                   "border-divider"},

//            {CTheme::ColorRole::ecrWindowBorder, "window-border"},

//            {CTheme::ColorRole::ecrTextNormal, "text-normal"},
//            {CTheme::ColorRole::ecrTextPressed, "text-normal-pressed"},

//            {CTheme::ColorRole::ecrTabSimpleActiveBackground, "tab-simple-active-background"},
//            {CTheme::ColorRole::ecrTabSimpleActiveText, "tab-simple-active-text"},
//            {CTheme::ColorRole::ecrTabDefaultActiveBackground, "tab-default-active-background"},
//            {CTheme::ColorRole::ecrTabDefaultActiveText, "tab-default-active-text"},

//            {CTheme::ColorRole::ecrButtonNormalOpacity, "button-normal-opacity"},
//            {CTheme::ColorRole::ecrLogoColor, "logo"}
        };
}

auto getUserThemesPath() -> QString
{
    return Utils::getAppCommonPath() + "/uithemes";
}

#ifdef __linux__
static const char *XDG_DSK_DBUS_SENDER = "org.freedesktop.portal.Desktop",
                  *XDG_STN_DBUS_ITF    = "org.freedesktop.portal.Settings",
                  *XDG_DSK_DBUS_PATH   = "/org/freedesktop/portal/desktop";

static void onSettingsChanged(GDBusConnection *conn, const gchar *sender,
                              const gchar *obj_path, const gchar *interface,
                              const gchar *signal_name, GVariant *params, gpointer data)
{
    const gchar *ns;
    const gchar *key;
    GVariant *value;
    g_variant_get(params, "(&s&s@v)", &ns, &key, &value);
    if (!value) return;

    GVariant *inner = g_variant_get_variant(value);
    if (!inner) {
        g_variant_unref(value);
        return;
    }
    if (g_strcmp0(ns, "org.freedesktop.appearance") == 0) {
        if (g_strcmp0(key, "color-scheme") == 0) {
            guint32 color_scheme = g_variant_get_uint32(inner);
            // 0: No preference
            // 1: Prefer dark appearance
            // 2: Prefer light appearance

            static int last_is_dark = -1;
            int is_dark = (color_scheme == 1);
            if (last_is_dark != is_dark) {
                last_is_dark = is_dark;

                NSEditorApi::CAscCefMenuEvent * ns_event = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND);
                NSEditorApi::CAscExecCommand * pData = new NSEditorApi::CAscExecCommand;
                pData->put_Command(L"system:changed");

                QJsonObject _json_obj{{"colorscheme", is_dark == 1 ? "dark" : "light"}};
                pData->put_Param(Utils::stringifyJson(_json_obj).toStdWString());

                ns_event->m_pData = pData;
                ns_event->m_nSenderId = 0;
                AscAppManager::getInstance().OnEvent(ns_event);
            }
        }
    }
    g_variant_unref(inner);
    g_variant_unref(value);
}

static bool themeExists(const char *theme)
{
    char path[256];
    snprintf(path, sizeof(path), "/usr/share/themes/%s/gtk-3.0", theme);
    return access(path, F_OK) == 0;
}

static void applyGtkTheme(bool isDark)
{
    const char *theme_light = "Adwaita";
    const char *theme_dark = "Adwaita-dark";
    if (!themeExists(theme_light) || !themeExists(theme_dark))
        return;
    qputenv("GTK_THEME", isDark ? theme_dark : theme_light);
    if (GtkSettings *stn = gtk_settings_get_default())
        g_object_set(stn, "gtk-theme-name", isDark ? theme_dark : theme_light, NULL);
}
#endif

/*
 * CThemePrivate
*/
class CTheme::CThemePrivate {
public:
    CThemePrivate() {}
    CThemePrivate(const CThemePrivate &other) :
        id(other.id),
        wstype(other.wstype),
        type(other.type),
        is_system(other.is_system),
        jsonValues(other.jsonValues),
        defdark(other.defdark),
        deflight(other.deflight),
        source_file(other.source_file)
    {}

    auto fromJsonObject(const QJsonObject& obj) -> void {
        id = obj.value("id").toString().toStdWString();
        type = obj.value("type").toString() == NSTheme::theme_type_dark ?
                            NSTheme::ThemeType::ttDark : NSTheme::ThemeType::ttLight;
        if ( obj.contains("colors") ) {
            QJsonObject colorValues = obj.value("colors").toObject();

            const CTheme * default_theme = type == NSTheme::ThemeType::ttDark ? defdark : deflight;
            jsonValues = QJsonObject(default_theme->m_priv->jsonValues);

            for ( const auto &color_name : NSTheme::map_names ) {
                if ( colorValues.contains(color_name.second) ) {
                    jsonValues[color_name.second] = colorValues.value(color_name.second);
                } else {
                    const auto it = NSTheme::map_alias_names.find(color_name.second);
                    if ( it != NSTheme::map_alias_names.end() &&
                            colorValues.contains(it->second) )
                    {
                        jsonValues[color_name.second] = colorValues.value(it->second).toString();
                    }
                }
            }

        } else {
            jsonValues = obj.value("values").toObject();
        }

//        is_system = false;
    }

    auto setDefaultThemes(const CTheme * const l, const CTheme * const d) -> void {
        defdark = d;
        deflight = l;
    }

    std::wstring id;
    std::wstring wstype;
    NSTheme::ThemeType type = NSTheme::ThemeType::ttUndef;
    bool is_system{false};

    QJsonObject jsonValues;
    const CTheme * defdark = nullptr,
            * deflight = nullptr;
    QString source_file;
    QString json;
};

/*
 * CThemes private class
*/

class CThemes::CThemesPrivate {
public:
    CThemesPrivate(CThemes * p)
        : parent(*p)
    {
        rc_themes = {
            {"theme-light", ":/themes/theme-light.json"},
            {"theme-classic-light", ":/themes/theme-classic-light.json"},
            {"theme-dark", ":/themes/theme-dark.json"},
            {"theme-contrast-dark", ":/themes/theme-contrast-dark.json"},
            {"theme-gray", ":/themes/theme-gray.json"},
            {"theme-white", ":/themes/theme-white.json"},
            {"theme-night", ":/themes/theme-night.json"},
        };

        GET_REGISTRY_USER(_reg_user);

#if !defined(__OS_WIN_XP)
        QString user_theme = _reg_user.value(REGISTRY_THEME_KEY, THEME_ID_SYSTEM).toString();
#else
        QString user_theme = _reg_user.value(REGISTRY_THEME_KEY, THEME_DEFAULT_LIGHT_ID).toString();
#endif

        /* TODO: remove for ver 7.3. for compatibility with ver 7.1 only */
        // if ( _reg_user.contains(REGISTRY_THEME_KEY_7_2) )
        //     user_theme = _reg_user.value(REGISTRY_THEME_KEY_7_2, THEME_ID_SYSTEM).toString();

#ifdef Q_OS_WIN
        QSettings _reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
        is_system_theme_dark = _reg.value("AppsUseLightTheme", 1).toInt() == 0;
#else
        dbus_conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, nullptr);
        if (dbus_conn) {
            GError *err = NULL;
            GVariant *value = g_dbus_connection_call_sync(
                dbus_conn,
                XDG_DSK_DBUS_SENDER,
                XDG_DSK_DBUS_PATH,
                XDG_STN_DBUS_ITF,
                "Read",
                g_variant_new("(ss)", "org.freedesktop.appearance", "color-scheme"),
                NULL, G_DBUS_CALL_FLAGS_NONE, -1,
                NULL, &err);

            if (!err) {
                if (value) {
                    GVariant *inner;
                    g_variant_get(value, "(v)", &inner);
                    if (inner) {
                        GVariant *final = g_variant_get_variant(inner);
                        if (final) {
                            guint32 color_scheme = g_variant_get_uint32(final);
                            g_variant_unref(final);
                            // 0: No preference
                            // 1: Prefer dark appearance
                            // 2: Prefer light appearance
                            is_system_theme_dark = (color_scheme == 1);
                        }
                        g_variant_unref(inner);
                    }
                    g_variant_unref(value);
                }
            } else {
                g_error_free(err);
            }

            settings_signal = g_dbus_connection_signal_subscribe(
                dbus_conn,
                XDG_DSK_DBUS_SENDER,
                XDG_STN_DBUS_ITF,
                "SettingChanged",
                XDG_DSK_DBUS_PATH,
                NULL, G_DBUS_SIGNAL_FLAGS_NONE,
                onSettingsChanged, NULL, NULL);
        }
#endif

        current = new CTheme;
        current->m_priv->setDefaultThemes(getDefault(NSTheme::ThemeType::ttDark), getDefault(NSTheme::ThemeType::ttLight));
        if ( user_theme.endsWith(".json") ) {
            auto search_user_theme = [](const std::vector<QString>& paths, const QString& name) -> QString
            {
                for ( auto& p: paths ) {
                    QFileInfo info{QDir(p).absoluteFilePath(name)};
                    if ( info.exists() )
                        return info.absoluteFilePath();
                }

                return "";
            };

            QString file_path = search_user_theme({Utils::getAppCommonPath() + "/uithemes",
                                                   qApp->applicationDirPath() + "/uithemes"}, user_theme);
            if ( file_path.isEmpty() || !current->fromFile(file_path) ) {
                user_theme = THEME_ID_SYSTEM;
            }
        } else
        if ( rc_themes.find(user_theme) == rc_themes.end() || !current->fromFile(rc_themes.at(user_theme)) ) {
#if !defined(__OS_WIN_XP)
            user_theme = THEME_ID_SYSTEM;
#else
            user_theme = THEME_DEFAULT_LIGHT_ID;
#endif
        }

        if ( user_theme == THEME_ID_SYSTEM ) {
            current->fromFile(rc_themes.at(is_system_theme_dark ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID));
            current->m_priv->is_system = true;
        }

#ifdef __linux__
        applyGtkTheme(current->isDark());
#endif
    }

    ~CThemesPrivate()
    {
        if ( current ) {
            delete current;
            current = nullptr;
        }

#ifdef __linux__
        if (dbus_conn) {
            if (settings_signal != 0)
                g_dbus_connection_signal_unsubscribe(dbus_conn, settings_signal);
            g_object_unref(dbus_conn);
        }
#endif
    }

    auto setCurrent(const QString& id, bool force = false) -> bool
    {
        if ( current->id() != id.toStdWString() || force ) {
            if ( id != THEME_ID_SYSTEM ) {
                delete current;

                current = new CTheme;
                current->m_priv->setDefaultThemes(getDefault(NSTheme::ThemeType::ttDark), getDefault(NSTheme::ThemeType::ttLight));
                if ( rc_themes.find(id) != rc_themes.end() ) {
                    return current->fromFile(rc_themes.at(id));
                } else
                if ( local_themes.find(id) != local_themes.end() ) {
                    if ( current->fromJson(local_themes.at(id).second) ) {
                        current->m_priv->source_file = local_themes.at(id).first;
                        return true;
                    }
//                    return current->fromJson(local_themes.at(id));
                }
            } else {
                QString visual_theme_id = is_system_theme_dark ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID;
                if ( current->id() != visual_theme_id.toStdWString() ) {
                    delete current;

                    current = new CTheme;
                    if ( !current->fromFile(rc_themes.at(visual_theme_id)) )
                        return false;
                }

                current->m_priv->is_system = true;
                return true;
            }

            return false;
        }

        return true;
    }

    auto getDefault(NSTheme::ThemeType type) -> const CTheme * const
    {
        if ( type == NSTheme::ThemeType::ttDark ) {
            if ( !dark ) {
                dark = new CTheme;
                dark->fromFile(rc_themes[THEME_DEFAULT_DARK_ID]);
            }

            return dark;
        } else {
            if ( !light ) {
                light = new CTheme;
                light->fromFile(rc_themes[THEME_DEFAULT_LIGHT_ID]);
            }

            return light;
        }
    }

    auto searchLocalThemes() -> void {
        QFileInfoList themes = QDir(qApp->applicationDirPath() + "/uithemes").entryInfoList(QStringList() << "*.json", QDir::Files);
        themes.append(QDir(Utils::getAppCommonPath() + "/uithemes").entryInfoList(QStringList() << "*.json", QDir::Files));

        foreach(auto t, themes) {
            addThemeFromFile(t.absoluteFilePath());
        }
    }

    auto parseLocalTheme(const QJsonObject& obj) -> void {

    }

    auto validateTheme(const QJsonObject& root) -> bool {
        if ( root.contains("id") && root.contains("name") ) {
            static QRegularExpression _re_legal_symb("[^\\w\\d\\-]");
//            static QRegularExpression _re_legal_color("[^\\w\\d\\-\\#]");
            if ( root.value("id").toString().contains(_re_legal_symb) ) {
                return false;
            }

            if ( root.contains("colors") ) {
                const QJsonObject _colors = root.value("colors").toObject();
                foreach (const auto& c, _colors.keys()) {
                    if ( c.contains(_re_legal_symb) /*|| _colors[c].toString().contains(_re_legal_color)*/ ) {
                        return false;
                    }
                }
            }

            return true;
        }
        return false;
    }

    auto addThemeFromFile(const QString& path) -> bool {
        QFile file(path);
        if ( file.open(QIODevice::ReadOnly) ) {
            QString fileName = file.fileName();
            QByteArray data{file.readAll()};
            file.close();

            QJsonParseError jpe;
            QJsonDocument doc = QJsonDocument::fromJson(data, &jpe);
            if ( jpe.error == QJsonParseError::NoError ) {
                QJsonObject objRoot = doc.object();

                if ( validateTheme(objRoot) ) {
                    local_themes[objRoot.value("id").toString()] = std::make_pair(fileName, data);
                    return true;
                }
            }
        }

        return false;
    }

    CThemes & parent;
    std::map<QString, QString> rc_themes;
    std::map<QString, std::pair<QString, QByteArray>> local_themes;
    bool is_system_theme_dark = false;

    CTheme * current = nullptr;
    CTheme * dark = nullptr;
    CTheme * light = nullptr;
#ifdef __linux__
    GDBusConnection *dbus_conn = nullptr;
    guint settings_signal = 0;
#endif
};

/*
 * CTheme
*/

CTheme::CTheme(const QString& path)
    : m_priv(new CTheme::CThemePrivate)
{
    if ( !path.isEmpty() )
        fromFile(path);
}

CTheme::CTheme(const CTheme &other)
    : m_priv(new CThemePrivate(*other.m_priv))
{}

CTheme::CTheme(CTheme &&other) noexcept
    : m_priv(other.m_priv)
{
    other.m_priv = nullptr;
}

CTheme::~CTheme()
{
    if ( m_priv ) {
        delete m_priv;
        m_priv = nullptr;
    }
}

CTheme& CTheme::operator=(const CTheme &other)
{
    if (this != &other) {
        if (m_priv)
            delete m_priv;
        m_priv = new CThemePrivate(*other.m_priv);
    }
    return *this;
}

CTheme& CTheme::operator=(CTheme &&other) noexcept
{
    if (this != &other) {
        if (m_priv)
            delete m_priv;
        m_priv = other.m_priv;
        other.m_priv = nullptr;
    }
    return *this;
}

auto CTheme::fromFile(const QString& path) -> bool
{
    QFile _file(path);
    if ( _file.open(QIODevice::ReadOnly) ) {
        QString data(_file.readAll());
        _file.close();
        return fromJson(data);
    }

    return false;
}

auto CTheme::fromJson(const QString& json) -> bool
{
    QJsonParseError jerror;

    QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8(), &jerror);
    if ( jerror.error == QJsonParseError::NoError ) {
        m_priv->fromJsonObject(jdoc.object());
        m_priv->json = json;
        return true;
    }

    return false;
}

auto CTheme::json() const -> QString
{
    return m_priv->json.isEmpty() ? "" : m_priv->json;
}

auto CTheme::id() const -> std::wstring
{
    return m_priv->is_system ? WSTR(THEME_ID_SYSTEM) : m_priv->id;
}

auto CTheme::originalId() const -> std::wstring
{
    return m_priv->id;
}

auto CTheme::stype() const -> QString
{
    switch (m_priv->type) {
    default:
    case NSTheme::ThemeType::ttLight: return NSTheme::theme_type_light;
    case NSTheme::ThemeType::ttDark: return NSTheme::theme_type_dark;
    }
}

auto CTheme::value(ColorRole r, const std::wstring& def) const -> std::wstring
{
    if ( NSTheme::map_names.find(r) != NSTheme::map_names.end() ) {
        if ( m_priv->jsonValues.contains(NSTheme::map_names.at(r)) ) {
            return m_priv->jsonValues.value(NSTheme::map_names.at(r)).toString().toStdWString();
        }
    }

    return def;
}

auto CTheme::color(ColorRole role) const -> QColor
{
    QString v = QSTRING_FROM_WSTR(value(role));
    if ( !v.isEmpty() ) {
        if ( v.startsWith("rgba") ) {
            QRegularExpression re("\\((\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3})");
            QRegularExpressionMatch match = re.match(v);
            if ( match.hasMatch() ) {
                const uint r = match.captured(1).toUInt(),
                            g = match.captured(2).toUInt(),
                            b = match.captured(3).toUInt(),
                            a = match.captured(4).toUInt();
                v = QString("#%1%2%3%4").arg(a, 2, 16, QLatin1Char('0'))
                                            .arg(r, 2, 16, QLatin1Char('0'))
                                            .arg(g, 2, 16, QLatin1Char('0'))
                                            .arg(b, 2, 16, QLatin1Char('0'));

                // keep normalized color
                m_priv->jsonValues[NSTheme::map_names.at(role)] = v;
            }
        }

        return QColor(v);
    }

    return QColor();
}

#ifdef Q_OS_WIN
auto CTheme::colorRef(ColorRole role) const -> COLORREF
{
    int r, g, b;
    color(role).getRgb(&r, &g, &b);

    return RGB(r,g,b);
}
#endif

auto CTheme::isDark() const -> bool
{
    return m_priv->type == NSTheme::ThemeType::ttDark;
}

auto CTheme::isSystem() const -> bool
{
    return m_priv->is_system;
}

auto CTheme::isValid() const -> bool
{
    return !m_priv->id.empty() && m_priv->type != NSTheme::ThemeType::ttUndef && !m_priv->jsonValues.isEmpty();
}

/**/

CThemes::CThemes()
    : m_priv(new CThemes::CThemesPrivate(this))
{
    m_priv->searchLocalThemes();
}

CThemes::~CThemes()
{
    if ( m_priv ) {
        delete m_priv;
        m_priv = nullptr;
    }
}

auto CThemes::current() -> const CTheme &
{
    return *(m_priv->current);
}

auto CThemes::defaultDark() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::ttDark);
}

auto CThemes::defaultLight() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::ttLight);
}

auto CThemes::localFromId(const QString &id) const -> CTheme
{
    CTheme theme;
    auto it = m_priv->rc_themes.find(id);
    if (it != m_priv->rc_themes.end()) {
        theme.fromFile(it->second);
    }
    return theme;
}

auto CThemes::setCurrentTheme(const std::wstring& name) -> void
{
    if ( !isThemeCurrent(name) && m_priv->setCurrent(QString::fromStdWString(name), true) ) {
        GET_REGISTRY_USER(_reg_user);

        if ( !m_priv->current->m_priv->source_file.isEmpty() )
            _reg_user.setValue(REGISTRY_THEME_KEY, m_priv->current->m_priv->source_file);
        else _reg_user.setValue(REGISTRY_THEME_KEY, QString::fromStdWString(name));

        // TODO: remove after ver 7.5. back to keep theme id in REGISTRY_THEME_KEY
        // if ( _reg_user.contains(REGISTRY_THEME_KEY_7_2) )
        //     _reg_user.remove(REGISTRY_THEME_KEY_7_2);

#ifdef __linux__
        applyGtkTheme(m_priv->current->isDark());
#endif
    }
}

auto CThemes::isThemeCurrent(const std::wstring& id) -> bool
{
    if ( m_priv->current->id() != id ) return false;
    else {
        if ( m_priv->current->isSystem() ) {
            return m_priv->current->isDark() == m_priv->is_system_theme_dark;
        }

        return true;
    }
}

auto CThemes::themeActualId(const std::wstring& id) const -> std::wstring
{
    return WSTR(THEME_ID_SYSTEM) != id ? id :
        m_priv->is_system_theme_dark ? WSTR(THEME_DEFAULT_DARK_ID) : WSTR(THEME_DEFAULT_LIGHT_ID);
}

auto CThemes::contains(const QString& id) -> bool
{
    return m_priv->local_themes.find(id) != m_priv->local_themes.end() ||
                m_priv->rc_themes.find(id) != m_priv->rc_themes.end();
}

auto CThemes::validate(const QJsonObject& json) -> bool
{
    return m_priv->validateTheme(json);
}

auto CThemes::isColorDark(const std::wstring& color) -> bool
{
    return isColorDark(QString::fromStdWString(color));
}

auto CThemes::isColorDark(const QString& color) -> bool
{
    int r, g, b;
    QColor(color).getRgb(&r, &g, &b);

    int luma = int(0.2126f * r) + int(0.7152f * g) + int(0.0722f * b);

    return luma < 128;
}

auto CThemes::parseThemeName(const std::wstring& wjson) -> std::wstring
{
    size_t pos = wjson.find(L"name\":");                // check if json in params
    if ( pos != std::wstring::npos ) {
        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject obj = jdoc.object();

            return obj.contains("name") ? obj["name"].toString().toStdWString() : QString(THEME_DEFAULT_LIGHT_ID).toStdWString();
        }
    }

    return wjson;
}

//auto CThemes::addLocalTheme(const std::wstring& path) -> bool
//{
//    if ( m_priv->addThemeFromFile(QString::fromStdWString(path)) ) {
//        return true;
//    }

//    return false;
//}

auto CThemes::addLocalTheme(QJsonObject& jsonobj, const QString& filepath) -> bool
{
    if ( m_priv->validateTheme(jsonobj) ) {
        if ( !filepath.isEmpty() ) {
            const QString dest_dir = getUserThemesPath();
            if ( QDir(dest_dir).mkpath(".") ) {
                QString dest_file_path = dest_dir + "/" + QFileInfo(filepath).fileName();
                if ( QFile::exists(dest_file_path) )
                    QFile::remove(dest_file_path);

                QJsonObject colors = jsonobj["colors"].toObject();
                auto theme_template = (jsonobj.contains("type") && jsonobj["type"].toString() == NSTheme::theme_type_dark) ?
                                          THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID;
                QJsonObject template_obj = Utils::parseJsonFile(m_priv->rc_themes[theme_template]);
                QJsonObject values = template_obj["values"].toObject();
                for (auto it = values.begin(); it != values.end(); it++) {
                    if (!colors.contains(it.key())) {
                        const auto alias_it = NSTheme::map_alias_names.find(it.key());
                        if (alias_it == NSTheme::map_alias_names.end()) {
                            colors.insert(it.key(), it.value());
                        } else
                        if (!colors.contains(alias_it->second))
                            colors.insert(alias_it->second, it.value());
                    }
                }
                jsonobj["colors"] = colors;
                QByteArray data = QJsonDocument(jsonobj).toJson(QJsonDocument::Compact);
                if (!Utils::writeFile(dest_file_path, data))
                    return false;
                else {
//                    m_priv->local_themes[jsonobj.value("id").toString()] = std::make_pair("", data);
                    m_priv->local_themes[jsonobj.value("id").toString()] = std::make_pair(dest_file_path, data);

                    return true;
                }
            }
        }
    }

    return false;
}

auto CThemes::localThemesToJson() -> QJsonArray
{
    QJsonArray json_themes_array;

    QJsonParseError je;
    QJsonDocument doc;
    foreach(auto const& t, m_priv->local_themes) {
        auto const& i = t.second;

        doc = QJsonDocument::fromJson(i.second, &je);
        if ( QJsonParseError::NoError == je.error ) {
            json_themes_array.append(doc.object());
        }
    }

    return json_themes_array;
}

auto CThemes::checkDestinationThemeFileExist(const QString& srcfilepath) -> bool
{
    const QString dest_file_path = getUserThemesPath() + "/" + QFileInfo(srcfilepath).fileName();
    return QFile::exists(dest_file_path);
}

auto CThemes::onSystemDarkColorScheme(bool isdark) -> void
{
    if ( isdark != m_priv->is_system_theme_dark ) {
        m_priv->is_system_theme_dark = isdark;
    }
}

auto CThemes::isSystemSchemeDark() -> const bool
{
    return m_priv->is_system_theme_dark;
}
