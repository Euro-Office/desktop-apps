#include "clangater.h"
#include "defines.h"
#include "defines_p.h"
#include "utils.h"
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QTranslator>
#include <QRegularExpression>
#include <list>
#include <algorithm>


#ifdef _WIN32
# ifndef __OS_WIN_XP
#  include <commctrl.h>


bool resolveLocaleName(LPCWSTR nameToResolve, LPWSTR localeName, int localeNameSize)
{
    int(WINAPI *_ResolveLocaleName)(LPCWSTR, LPWSTR, int) = NULL;
    if (HMODULE module = GetModuleHandleA("kernel32"))
        *(FARPROC*)&_ResolveLocaleName = GetProcAddress(module, "ResolveLocaleName");
    return _ResolveLocaleName ? _ResolveLocaleName(nameToResolve, localeName, localeNameSize) != 0 : false;
}
# endif

void setNativeUILanguage(std::wstring localeTag)
{
    if (localeTag.empty())
        return;
#ifndef __OS_WIN_XP
    WCHAR localeName[LOCALE_NAME_MAX_LENGTH] = {0};
    if (resolveLocaleName(localeTag.c_str(), localeName, LOCALE_NAME_MAX_LENGTH) && wcslen(localeName) != 0)
        localeTag = localeName;
    if (!IsValidLocaleName(localeTag.c_str()))
        return;
    ULONG langCount = 1;
    if (GetLocaleInfo(LOCALE_CUSTOM_UI_DEFAULT, LOCALE_SNAME, localeName, LOCALE_NAME_MAX_LENGTH) > 0) {
        if (wcscmp(localeTag.c_str(), localeName) != 0) {
            ++langCount;
            localeTag.append(L";");
            localeTag.append(localeName);
        }
    }
    std::replace(localeTag.begin(), localeTag.end(), L';', L'\0');
    localeTag.push_back(L'\0');
    SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, (PCZZWSTR)localeTag.c_str(), &langCount);
    LCID lcid = LocaleNameToLCID(localeTag.c_str(), 0);
    if (lcid != 0)
        InitMUILanguage(LANGIDFROMLCID(lcid));
#else
//    LCID lcid = LocaleNameToLCID(localeTag.c_str(), 0);
//    if (lcid != 0 && IsValidLocale(lcid, LCID_INSTALLED) != 0)
//        SetThreadUILanguage(LANGIDFROMLCID(lcid));
#endif
}
#endif

QString normalizeLocale(const QString &locale)
{
    int len = locale.length();
    if (len > 1) {
        int prim = locale.indexOf(QRegularExpression("[-_]"));
        if (prim == -1) {
            if (len <= 3)
                return locale.toLower();
        } else
        if (prim == 2 || prim == 3) {
            QString out = locale.mid(0, prim).toLower();
            int scnd = locale.indexOf(QRegularExpression("[-_]"), prim + 1);
            if ((scnd == -1 && len - prim == 3) || (scnd != -1 && scnd - prim == 3)) {
                out += "-" + locale.mid(prim + 1, 2).toUpper();
            } else
            if ((scnd == -1 && len - prim == 5) || (scnd != -1 && scnd - prim == 5)) {
                out += QString("-") + locale.at(prim + 1).toUpper() + locale.mid(prim + 2, 3).toLower();
                if (scnd != -1) {
                    int thrd = locale.indexOf(QRegularExpression("[-_]"), scnd + 1);
                    if ((thrd == -1 && len - scnd == 3) || (thrd != -1 && thrd - scnd == 3))
                        out += "-" + locale.mid(scnd + 1, 2).toUpper();
                }
            }
            return out;
        }
    }
    return "en";
}

enum class LocaleParts : unsigned char {Language, LanguageAndScript, LanguageAndRegion};

QString getLocaleParts(const QString &locale, LocaleParts parts = LocaleParts::Language)
{
    int len = locale.length();
    if (len > 1) {
        int prim = locale.indexOf('-');
        if (prim == -1) {
            if (len <= 3)
                return locale;
        } else
        if (prim == 2 || prim == 3) {
            QString out = locale.mid(0, prim);
            int scnd = locale.indexOf('-', prim + 1);
            if (parts == LocaleParts::LanguageAndScript) {
                if ((scnd == -1 && len - prim == 5) || (scnd != -1 && scnd - prim == 5))
                    return locale.mid(0, prim + 5);
            } else
            if (parts == LocaleParts::LanguageAndRegion) {
                if ((scnd == -1 && len - prim == 3) || (scnd != -1 && scnd - prim == 3)) {
                    return locale.mid(0, prim + 3);
                } else
                if (scnd != -1 && scnd - prim == 5) {
                    int thrd = locale.indexOf('-', scnd + 1);
                    if ((thrd == -1 && len - scnd == 3) || (thrd != -1 && thrd - scnd == 3))
                        out += locale.mid(scnd, 3);
                }
            }
            return out;
        }
    }
    return "en";
}

class CLangater::CLangaterIntf
{
    friend class CLangater;
public:
    QTranslator * createTranslator()
    {
        m_list.push_back(new QTranslator);
        return m_list.back();
    }

    QTranslator * createTranslator(const QString& file)
    {
        QTranslator * t = nullptr;
        for (const auto &d: m_dirs) {
            t = createTranslator(file, d);
            if ( t ) break;
        }

        return t;
    }

    QTranslator * createTranslator(const QString& file, const QString& path)
    {
        auto * t = new QTranslator;
        if ( t->load(file, path) ) {
            m_list.push_back(t);

            auto d = std::find(m_dirs.begin(), m_dirs.end(), path);
            if ( d == m_dirs.end() )
                m_dirs.push_back(QString(path));
        } else {
            delete t,
            t = nullptr;
        }

        return t;
    }

    ~CLangaterIntf()
    {
        if ( !m_list.empty() ) {
            for (auto p: m_list) {
                QTranslator * t = p;
                if ( !t->parent() )
                    delete t;
            }
        }
    }

    QString langName(const QString& code) {
        foreach (const auto &lang, m_langs) {
            if (lang.first == code)
                return lang.second.first;
        }
        return "";
    }

    bool reload(const QString& lang) {
        QTranslator * t;
        for (auto p: m_list) {
            t = p;
            if ( !t->parent() )
                delete t;
        }

        m_list.clear();

        bool _res = false;
        for (auto d: m_dirs) {
            t = createTranslator(lang, d);
            if ( !_res && t ) _res = true;
        }

        return _res;
    }

    void addSearchPath(const QString& path) {
        auto d = std::find(m_dirs.begin(), m_dirs.end(), path);
        if ( d == m_dirs.end() )
            m_dirs.push_back(QString(path));
    }

    void addSearchPath(const std::list<QString>& l) {
        m_dirs.insert(std::end(m_dirs), std::begin(l), std::end(l));
    }

    QString findCloseLang(const QString& n) {
        auto i = m_langs.begin();
        while ( i != m_langs.end() ) {
            if ( i->first.startsWith(n) )
                return i->first;

            ++i;
        }

        return "";
    }

private:
    std::list<QTranslator *> m_list;
    std::list<QString> m_dirs;

    QVector<std::pair<QString, std::pair<QString, QString>>> m_langs{
        {"en-US", {"English (United States)", ""}},
        {"en-GB", {"English (United Kingdom)", ""}},
        {"ru-RU", {"Русский", "Russian"}},
        {"de-DE", {"Deutsch", "German"}},
        {"fr-FR", {"Français", "French"}},
        {"es-ES", {"Español", "Spanish"}},
        {"sk-SK", {"Slovenčina", "Slovak"}},
        {"cs-CZ", {"Čeština", "Czech"}},
        {"it-IT", {"Italiano", "Italian"}},
        {"pt-PT", {"Português (Portugal)", "Portuguese (Portugal)"}},
        {"pt-BR", {"Português Brasileiro", "Portuguese (Brazil)"}}
        ,{"pl-PL", {"Polski", "Polish"}}
        ,{"zh-CN", {"简体中文", "Chinese (Simplified)"}}
        ,{"zh-TW", {"繁體中文", "Chinese (Traditional)"}}
        ,{"ca-ES", {"Catalan", "Catalan"}}
        ,{"da-DK", {"Dansk", "Danish"}}
        ,{"el-GR", {"Ελληνικά", "Greek"}}
//        ,{"et-EE", "Eesti"}
        ,{"fi-FI", {"Suomi", "Finnish"}}
//        ,{"ga-IE", {"Gaeilge", "Galego"}}
        ,{"he-IL", {"עברית", "Hebrew"}}
//        ,{"hi-IN", "हिन्दी"}
//        ,{"hr-HR", "Hrvatska"}
        ,{"hu-HU", {"Magyar", "Hungarian"}}
        ,{"hy-AM", {"Հայերեն", "Armenian"}}
        ,{"id-ID", {"Indonesian", "Indonesian"}}
        ,{"no-NO", {"Norsk", "Norwegian"}}
        ,{"ro-RO", {"Romanian", "Romanian"}}
        ,{"sl-SI", {"Slovene", "Slovenian"}}
        ,{"sq-AL", {"Shqip", "Albanian"}}
        ,{"sv-SE", {"Svenska", "Swedish"}}
        ,{"tr-TR", {"Türkçe", "Turkish"}}
        ,{"ja-JP", {"日本語", "Japanese"}}
        ,{"ko-KR", {"한국어", "Korean"}}
        ,{"bg-BG", {"Български", "Bulgarian"}}
        ,{"nl-NL", {"Nederlands", "Dutch"}}
        ,{"vi-VN", {"Tiếng Việt", "Vietnamese"}}
        ,{"lv-LV", {"Latviešu valoda", "Latvian"}}
//        ,{"lt-LT", "Lietuvių kalba"}
        ,{"be-BY", {"Беларуская мова", "Belarusian"}}
        ,{"uk-UA", {"Украї́нська мо́ва", "Ukrainian"}}
        ,{"ur-PK", {"اردو", "Urdu"}}
        ,{"lo-LA", {"ພາສາລາວ", "Lao"}}
        ,{"gl-ES", {"Galego", "Galego"}}
        ,{"si-LK", {"සිංහල", "Sinhala (Sri Lanka)"}}
        ,{"ar-SA", {"اَلْعَرَبِيَّة", "Arabic"}}
        ,{"sr-Latn-RS", {"Srpski (Latin)", "Serbian (Latin)"}}
        ,{"sr-Cyrl-RS", {"Српски (Ћирилица)", "Serbian (Cyrillic)"}}
    };
};

CLangater::CLangater()
    : m_intf(new CLangater::CLangaterIntf)
{
}

CLangater::~CLangater()
{
    delete m_intf, m_intf = nullptr;
}

CLangater * CLangater::getInstance()
{
    static CLangater _instance;
    return &_instance;
}

void CLangater::init()
{
    GET_REGISTRY_USER(reg_user)
    GET_REGISTRY_SYSTEM(reg_system)

    QString _lang = QString::fromStdWString(InputArgs::argument_value(L"--keeplang"));
    if ( _lang.isEmpty() ) {
        _lang = QString::fromStdWString(InputArgs::argument_value(L"--lang"));
    } else reg_user.setValue("locale", _lang);

    if ( _lang.isEmpty() )
        _lang = reg_user.value("locale").value<QString>();

#ifdef __linux
//    if ( _lang.isEmpty() ) {
//        _lang = QLocale::system().name();
//    }

    if ( _lang.isEmpty() ) {
        if ( APP_DEFAULT_SYSTEM_LOCALE ) {
            QRegularExpression _re;
            QString _env_name = qgetenv("LANG");
            _re.setPattern("^(\\w{2,5})\\.?");
            QRegularExpressionMatch _re_match = _re.match(_env_name);

            if ( _re_match.hasMatch() ) {
                _lang = _re_match.captured(1);
            }
        } else _lang = APP_DEFAULT_LOCALE;
    }
#else
    // read setup language and set application locale
    _lang.isEmpty() &&
        !((_lang = reg_system.value("locale").value<QString>()).size()) && (_lang = APP_DEFAULT_LOCALE).size();
#endif
    _lang = normalizeLocale(_lang);
    getInstance()->m_intf->addSearchPath({"./langs", ":/i18n/langs", ":/i18n"});

    auto _check_lang = [=](const std::list<QString>& dirs, const QString& l) {
        for ( auto& d : dirs ) {
            if ( QFile(d + "/" + l + ".qm").exists() ) return true;
        }

        return false;
    };

    bool _exist = _check_lang(getInstance()->m_intf->m_dirs, _lang);
    if ( !_exist ) {
        QString lang_part = getLocaleParts(_lang, LocaleParts::LanguageAndScript);
        QString _close_lang = getInstance()->m_intf->findCloseLang(lang_part);
        if (_close_lang.isEmpty() && lang_part.length() > 3) {
            lang_part = getLocaleParts(_lang, LocaleParts::LanguageAndRegion);
            _close_lang = getInstance()->m_intf->findCloseLang(lang_part);
            if (_close_lang.isEmpty() && lang_part.length() > 3) {
                lang_part = getLocaleParts(_lang);
                _close_lang = getInstance()->m_intf->findCloseLang(lang_part);
            }
        }
        if ( !_close_lang.isEmpty() )
            _lang = _close_lang,
            _exist = _check_lang(getInstance()->m_intf->m_dirs, _lang);
    }

    if ( !_exist ) {
        if ( _lang.at(2) == '-' ) _lang.replace(2, 1, '_'); else
        if ( _lang.at(2) == '_' ) _lang.replace(2, 1, '-');

        if ( !_check_lang(getInstance()->m_intf->m_dirs, _lang) ) {
            _lang = APP_DEFAULT_LOCALE;
        }
    }

    QTranslator * tr = getInstance()->m_intf->createTranslator("qtbase_" + _lang);
    if ( tr ) QCoreApplication::installTranslator(tr);

    tr = getInstance()->m_intf->createTranslator(_lang);
    if ( tr ) getInstance()->m_lang = _lang;

    QCoreApplication::installTranslator(tr);

#ifdef _WIN32
    setNativeUILanguage(_lang.toStdWString());
#endif
}

void CLangater::reloadTranslations(const QString& lang)
{
    for ( auto p : getInstance()->m_intf->m_list ) {
        QCoreApplication::removeTranslator(p);
    }

    getInstance()->m_intf->createTranslator("qtbase_" + lang);
    if ( getInstance()->m_intf->reload(lang) ) {
        getInstance()->m_lang = lang;

        for ( auto t : getInstance()->m_intf->m_list ) {
            QCoreApplication::installTranslator(t);
        }

        emit getInstance()->onLangChanged(lang);
    }

#ifdef _WIN32
    setNativeUILanguage(lang.toStdWString());
#endif
}

void CLangater::refreshLangs(const QVector<std::pair<QString, std::pair<QString, QString>>>& map)
{
    getInstance()->m_intf->m_langs = {map};
}

QString CLangater::getCurrentLangCode()
{
    return getInstance()->m_lang.isEmpty() ? "en" : getInstance()->m_lang;
}

QString CLangater::getLangName(const QString& code)
{
    if ( code.isEmpty() ) {
        return getInstance()->m_lang;
    }

    return getInstance()->m_intf->langName(code);
}

QJsonObject CLangater::availableLangsToJson()
{
    QJsonObject _out_obj;
    QMap<QString, std::pair<QString, QString>> langMap;
    foreach (const auto &lang, getInstance()->m_intf->m_langs)
        langMap.insert(lang.first, lang.second);

    auto i = langMap.constBegin();
    while ( i != langMap.constEnd() ) {
        _out_obj.insert(i.key(), QJsonObject{{"name", i.value().first},{"enname", i.value().second}});
        ++i;
    }

    return _out_obj;
}

void CLangater::addTranslation(const QString& dir, const QString& lang)
{
    QTranslator * tr = getInstance()->m_intf->createTranslator(lang, dir);
    if ( tr ) {
        QCoreApplication::installTranslator(tr);
    }
}

void CLangater::addTranslation(const QString& dir)
{
    addTranslation(dir, getInstance()->m_lang);
}

bool CLangater::isRtlLanguage(const QString &lang) {
    QLocale loc = lang.isEmpty() ? QLocale::system() : QLocale(lang);
    return loc.textDirection() == Qt::LayoutDirection::RightToLeft;
}
