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
