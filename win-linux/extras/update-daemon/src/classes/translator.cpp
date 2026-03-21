#include "translator.h"
#include <sstream>
#include <algorithm>
#ifdef _WIN32
# include "platform_win/resource.h"
# include "platform_win/utils.h"
# include <Windows.h>
# include <objidl.h>
# include <codecvt>
# define tistringstream std::wistringstream
#else
# include "platform_linux/utils.h"
# include "res/gresource.c"
# include <cstdint>
# define tistringstream std::istringstream
  typedef uint16_t WORD;
#endif


tstring getPrimaryLang(const tstring &lang, bool withScript = false)
{
    if (lang.empty()) {
        NS_Logger::WriteLog(_T("An error occurred: ") + FUNCTION_INFO);
        return _T("en");
    }
    tistringstream iss(lang);
    tstring primlang, script;
    std::getline(iss, primlang, _T('_'));
    if (primlang.length() == 2 || primlang.length() == 3) {
        if (!withScript)
            return primlang;
        std::getline(iss, script, _T('_'));
        return (script.length() == 4) ? primlang + _T("_") + script : primlang;
    }
    NS_Logger::WriteLog(_T("An error occurred: ") + FUNCTION_INFO);
    return _T("en");
}

#ifdef _WIN32
static IStream* LoadResourceToStream(int resourceId)
{
    IStream *pStream = nullptr;
    HMODULE hInst = GetModuleHandle(nullptr);
    if (HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(resourceId), RT_RCDATA)) {
        DWORD dataSize = SizeofResource(hInst, hRes);
        if (dataSize > 0) {
            if (HGLOBAL hResData = LoadResource(hInst, hRes)) {
                if (LPVOID pData = LockResource(hResData)) {
                    if (HGLOBAL hGlobal = GlobalAlloc(GHND, dataSize)) {
                        if (LPVOID pBuffer = GlobalLock(hGlobal)) {
                            memcpy(pBuffer, pData, dataSize);
                            GlobalUnlock(hGlobal);
                            HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
                            if (FAILED(hr)) {
                                GlobalFree(hGlobal);
                                pStream = nullptr;
                            }
                        } else {
                            GlobalFree(hGlobal);
                        }
                    }
                }
                FreeResource(hResData);
            }
        }
    }
    return pStream;
}

wstring StrToWStr(const string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}
#else
static GInputStream* LoadResourceToStream(const char *resourcePath)
{
    GInputStream *pStream = nullptr;
    if (GResource *res = gresource_get_resource()) {
        g_resources_register(res);
        if (GBytes *bytes = g_resource_lookup_data(res, resourcePath, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL)) {
            gsize dataSize = 0;
            const char *pData = (const char*)g_bytes_get_data(bytes, &dataSize);
            if (dataSize > 0) {
                pStream = g_memory_input_stream_new_from_data(pData, dataSize, NULL);
            }
            g_bytes_unref(bytes);
        }
        g_resource_unref(res);
    }
    return pStream;
}
#endif

Translator::Translator() :
    langName(_T("en")),
    is_translations_valid(false)
{

}

Translator& Translator::instance()
{
    static Translator inst;
    return inst;
}

#ifdef _WIN32
void Translator::init(const tstring &lang, int resourceId)
#else
void Translator::init(const tstring &lang, const char *resourcePath)
#endif
{
    langName = lang;
    std::replace(langName.begin(), langName.end(), '-', '_');
    NS_Logger::WriteLog(_T("Current locale: ") + langName);

    is_translations_valid = false;
    const char ISL_MAGIC[] = "ISL";
#ifdef _WIN32
    if (IStream *pStream = LoadResourceToStream(resourceId)) {
        ULONG bytesRead = 0;
        HRESULT hr = S_OK;
        char magic[sizeof(ISL_MAGIC)] = { 0 };
        hr = pStream->Read(magic, sizeof(magic), &bytesRead);
        if (FAILED(hr) || bytesRead != sizeof(magic) || strncmp(magic, ISL_MAGIC, sizeof(magic) - 1) != 0) {
            pStream->Release();
            return;
        }
        WORD stringsMapSize = 0;
        hr = pStream->Read(&stringsMapSize, sizeof(stringsMapSize), &bytesRead);
        if (FAILED(hr) || bytesRead != sizeof(stringsMapSize)) {
            pStream->Release();
            return;
        }
        for (WORD i = 0; i < stringsMapSize; i++) {
            uint8_t stringIdLen = 0;
            hr = pStream->Read(&stringIdLen, sizeof(stringIdLen), &bytesRead);
            if (FAILED(hr) || bytesRead != sizeof(stringIdLen)) {
                pStream->Release();
                return;
            }
            std::string stringId(stringIdLen, '\0');
            hr = pStream->Read(&stringId[0], stringIdLen, &bytesRead);
            if (FAILED(hr) || bytesRead != stringIdLen) {
                pStream->Release();
                return;
            }
            WORD localeMapSize = 0;
            hr = pStream->Read(&localeMapSize, sizeof(localeMapSize), &bytesRead);
            if (FAILED(hr) || bytesRead != sizeof(localeMapSize)) {
                pStream->Release();
                return;
            }
            LocaleMap localeMap;
            for (WORD j = 0; j < localeMapSize; j++) {
                uint8_t localeLen = 0;
                hr = pStream->Read(&localeLen, sizeof(localeLen), &bytesRead);
                if (FAILED(hr) || bytesRead != sizeof(localeLen)) {
                    pStream->Release();
                    return;
                }
                std::string localeName(localeLen, '\0');
                hr = pStream->Read(&localeName[0], localeLen, &bytesRead);
                if (FAILED(hr) || bytesRead != localeLen) {
                    pStream->Release();
                    return;
                }
                WORD translationLen = 0;
                hr = pStream->Read(&translationLen, sizeof(translationLen), &bytesRead);
                if (FAILED(hr) || bytesRead != sizeof(translationLen)) {
                    pStream->Release();
                    return;
                }
                std::string translationString(translationLen, '\0');
                hr = pStream->Read(&translationString[0], translationLen, &bytesRead);
                if (FAILED(hr) || bytesRead != translationLen) {
                    pStream->Release();
                    return;
                }
                localeMap[StrToWStr(localeName)] = StrToWStr(translationString);
            }
            translMap[StrToWStr(stringId)] = localeMap;
        }
        pStream->Release();
        is_translations_valid = true;
    }
#else
    if (GInputStream *pStream = LoadResourceToStream(resourcePath)) {
        gsize bytesRead = 0;
        gboolean hr = true;
        char magic[sizeof(ISL_MAGIC)] = { 0 };
        hr = g_input_stream_read_all(pStream, magic, sizeof(magic), &bytesRead, NULL, NULL);
        if (!hr || bytesRead != sizeof(magic) || strncmp(magic, ISL_MAGIC, sizeof(magic) - 1) != 0) {
            g_object_unref(pStream);
            return;
        }
        WORD stringsMapSize = 0;
        hr = g_input_stream_read_all(pStream, &stringsMapSize, sizeof(stringsMapSize), &bytesRead, NULL, NULL);
        if (!hr || bytesRead != sizeof(stringsMapSize)) {
            g_object_unref(pStream);
            return;
        }
        for (WORD i = 0; i < stringsMapSize; i++) {
            uint8_t stringIdLen = 0;
            hr = g_input_stream_read_all(pStream, &stringIdLen, sizeof(stringIdLen), &bytesRead, NULL, NULL);
            if (!hr || bytesRead != sizeof(stringIdLen)) {
                g_object_unref(pStream);
                return;
            }
            std::string stringId(stringIdLen, '\0');
            hr = g_input_stream_read_all(pStream, &stringId[0], stringIdLen, &bytesRead, NULL, NULL);
            if (!hr || bytesRead != stringIdLen) {
                g_object_unref(pStream);
                return;
            }
            WORD localeMapSize = 0;
            hr = g_input_stream_read_all(pStream, &localeMapSize, sizeof(localeMapSize), &bytesRead, NULL, NULL);
            if (!hr || bytesRead != sizeof(localeMapSize)) {
                g_object_unref(pStream);
                return;
            }
            LocaleMap localeMap;
            for (WORD j = 0; j < localeMapSize; j++) {
                uint8_t localeLen = 0;
                hr = g_input_stream_read_all(pStream, &localeLen, sizeof(localeLen), &bytesRead, NULL, NULL);
                if (!hr || bytesRead != sizeof(localeLen)) {
                    g_object_unref(pStream);
                    return;
                }
                std::string localeName(localeLen, '\0');
                hr = g_input_stream_read_all(pStream, &localeName[0], localeLen, &bytesRead, NULL, NULL);
                if (!hr || bytesRead != localeLen) {
                    g_object_unref(pStream);
                    return;
                }
                WORD translationLen = 0;
                hr = g_input_stream_read_all(pStream, &translationLen, sizeof(translationLen), &bytesRead, NULL, NULL);
                if (!hr || bytesRead != sizeof(translationLen)) {
                    g_object_unref(pStream);
                    return;
                }
                std::string translationString(translationLen, '\0');
                hr = g_input_stream_read_all(pStream, &translationString[0], translationLen, &bytesRead, NULL, NULL);
                if (!hr || bytesRead != translationLen) {
                    g_object_unref(pStream);
                    return;
                }
                localeMap[localeName] = translationString;
            }
            translMap[stringId] = localeMap;
        }
        g_object_unref(pStream);
        is_translations_valid = true;
    }
#endif
}

Translator::~Translator()
{

}

tstring Translator::tr(const tchar *str) const
{
    if (is_translations_valid) {
        auto it = translMap.find(str);
        if (it != translMap.end()) {
            const LocaleMap &lcmap = it->second;
            auto lc_it = lcmap.find(langName);
            if (lc_it == lcmap.end()) {
                tstring primaryLangAndScript = getPrimaryLang(langName, true);
                if ((lc_it = lcmap.find(primaryLangAndScript)) == lcmap.end()) {
                    tstring primaryLang = getPrimaryLang(langName);
                    if ((lc_it = lcmap.find(primaryLang)) == lcmap.end()) {
                        lc_it = lcmap.find(_T("en"));
                    }
                }
            }
            if (lc_it != lcmap.end())
                return lc_it->second;
        }
    }
    return str;
}

void Translator::setLanguage(const tstring &lang)
{
    langName = lang;
    std::replace(langName.begin(), langName.end(), '-', '_');
    NS_Logger::WriteLog(_T("Current locale: ") + langName);
}
