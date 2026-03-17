#define _GNU_SOURCE 1
#include "xdgdesktopportal.h"
#include "components/cmessage.h"
#include "platform_linux/xcbutils.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <sys/syscall.h>
#include <linux/random.h>
#include <X11/Xlib.h>

#if defined(__x86_64__)
# define GETRANDOM_NR 318
#elif defined(__i386__)
# define GETRANDOM_NR 355
#elif defined(__arm__)
# define GETRANDOM_NR 384
#elif defined(__ppc64le__)
# define GETRANDOM_NR 359
#elif defined(__PPC64LE__)
# define GETRANDOM_NR 359
#elif defined(__ppc64__)
# define GETRANDOM_NR 359
#elif defined(__PPC64__)
# define GETRANDOM_NR 359
#elif defined(__s390x__)
# define GETRANDOM_NR 349
#elif defined(__s390__)
# define GETRANDOM_NR 349
#endif

#if defined(SYS_getrandom)
# if defined(GETRANDOM_NR)
static_assert(GETRANDOM_NR == SYS_getrandom, "GETRANDOM_NR should match the actual SYS_getrandom value");
# endif
#else
# define SYS_getrandom GETRANDOM_NR
#endif

#if defined(GRND_NONBLOCK)
static_assert(GRND_NONBLOCK == 1, "If GRND_NONBLOCK is not 1 the #define below is wrong");
#else
# define GRND_NONBLOCK 1
#endif

#define __dbusOpen dbus_message_iter_open_container
#define __dbusClose dbus_message_iter_close_container
#define __dbusAppend dbus_message_iter_append_basic
//#define ADD_EXTENSION // not reccomended

typedef unsigned int uint;
const char URI_PREFIX[] = "file://";
constexpr size_t URI_PREFIX_SIZE = sizeof(URI_PREFIX) - 1;
const char* dbus_unique_name = nullptr;
const char* error_code = nullptr;
DBusConnection* dbus_conn;
DBusError dbus_err;

enum class EntryType : uchar {
    Directory, Multiple
};

enum Result {
    SUCCESS, ERROR, CANCEL
};

struct FilterItem {
    const char* name;
    const char* pattern;
};

Result initDBus(void);
const char* getErrorText(void);
char* strcopy(const char* start, const char* end, char* out);
void quitDBus(void);
void clearDBusError(void);
void setErrorText(const char* msg);
void Free(void* p);
void freePath(char* filePath);


struct UnrefLater_DBusMessage {
    UnrefLater_DBusMessage(DBusMessage *_msg) noexcept :
        msg(_msg) {}
    ~UnrefLater_DBusMessage() {
        dbus_message_unref(msg);
    }
    DBusMessage *msg;
};

struct FreeLater {
    FreeLater(char* p) noexcept :
        ptr(p) {}
    ~FreeLater() {
        Free(ptr);
    }
    char* ptr;
};

class DBusSignalHandler
{
public:
    DBusSignalHandler() :
        resp_path(nullptr)
    {}
    ~DBusSignalHandler() {
        if (resp_path)
            unsubscribe();
    }

    Result subscribe(const char* unique_path) {
        if (resp_path)
            unsubscribe();
        resp_path = CreateResponsePath(unique_path, dbus_unique_name);
        DBusError err;
        dbus_error_init(&err);
        dbus_bus_add_match(dbus_conn, resp_path, &err);
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&dbus_err);
            dbus_move_error(&err, &dbus_err);
            setErrorText(dbus_err.message);
            return ERROR;
        }
        return SUCCESS;
    }

    void unsubscribe() {
        DBusError err;
        dbus_error_init(&err);
        dbus_bus_remove_match(dbus_conn, resp_path, &err);
        Free(resp_path);
        dbus_error_free(&err);
    }

private:
    char* resp_path;
    static char* CreateResponsePath(const char* unique_path, const char* unique_name) {
        constexpr const char PART_1[] = "type='signal',sender='org.freedesktop.portal.Desktop',path='";
        constexpr const char PART_2[] = "',interface='org.freedesktop.portal.Request',member='Response',destination='";
        constexpr const char PART_3[] = "'";
        constexpr const char PART_1_SIZE = sizeof(PART_1) - 1;
        constexpr const char PART_2_SIZE = sizeof(PART_2) - 1;
        constexpr const char PART_3_SIZE = sizeof(PART_3) - 1;

        const size_t handle_len = strlen(unique_path);
        const size_t unique_len = strlen(unique_name);
        const size_t len = PART_1_SIZE + handle_len +
                           PART_2_SIZE + unique_len +
                           PART_3_SIZE;
        char* path = (char*)malloc(len + 1);
        char* path_ptr = path;
        path_ptr = strcopy(PART_1, PART_1 + PART_1_SIZE, path_ptr);
        path_ptr = strcopy(unique_path, unique_path + handle_len, path_ptr);
        path_ptr = strcopy(PART_2, PART_2 + PART_2_SIZE, path_ptr);
        path_ptr = strcopy(unique_name, unique_name + unique_len, path_ptr);
        path_ptr = strcopy(PART_3, PART_3 + PART_3_SIZE, path_ptr);
        *path_ptr = '\0';
        return path;
    }
}; // DBusSignalHandler

template <class Fn>
char* replaceSymbol(const char *start, const char *end, char *out, Fn func) {
    for (; start != end; ++start) {
        *out++ = func(*start);
    }
    return out;
}

void setOpenFileEntryType(DBusMessageIter &msg_iter, EntryType entry_type) {
    const char* ENTRY_TYPE = (entry_type == EntryType::Multiple) ? "multiple" : "directory";
    DBusMessageIter iter;
    DBusMessageIter var_iter;
    __dbusOpen(&msg_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &iter);
    __dbusAppend(&iter, DBUS_TYPE_STRING, &ENTRY_TYPE);
    __dbusOpen(&iter, DBUS_TYPE_VARIANT, "b", &var_iter);
    {
        int val = 1;
        __dbusAppend(&var_iter, DBUS_TYPE_BOOLEAN, &val);
    }
    __dbusClose(&iter, &var_iter);
    __dbusClose(&msg_iter, &iter);
}

void setHandleToken(DBusMessageIter &msg_iter, const char *handle_token) {
    const char* HANDLE_TOKEN = "handle_token";
    DBusMessageIter iter;
    DBusMessageIter var_iter;
    __dbusOpen(&msg_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &iter);
    __dbusAppend(&iter, DBUS_TYPE_STRING, &HANDLE_TOKEN);
    __dbusOpen(&iter, DBUS_TYPE_VARIANT, "s", &var_iter);
    __dbusAppend(&var_iter, DBUS_TYPE_STRING, &handle_token);
    __dbusClose(&iter, &var_iter);
    __dbusClose(&msg_iter, &iter);
}

void setFilter(DBusMessageIter &msg_iter, const FilterItem &filterItem) {
    DBusMessageIter struct_iter;
    DBusMessageIter array_iter;
    DBusMessageIter array_struct_iter;
    __dbusOpen(&msg_iter, DBUS_TYPE_STRUCT, nullptr, &struct_iter);
    // add filter name
    __dbusAppend(&struct_iter, DBUS_TYPE_STRING, &filterItem.name);

    // add filter extentions
    __dbusOpen(&struct_iter, DBUS_TYPE_ARRAY, "(us)", &array_iter);
    const QString patterns = QString::fromUtf8(filterItem.pattern);
    foreach (auto &pattern, patterns.split(' ')) {
        __dbusOpen(&array_iter, DBUS_TYPE_STRUCT, nullptr, &array_struct_iter);
        {
            const unsigned nil = 0;
            __dbusAppend(&array_struct_iter, DBUS_TYPE_UINT32, &nil);
        }
        char *ptrn = pattern.toUtf8().data();
        __dbusAppend(&array_struct_iter, DBUS_TYPE_STRING, &ptrn);
        __dbusClose(&array_iter, &array_struct_iter);
    }
    __dbusClose(&struct_iter, &array_iter);
    __dbusClose(&msg_iter, &struct_iter);
}

void setFilters(DBusMessageIter &msg_iter, const FilterItem *filterList, uint filterCount, FilterItem *selFilter) {
    if (filterCount != 0) {
        DBusMessageIter dict_iter, var_iter, arr_iter;
        const char* FILTERS = "filters";
        const char* CURRENT_FILTER = "current_filter";
        // set filters
        __dbusOpen(&msg_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_iter);
        __dbusAppend(&dict_iter, DBUS_TYPE_STRING, &FILTERS);
        __dbusOpen(&dict_iter, DBUS_TYPE_VARIANT, "a(sa(us))", &var_iter);
        __dbusOpen(&var_iter, DBUS_TYPE_ARRAY, "(sa(us))", &arr_iter);
        for (uint i = 0; i != filterCount; ++i) {
            setFilter(arr_iter, filterList[i]);
        }
        __dbusClose(&var_iter, &arr_iter);
        __dbusClose(&dict_iter, &var_iter);
        __dbusClose(&msg_iter, &dict_iter);

        // set current filter
        if (selFilter && selFilter->name && selFilter->pattern) {
            __dbusOpen(&msg_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_iter);
            __dbusAppend(&dict_iter, DBUS_TYPE_STRING, &CURRENT_FILTER);
            __dbusOpen(&dict_iter, DBUS_TYPE_VARIANT, "(sa(us))", &var_iter);
            setFilter(var_iter, *selFilter);
            __dbusClose(&dict_iter, &var_iter);
            __dbusClose(&msg_iter, &dict_iter);
        }
    }
}

void setCurrentName(DBusMessageIter &msg_iter, const char *name) {
    if (!name)
        return;
    const char* CURRENT_NAME = "current_name";
    DBusMessageIter dict_iter, variant_iter;
    __dbusOpen(&msg_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_iter);
    __dbusAppend(&dict_iter, DBUS_TYPE_STRING, &CURRENT_NAME);
    __dbusOpen(&dict_iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
    __dbusAppend(&variant_iter, DBUS_TYPE_STRING, &name);
    __dbusClose(&dict_iter, &variant_iter);
    __dbusClose(&msg_iter, &dict_iter);
}

void setCurrentFolder(DBusMessageIter &msg_iter, const char *path) {
    if (!path)
        return;
    const char* CURRENT_FOLDER = "current_folder";
    DBusMessageIter dict_iter, var_iter, arr_iter;
    __dbusOpen(&msg_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_iter);
    __dbusAppend(&dict_iter, DBUS_TYPE_STRING, &CURRENT_FOLDER);
    __dbusOpen(&dict_iter, DBUS_TYPE_VARIANT, "ay", &var_iter);
    __dbusOpen(&var_iter, DBUS_TYPE_ARRAY, "y", &arr_iter);
    // append string as byte array
    const char *p = path;
    do {
        __dbusAppend(&arr_iter, DBUS_TYPE_BYTE, p);
    } while (*p++);
    __dbusClose(&var_iter, &arr_iter);
    __dbusClose(&dict_iter, &var_iter);
    __dbusClose(&msg_iter, &dict_iter);
}

void setCurrentFile(DBusMessageIter &msg_iter, const char *path, const char *name) {
    if (!path || !name)
        return;
    const size_t path_len = strlen(path);
    const size_t name_len = strlen(name);
    char* pathname;
    char* pathname_end;
    size_t pathname_len;
    if (path_len && path[path_len - 1] == '/') {
        pathname_len = path_len + name_len;
        pathname = (char*)malloc(pathname_len + 1);
        pathname_end = pathname;
        pathname_end = strcopy(path, path + path_len, pathname_end);
        pathname_end = strcopy(name, name + name_len, pathname_end);
        *pathname_end++ = '\0';
    } else {
        pathname_len = path_len + 1 + name_len;
        pathname = (char*)malloc(pathname_len + 1);
        pathname_end = pathname;
        pathname_end = strcopy(path, path + path_len, pathname_end);
        *pathname_end++ = '/';
        pathname_end = strcopy(name, name + name_len, pathname_end);
        *pathname_end++ = '\0';
    }
    FreeLater __freeLater(pathname);
    if (access(pathname, F_OK) != 0)
        return;
    const char* CURRENT_FILE = "current_file";
    DBusMessageIter dict_iter, var_iter, arr_iter;
    __dbusOpen(&msg_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_iter);
    __dbusAppend(&dict_iter, DBUS_TYPE_STRING, &CURRENT_FILE);
    __dbusOpen(&dict_iter, DBUS_TYPE_VARIANT, "ay", &var_iter);
    __dbusOpen(&var_iter, DBUS_TYPE_ARRAY, "y", &arr_iter);
    // includes the '\0'
    for (const char* p = pathname; p != pathname_end; ++p) {
        __dbusAppend(&arr_iter, DBUS_TYPE_BYTE, p);
    }
    __dbusClose(&var_iter, &arr_iter);
    __dbusClose(&dict_iter, &var_iter);
    __dbusClose(&msg_iter, &dict_iter);
}

Result readDictImpl(const char*, DBusMessageIter&) {
    return SUCCESS;
}

template <class Fn, typename... Args>
Result readDictImpl(const char* key, DBusMessageIter& msg, const char* &candidate_key, Fn& callback, Args&... args) {
    return strcmp(key, candidate_key) == 0 ? callback(msg) : readDictImpl(key, msg, args...);
}

template <typename... Args>
Result readDict(DBusMessageIter msg, Args... args) {
    if (dbus_message_iter_get_arg_type(&msg) != DBUS_TYPE_ARRAY) {
        setErrorText("D-Bus response is not an array");
        return ERROR;
    }
    DBusMessageIter dict_iter;
    dbus_message_iter_recurse(&msg, &dict_iter);
    while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter iter;
        dbus_message_iter_recurse(&dict_iter, &iter);
        if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) {
            setErrorText("D-Bus response dict entry does not start with a string");
            return ERROR;
        }
        const char* key;
        dbus_message_iter_get_basic(&iter, &key);
        if (!dbus_message_iter_next(&iter)) {
            setErrorText("D-Bus response dict entry is missing arguments");
            return ERROR;
        }
        if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT) {
            setErrorText("D-Bus response dict entry is not a variant");
            return ERROR;
        }
        DBusMessageIter var_iter;
        dbus_message_iter_recurse(&iter, &var_iter);
        if (readDictImpl(key, var_iter, args...) == ERROR)
            return ERROR;
        if (!dbus_message_iter_next(&dict_iter))
            break;
    }
    return SUCCESS;
}

Result readResponseResults(DBusMessage *msg, DBusMessageIter &resIter) {
    DBusMessageIter iter;
    if (!dbus_message_iter_init(msg, &iter)) {
        setErrorText("D-Bus response is missing arguments");
        return ERROR;
    }
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT32) {
        setErrorText("D-Bus response argument is not a uint32");
        return ERROR;
    }
    dbus_uint32_t resp_code;
    dbus_message_iter_get_basic(&iter, &resp_code);
    if (resp_code != 0) {
        if (resp_code == 1) {
            return CANCEL;
        } else {
            setErrorText("D-Bus file dialog interaction was ended abruptly");
            return ERROR;
        }
    }
    if (!dbus_message_iter_next(&iter)) {
        setErrorText("D-Bus response is missing arguments");
        return ERROR;
    }
    resIter = iter;
    return SUCCESS;
}

Result readResponseUris(DBusMessage* msg, DBusMessageIter& uriIter) {
    DBusMessageIter iter;
    const Result res = readResponseResults(msg, iter);
    if (res != SUCCESS)
        return res;
    bool has_uris = false;
    if (readDict(iter, "uris", [&uriIter, &has_uris](DBusMessageIter &uris_iter) {
            if (dbus_message_iter_get_arg_type(&uris_iter) != DBUS_TYPE_ARRAY) {
                setErrorText("D-Bus response URI is not an array");
                return ERROR;
            }
            dbus_message_iter_recurse(&uris_iter, &uriIter);
            has_uris = true;
            return SUCCESS;
        }) == ERROR)
        return ERROR;

    if (!has_uris) {
        setErrorText("D-Bus response has no URI");
        return ERROR;
    }
    return SUCCESS;
}

void readResponseUrisUnchecked(DBusMessage* msg, DBusMessageIter& uriIter) {
    DBusMessageIter iter;
    dbus_message_iter_init(msg, &iter);
    dbus_message_iter_next(&iter);
    readDict(iter, "uris", [&uriIter](DBusMessageIter& uris_iter) {
        dbus_message_iter_recurse(&uris_iter, &uriIter);
        return SUCCESS;
    });
}

uint readResponseUrisUncheckedSize(DBusMessage* msg) {
    DBusMessageIter iter;
    dbus_message_iter_init(msg, &iter);
    dbus_message_iter_next(&iter);
    uint arr_size = 0;
    readDict(iter, "uris", [&arr_size](DBusMessageIter& uris_iter) {
        //arr_size = dbus_message_iter_get_element_count(&uris_iter);
        // elements count for old D-Bus versions
        if (dbus_message_iter_get_arg_type(&uris_iter) == DBUS_TYPE_ARRAY) {
            DBusMessageIter arr_iter;
            dbus_message_iter_recurse(&uris_iter, &arr_iter);
            while (dbus_message_iter_get_arg_type(&arr_iter) == DBUS_TYPE_STRING) {
                 ++arr_size;
                 if (!dbus_message_iter_next(&arr_iter))
                     break;
            }
        }
        return SUCCESS;
    });
    return arr_size;
}

Result readResponseCurrentFilter(DBusMessage* msg, FilterItem* selFilter) {
    DBusMessageIter iter;
    const Result res = readResponseResults(msg, iter);
    if (res != SUCCESS)
        return res;
    const char* tmp_curr_filter = nullptr;
    const char* tmp_extn = nullptr;
    if (readDict(iter,
                 "current_filter",
                [&tmp_extn, &tmp_curr_filter](DBusMessageIter& curr_flt_iter) {
                    if (dbus_message_iter_get_arg_type(&curr_flt_iter) != DBUS_TYPE_STRUCT) {
                         // D-Bus response current_filter is not a struct
                         return SUCCESS;
                    }
                    DBusMessageIter curr_flt_struct_iter;
                    dbus_message_iter_recurse(&curr_flt_iter, &curr_flt_struct_iter);
                    if (dbus_message_iter_get_arg_type(&curr_flt_struct_iter) == DBUS_TYPE_STRING) {
                         // Get current filter
                         dbus_message_iter_get_basic(&curr_flt_struct_iter, &tmp_curr_filter);
                    }
                    if (!dbus_message_iter_next(&curr_flt_struct_iter)) {
                         // D-Bus response current_filter struct ended prematurely
                         return SUCCESS;
                    }
                    if (dbus_message_iter_get_arg_type(&curr_flt_struct_iter) != DBUS_TYPE_ARRAY) {
                         // D-Bus response URI is not a string
                         return SUCCESS;
                    }
                    DBusMessageIter curr_flt_arr_iter;
                    dbus_message_iter_recurse(&curr_flt_struct_iter, &curr_flt_arr_iter);
                    if (dbus_message_iter_get_arg_type(&curr_flt_arr_iter) != DBUS_TYPE_STRUCT) {
                         // D-Bus response current_filter is not a struct
                         return SUCCESS;
                    }
                    DBusMessageIter curr_flt_extn_iter;
                    dbus_message_iter_recurse(&curr_flt_arr_iter, &curr_flt_extn_iter);
                    if (dbus_message_iter_get_arg_type(&curr_flt_extn_iter) != DBUS_TYPE_UINT32) {
                         // D-Bus response URI is not a string
                         return SUCCESS;
                    }
                    dbus_uint32_t type;
                    dbus_message_iter_get_basic(&curr_flt_extn_iter, &type);
                    if (type != 0) {
                         // Wrong filter type
                         return SUCCESS;
                    }
                    if (!dbus_message_iter_next(&curr_flt_extn_iter)) {
                         // D-Bus response current_filter struct ended prematurely
                         return SUCCESS;
                    }
                    if (dbus_message_iter_get_arg_type(&curr_flt_extn_iter) != DBUS_TYPE_STRING) {
                         // D-Bus response URI is not a string
                         return SUCCESS;
                    }
                    dbus_message_iter_get_basic(&curr_flt_extn_iter, &tmp_extn);
                    return SUCCESS;
                }) == ERROR)
        return ERROR;

    if (tmp_extn) {
        Free((void*)selFilter->pattern);
        selFilter->pattern = strdup(tmp_extn);
    }
    if (tmp_curr_filter) {
        Free((void*)selFilter->name);
        selFilter->name = strdup(tmp_curr_filter);
    }
    return SUCCESS;
}

Result readResponseUrisSingle(DBusMessage* msg, const char* &file) {
    DBusMessageIter uri_iter;
    const Result res = readResponseUris(msg, uri_iter);
    if (res != SUCCESS)
        return res;
    if (dbus_message_iter_get_arg_type(&uri_iter) != DBUS_TYPE_STRING) {
        setErrorText("D-Bus response URI is not a string");
        return ERROR;
    }
    dbus_message_iter_get_basic(&uri_iter, &file);
    return SUCCESS;
}

#ifdef ADD_EXTENSION
Result readResponseUrisSingleAndCurrentExtension(DBusMessage* msg, const char* &file, const char* &extn, FilterItem* selFilter) {
    DBusMessageIter iter;
    const Result res = readResponseResults(msg, iter);
    if (res != SUCCESS)
        return res;
    const char* tmp_file = nullptr;
    const char* tmp_extn = nullptr;
    const char* tmp_curr_filter = nullptr;
    if (readDict(iter, "uris",
            [&tmp_file](DBusMessageIter& uris_iter) {
                if (dbus_message_iter_get_arg_type(&uris_iter) != DBUS_TYPE_ARRAY) {
                     setErrorText("D-Bus response URI is not an array");
                     return ERROR;
                }
                DBusMessageIter uri_iter;
                dbus_message_iter_recurse(&uris_iter, &uri_iter);
                if (dbus_message_iter_get_arg_type(&uri_iter) != DBUS_TYPE_STRING) {
                     setErrorText("D-Bus response URI is not a string");
                     return ERROR;
                }
                dbus_message_iter_get_basic(&uri_iter, &tmp_file);
                return SUCCESS;
            },
            "current_filter",
            [&tmp_extn, &tmp_curr_filter](DBusMessageIter& curr_flt_iter) {
                if (dbus_message_iter_get_arg_type(&curr_flt_iter) != DBUS_TYPE_STRUCT) {
                     // D-Bus response current_filter is not a struct
                     return SUCCESS;
                }
                DBusMessageIter curr_flt_struct_iter;
                dbus_message_iter_recurse(&curr_flt_iter, &curr_flt_struct_iter);
                if (dbus_message_iter_get_arg_type(&curr_flt_struct_iter) == DBUS_TYPE_STRING) {
                     // Get current filter
                     dbus_message_iter_get_basic(&curr_flt_struct_iter, &tmp_curr_filter);
                }
                if (!dbus_message_iter_next(&curr_flt_struct_iter)) {
                     // D-Bus response current_filter struct ended prematurely
                     return SUCCESS;
                }
                if (dbus_message_iter_get_arg_type(&curr_flt_struct_iter) != DBUS_TYPE_ARRAY) {
                     // D-Bus response URI is not a string
                     return SUCCESS;
                }
                DBusMessageIter curr_flt_arr_iter;
                dbus_message_iter_recurse(&curr_flt_struct_iter, &curr_flt_arr_iter);
                if (dbus_message_iter_get_arg_type(&curr_flt_arr_iter) != DBUS_TYPE_STRUCT) {
                     // D-Bus response current_filter is not a struct
                     return SUCCESS;
                }
                DBusMessageIter curr_flt_extn_iter;
                dbus_message_iter_recurse(&curr_flt_arr_iter, &curr_flt_extn_iter);
                if (dbus_message_iter_get_arg_type(&curr_flt_extn_iter) != DBUS_TYPE_UINT32) {
                     // D-Bus response URI is not a string
                     return SUCCESS;
                }
                dbus_uint32_t type;
                dbus_message_iter_get_basic(&curr_flt_extn_iter, &type);
                if (type != 0) {
                     // Wrong filter type
                     return SUCCESS;
                }
                if (!dbus_message_iter_next(&curr_flt_extn_iter)) {
                     // D-Bus response current_filter struct ended prematurely
                     return SUCCESS;
                }
                if (dbus_message_iter_get_arg_type(&curr_flt_extn_iter) != DBUS_TYPE_STRING) {
                     // D-Bus response URI is not a string
                     return SUCCESS;
                }
                dbus_message_iter_get_basic(&curr_flt_extn_iter, &tmp_extn);
                return SUCCESS;
            }) == ERROR)
        return ERROR;

    if (!tmp_file) {
        setErrorText("D-Bus response has no URI field");
        return ERROR;
    }
    file = tmp_file;
    extn = tmp_extn;
    if (tmp_curr_filter) {
        Free((void*)selFilter->pattern);
        Free((void*)selFilter->name);
        selFilter->name = strdup(tmp_curr_filter);
        selFilter->pattern = strdup(extn);
    }
    return SUCCESS;
}
#endif

char* generateChars(char* out) {
    size_t count = 32;
    while (count > 0) {
        unsigned char buff[32];
        //ssize_t rnd = getrandom(buff, count, 0);
        ssize_t rnd = syscall(SYS_getrandom, buff, count, 0);
        if (rnd == -1) {
            if (errno == EINTR)
                continue;
            else
                break;
        }
        count -= rnd;
        // must be [A-Z][a-z][0-9]_
        for (size_t i = 0; i != static_cast<size_t>(rnd); ++i) {
            *out++ = 'A' + static_cast<char>(buff[i] & 15);
            *out++ = 'A' + static_cast<char>(buff[i] >> 4);
        }
    }
    return out;
}

char* createUniquePath(const char** handle_token) {
    const char RESPONSE_PATH[] = "/org/freedesktop/portal/desktop/request/";
    constexpr size_t RESPONSE_PATH_SIZE = sizeof(RESPONSE_PATH) - 1;
    const char* dbus_name = dbus_unique_name;
    if (*dbus_name == ':')
        ++dbus_name;
    const size_t sender_len = strlen(dbus_name);
    const size_t size = RESPONSE_PATH_SIZE + sender_len + 1 + 64;  // 1 for '/'
    char* path = (char*)malloc(size + 1);
    char* path_ptr = path;
    path_ptr = strcopy(RESPONSE_PATH, RESPONSE_PATH + RESPONSE_PATH_SIZE, path_ptr);
    path_ptr = replaceSymbol(dbus_name, dbus_name + sender_len, path_ptr, [](char chr) {
        return (chr != '.') ? chr : '_';
    });
    *path_ptr++ = '/';
    *handle_token = path_ptr;
    path_ptr = generateChars(path_ptr);
    *path_ptr = '\0';
    return path;
}

bool isHex(char ch) {
    return ('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F') || ('a' <= ch && ch <= 'f');
}

bool tryUriDecodeLen(const char* fileUri, size_t &out, const char* &fileUriEnd) {
    size_t len = 0;
    while (*fileUri) {
        if (*fileUri != '%') {
            ++fileUri;
        } else {
            if (*(fileUri + 1) == '\0' || *(fileUri + 2) == '\0') {
                return false;
            }
            if (!isHex(*(fileUri + 1)) || !isHex(*(fileUri + 2))) {
                return false;
            }
            fileUri += 3;
        }
        ++len;
    }
    out = len;
    fileUriEnd = fileUri;
    return true;
}

char parseHex(char chr) {
    if ('0' <= chr && chr <= '9')
        return chr - '0';
    if ('A' <= chr && chr <= 'F')
        return chr - ('A' - 10);
    if ('a' <= chr && chr <= 'f')
        return chr - ('a' - 10);
#if defined(__GNUC__)
    __builtin_unreachable();
#endif
}

char* uriDecodeUnchecked(const char* fileUri, const char* fileUriEnd, char* outPath) {
    while (fileUri != fileUriEnd) {
        if (*fileUri != '%') {
            *outPath++ = *fileUri++;
        } else {
            ++fileUri;
            const char high_nibble = parseHex(*fileUri++);
            const char low_nibble = parseHex(*fileUri++);
            *outPath++ = (high_nibble << 4) | low_nibble;
        }
    }
    return outPath;
}

Result allocAndCopyFilePath(const char* fileUri, char* &outPath) {
    const char* prefix_begin = URI_PREFIX;
    const char* const prefix_end = URI_PREFIX + URI_PREFIX_SIZE;
    for (; prefix_begin != prefix_end; ++prefix_begin, ++fileUri) {
        if (*prefix_begin != *fileUri) {
            setErrorText("Portal returned not a file URI");
            return ERROR;
        }
    }
    size_t decoded_len;
    const char* file_uri_end;
    if (!tryUriDecodeLen(fileUri, decoded_len, file_uri_end)) {
        setErrorText("Portal returned a malformed URI");
        return ERROR;
    }
    char* const path_without_prefix = (char*)malloc(decoded_len + 1);
    char* const out_end = uriDecodeUnchecked(fileUri, file_uri_end, path_without_prefix);
    *out_end = '\0';
    outPath = path_without_prefix;
    return SUCCESS;
}

#ifdef ADD_EXTENSION
bool tryGetExtension(const char* extn, const char* &trimmed_extn, const char* &trimmed_extn_end) {
    if (!extn)
        return false;
    if (*extn != '*')
        return false;
    ++extn;
    if (*extn != '.')
        return false;
    trimmed_extn = extn;
    for (++extn; *extn != '\0'; ++extn)
        ;
    ++extn;
    trimmed_extn_end = extn;
    return true;
}

Result allocAndCopyFilePathWithExtn(const char* fileUri, const char* extn, char* &outPath) {
    const char* prefix_begin = URI_PREFIX;
    const char* const prefix_end = URI_PREFIX + URI_PREFIX_SIZE;
    for (; prefix_begin != prefix_end; ++prefix_begin, ++fileUri) {
        if (*prefix_begin != *fileUri) {
            setErrorText("D-Bus portal returned a not file URI");
            return ERROR;
        }
    }

    size_t decoded_len;
    const char* file_uri_end;
    if (!tryUriDecodeLen(fileUri, decoded_len, file_uri_end)) {
        setErrorText("D-Bus portal returned a malformed URI");
        return ERROR;
    }
    const char* file_it = file_uri_end;

    do {
        --file_it;
    } while (*file_it != '/' && *file_it != '.');
    const char* trimmed_extn;      // includes the '.'
    const char* trimmed_extn_end;  // includes the '\0'
    if (*file_it == '.' || !tryGetExtension(extn, trimmed_extn, trimmed_extn_end)) {
        // has file extension or no valid extension
        char* const path_without_prefix = (char*)malloc(decoded_len + 1);
        char* const out_end = uriDecodeUnchecked(fileUri, file_uri_end, path_without_prefix);
        *out_end = '\0';
        outPath = path_without_prefix;
    } else {
        // no file extension
        char* const path_without_prefix = (char*)malloc(decoded_len + (trimmed_extn_end - trimmed_extn));
        char* const out_mid = uriDecodeUnchecked(fileUri, file_uri_end, path_without_prefix);
        char* const out_end = strcopy(trimmed_extn, trimmed_extn_end, out_mid);
        *out_end = '\0';
        outPath = path_without_prefix;
    }
    return SUCCESS;
}
#endif

Result callXdgPortal(Window parent, Xdg::Mode mode, const char* title,
                     DBusMessage* &outMsg,
                     const FilterItem* filterList,
                     uint filterCount,
                     FilterItem* selFilter,
                     const char* defltPath,
                     const char* defltName,
                     bool multiple) {
    const char* handle_token;
    char* handle_path = createUniquePath(&handle_token);
    FreeLater __freeLater(handle_path);
    DBusError err;
    dbus_error_init(&err);

    DBusSignalHandler signal_hand;
    Result res = signal_hand.subscribe(handle_path);
    if (res != SUCCESS)
        return res;

    DBusMessage* methd = dbus_message_new_method_call("org.freedesktop.portal.Desktop",
                                                      "/org/freedesktop/portal/desktop",
                                                      "org.freedesktop.portal.FileChooser",
                                                      (mode == Xdg::Mode::SAVE) ? "SaveFile" : "OpenFile");
    UnrefLater_DBusMessage __unrefLater(methd);
    DBusMessageIter iter;
    dbus_message_iter_init_append(methd, &iter);

    QString parent_window_qstr = "x11:" + QString::number((long)parent, 16);
    char* parent_window = parent_window_qstr.toUtf8().data();
    __dbusAppend(&iter, DBUS_TYPE_STRING, &parent_window);
    __dbusAppend(&iter, DBUS_TYPE_STRING, &title);

    DBusMessageIter arr_iter;
    __dbusOpen(&iter, DBUS_TYPE_ARRAY, "{sv}", &arr_iter);
    setHandleToken(arr_iter, handle_token);

    if (mode == Xdg::Mode::SAVE) {
        // Save file
        setFilters(arr_iter, filterList, filterCount, selFilter);
        setCurrentName(arr_iter, defltName);
        setCurrentFolder(arr_iter, defltPath);
        setCurrentFile(arr_iter, defltPath, defltName);
    } else
    if (mode == Xdg::Mode::OPEN) {
        // Open file(s)
        if (multiple)
            setOpenFileEntryType(arr_iter, EntryType::Multiple);

        setFilters(arr_iter, filterList, filterCount, selFilter);
    } else {
        // Open folder
        setCurrentFolder(arr_iter, defltPath);
        setOpenFileEntryType(arr_iter, EntryType::Directory);
    }
    __dbusClose(&iter, &arr_iter);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(
                            dbus_conn, methd, DBUS_TIMEOUT_INFINITE, &err);
    if (!reply) {
        dbus_error_free(&dbus_err);
        dbus_move_error(&err, &dbus_err);
        setErrorText(dbus_err.message);
        return ERROR;
    }
    UnrefLater_DBusMessage __replyUnrefLater(reply);
    {
        DBusMessageIter iter;
        if (!dbus_message_iter_init(reply, &iter)) {
            setErrorText("D-Bus reply is missing an argument");
            return ERROR;
        }
        if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_OBJECT_PATH) {
            setErrorText("D-Bus reply is not an object path");
            return ERROR;
        }
        const char* path;
        dbus_message_iter_get_basic(&iter, &path);
        if (strcmp(path, handle_path) != 0) {
            signal_hand.subscribe(path);
        }
    }

    do {
        while (true) {
            DBusMessage* msg = dbus_connection_pop_message(dbus_conn);
            if (!msg)
                break;
            if (dbus_message_is_signal(msg, "org.freedesktop.portal.Request", "Response")) {
                outMsg = msg;
                return SUCCESS;
            }
            dbus_message_unref(msg);
        }
    } while (dbus_connection_read_write(dbus_conn, -1));

    setErrorText("Portal did not give a reply");
    return ERROR;
}

const char* getErrorText(void) {
    return error_code;
}

void clearDBusError(void) {
    setErrorText(nullptr);
    dbus_error_free(&dbus_err);
}

Result initDBus(void) {
    dbus_error_init(&dbus_err);
    dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &dbus_err);
    if (!dbus_conn) {
        setErrorText(dbus_err.message);
        return ERROR;
    }
    dbus_unique_name = dbus_bus_get_unique_name(dbus_conn);
    if (!dbus_unique_name) {
        setErrorText("Cannot get name of connection");
        return ERROR;
    }
    return SUCCESS;
}

void quitDBus(void) {
    dbus_connection_unref(dbus_conn);
}

void freePath(char* filePath) {
    assert(filePath);
    Free(filePath);
}

Result openDialog(Window parent, Xdg::Mode mode, const char* title,
                  char** outPaths,
                  const FilterItem* filterList,
                  uint filterCount,
                  FilterItem* selFilter,
                  const char* defltPath,
                  const char* defltName,
                  bool multiple) {
    DBusMessage* msg;
    {
        const Result res = callXdgPortal(parent, mode, title,
                                         msg,
                                         filterList,
                                         filterCount,
                                         selFilter,
                                         defltPath,
                                         defltName,
                                         multiple);
        if (res != SUCCESS)
            return res;
    }

    if (mode == Xdg::Mode::OPEN) {
        if (!multiple) {
            // Open file
            UnrefLater_DBusMessage __msgUnrefLater(msg);
            const char* uri;
            {
                const Result res = readResponseUrisSingle(msg, uri);
                if (res != SUCCESS)
                    return res;
                readResponseCurrentFilter(msg, selFilter);
            }
            return allocAndCopyFilePath(uri, *outPaths);
        } else {
            // Open files
            DBusMessageIter uri_iter;
            const Result res = readResponseUris(msg, uri_iter);
            if (res != SUCCESS) {
                dbus_message_unref(msg);
                return res;
            }
            *outPaths = (char*)msg;
            return SUCCESS;
        }

    } else
    if (mode == Xdg::Mode::SAVE) {
        // Save file
        UnrefLater_DBusMessage __msgUnrefLater(msg);
#ifdef ADD_EXTENSION
        const char* uri = NULL;
        const char* extn = NULL;
        {
            const Result res = readResponseUrisSingleAndCurrentExtension(msg, uri, extn, selFilter);
            if (res != SUCCESS) {
                return res;
            }
        }
        return allocAndCopyFilePathWithExtn(uri, extn, *outPaths);
#else
        const char* uri;
        {
            const Result res = readResponseUrisSingle(msg, uri);
            if (res != SUCCESS)
                return res;
            readResponseCurrentFilter(msg, selFilter);
        }
        return allocAndCopyFilePath(uri, *outPaths);
#endif
    } else {
        // Open folder
        UnrefLater_DBusMessage __msgUnrefLater(msg);
        const char* uri;
        {
            const Result res = readResponseUrisSingle(msg, uri);
            if (res != SUCCESS)
                return res;
        }
        return allocAndCopyFilePath(uri, *outPaths);
    }
}

Result pathSetGetCount(const void* pathSet, uint* count) {
    assert(pathSet);
    DBusMessage* msg = const_cast<DBusMessage*>(static_cast<const DBusMessage*>(pathSet));
    *count = readResponseUrisUncheckedSize(msg);
    return SUCCESS;
}

Result pathSetGetPath(const void* pathSet, uint index, char** outPath) {
    assert(pathSet);
    DBusMessage* msg = const_cast<DBusMessage*>(static_cast<const DBusMessage*>(pathSet));
    DBusMessageIter uri_iter;
    readResponseUrisUnchecked(msg, uri_iter);
    while (index > 0) {
        --index;
        if (!dbus_message_iter_next(&uri_iter)) {
            setErrorText("Index out of bounds");
            return ERROR;
        }
    }
    if (dbus_message_iter_get_arg_type(&uri_iter) != DBUS_TYPE_STRING) {
        setErrorText("D-Bus response URI is not a string");
        return ERROR;
    }
    const char* uri;
    dbus_message_iter_get_basic(&uri_iter, &uri);
    return allocAndCopyFilePath(uri, *outPath);
}

void pathSetFreePath(const char* filePath) {
    assert(filePath);
    freePath(const_cast<char*>(filePath));
}

void pathSetFree(const void* pathSet) {
    assert(pathSet);
    DBusMessage* msg = const_cast<DBusMessage*>(static_cast<const DBusMessage*>(pathSet));
    dbus_message_unref(msg);
}

char* strcopy(const char* start, const char* end, char* out) {
    for (; start != end; ++start) {
        *out++ = *start;
    }
    return out;
}

void setErrorText(const char* msg) {
    error_code = msg;
}

void Free(void* p) {
    if (p != NULL) {
        free(p);
        p = NULL;
    }
}

void onWindowFound(xcb_window_t w, void *user_data)
{
    if (QWidget *p = (QWidget*)user_data)
        XcbUtils::moveWindow(w, p->x() + 20, p->y() + 80);
    XcbUtils::setNativeFocusTo(w);
}

void parseFilterString(Xdg::Mode mode, const QString &filter, FilterItem &filterItem) {
    int pos = filter.indexOf('(');
    QString flt_name = (mode == Xdg::Mode::OPEN && filter.length() > 255 && pos > 1) ? filter.mid(0, pos - 1) : filter;
    filterItem.name = strdup(flt_name.toUtf8().data());
    auto parse = filter.split('(');
    if (parse.size() == 1) {
        filterItem.pattern = strdup("");
    } else
    if (parse.size() == 2) {
        const QString pattern = parse[1].replace(")", "");
        filterItem.pattern = strdup(pattern.toUtf8().data());
    }
}

QStringList Xdg::openXdgPortal(QWidget *parent,
                               Mode mode,
                               const QString &title,
                               const QString &file_name,
                               const QString &path,
                               QString filter,
                               QString *sel_filter,
                               bool sel_multiple)
{
    initDBus();
    Window parentWid = (parent) ? (Window)parent->winId() : 0L;
    const int pos = file_name.lastIndexOf('/');
    const QString _file_name = (pos != -1) ? file_name.mid(pos + 1) : file_name;
    const QString _path = (path.isEmpty() && pos != -1) ? file_name.mid(0, pos) : path;

    int filterSize = 0;
    FilterItem *filterItem = nullptr;
    FilterItem selFilterItem = {nullptr, nullptr};
    if (!filter.isEmpty()) {
        filter.replace("/", " \u2044 ");
        QStringList filterList = filter.split(";;");
        filterSize = filterList.size();
        filterItem = new FilterItem[filterSize];
        int index = 0;
        foreach (const QString &flt, filterList) {
            parseFilterString(mode, flt, filterItem[index]);
            index++;
        }

        if (mode != Mode::FOLDER && sel_filter) {
            if (sel_filter->isEmpty() && !filterList.isEmpty())
                *sel_filter = filterList[0];

            sel_filter->replace("/", " \u2044 ");
            parseFilterString(mode, *sel_filter, selFilterItem);
        }
    }

    char* outPaths;
    XcbUtils::findWindowAsync("xdg-desktop-portal", (void*)parent, 3000, onWindowFound);
    Result result;
    result = openDialog(parentWid, mode, title.toUtf8().data(),
                        &outPaths,
                        filterItem,
                        filterSize,
                        &selFilterItem,
                        _path.toLocal8Bit().data(),
                        _file_name.toLocal8Bit().data(),
                        sel_multiple);

    QStringList files;
    if (result == Result::SUCCESS) {
        if (mode == Mode::OPEN && sel_multiple) {
            uint numPaths = 0;
            pathSetGetCount(outPaths, &numPaths);
            for (uint i = 0; i < numPaths; ++i) {
                char* path = nullptr;
                pathSetGetPath(outPaths, i, &path);
                files.append(QString::fromUtf8(path));
                pathSetFreePath(path);
            }
            pathSetFree(outPaths);
        } else {
            files.append(QString::fromUtf8(outPaths));
            freePath(outPaths);
        }
    } else
    if (result == Result::ERROR)
        CMessage::error(parent, QObject::tr("An error occurred while opening the portal:<br>%1").arg(QString::fromUtf8(getErrorText())));

    quitDBus();

    Free((void*)selFilterItem.pattern);
    if (selFilterItem.name != NULL) {
        if (sel_filter) {
            *sel_filter = QString::fromUtf8(selFilterItem.name);
            sel_filter->replace(" \u2044 ", "/");
        }
        Free((void*)selFilterItem.name);
    }

    if (filterItem) {
        for (int i = 0; i < filterSize; i++) {
            Free((void*)filterItem[i].pattern);
            Free((void*)filterItem[i].name);
        }
        delete[] filterItem;
    }

    return files;
}
