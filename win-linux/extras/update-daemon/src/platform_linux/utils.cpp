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

#include "platform_linux/utils.h"
#include "version.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <string.h>
#include <openssl/md5.h>
#include <vector>
#include <fcntl.h>
#include <fts.h>
#include <linux/limits.h>
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#define APP_CONFIG_PATH "/.config/" REG_GROUP_KEY "/" REG_APP_NAME ".conf"
#define BUFSIZE 1024


static void replace(string &str, const string &from, const string &to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

static bool copyFile(const string &oldFile, const string &newFile)
{
    struct stat st;
    if (lstat(oldFile.c_str(), &st) != 0)
        return false;

    char buf[BUFSIZ];
    int fd_src = -1, fd_dst = -1, n_read = 0;
    if ((fd_src = open(oldFile.c_str(), O_RDONLY)) < 0)
        return false;

    if ((fd_dst = creat(newFile.c_str(), 0666)) < 0) {
        close(fd_src);
        return false;
    }
    if (fchmod(fd_dst, st.st_mode) != 0) {
        close(fd_src);
        close(fd_dst);
        return false;
    }

    while ((n_read = read(fd_src, buf, sizeof(buf))) > 0) {
        if (write(fd_dst, buf, n_read) != n_read) {
            close(fd_src);
            close(fd_dst);
            return false;
        }
    }
    if (close(fd_src) != 0)
        return false;
    if (close(fd_dst) != 0)
        return false;

    return n_read == 0;
};

namespace NS_Utils
{
    std::vector<string> cmd_args;

    void parseCmdArgs(int argc, char *argv[])
    {
        for (int i = 0; i < argc; i++)
            cmd_args.push_back(argv[i]);
    }

    bool cmdArgContains(const string &param)
    {
        auto len = param.length();
        return std::any_of(cmd_args.cbegin(), cmd_args.cend(), [&param, len](const string &arg) {
            return arg.find(param) == 0 && (len == arg.length() || arg[len] == '=' || arg[len] == ':' || arg[len] == '|');
        });
    }

    string cmdArgValue(const string &param)
    {
        auto len = param.length();
        for (const auto &arg : cmd_args) {
            if (arg.find(param) == 0 && len < arg.length() && (arg[len] == '=' || arg[len] == ':' || arg[len] == '|'))
                return arg.substr(len + 1);
        }
        return "";
    }

    string GetLastErrorAsString()
    {
        char buff[LINE_MAX] = {0};
        char *res = strerror_r(errno, buff, sizeof(buff));
        return res ? string(res) : "";
    }

    int ShowMessage(string str, bool showError)
    {
        if (showError)
            str += " " + GetLastErrorAsString();

        gtk_init(NULL, NULL);
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", str.c_str());
        string prod_name = _TR(CAPTION_TEXT);
        gtk_window_set_title(GTK_WINDOW(dialog), prod_name.c_str());
        gtk_window_set_keep_above(GTK_WINDOW(dialog), true);
        gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 150);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), false);
        int res = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return res;
    }

//    string GetSysLanguage()
//    {
//        string lang("en_EN");
//        size_t pos = std::string::npos;
//        if (char *_lang = getenv("LANG")) {
//            lang = _lang;
//            pos = lang.find('.');
//        }
//        return (pos == std::string::npos) ? lang : lang.substr(0, pos);
//    }

    string GetAppLanguage()
    {
        string lang = "en_US";
        if (char *uname = getlogin()) {
            string value = "locale=", path = string("/home/") + uname + APP_CONFIG_PATH;
            list<string> lst;
            NS_File::readFile(path, lst);
            for (const auto &str : lst) {
                size_t pos = str.find(value);
                if (pos != string::npos) {
                    lang = str.substr(pos + value.length());
                    std::replace(lang.begin(), lang.end(), '-', '_');
                    return lang;
                }
            }
        }
        return lang;
    }
}

namespace NS_File
{
    bool GetFilesList(const string &path, list<string> *lst, string &error, bool ignore_locked, bool folders_only)
    {
        DIR *dir = opendir(path.c_str());
        if (!dir) {
            error = string("FindFirstFile invalid handle value: ") + path;
            return false;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char _path[PATH_MAX];
            snprintf(_path, sizeof(_path), "%s/%s", path.c_str(), entry->d_name);
            unsigned char d_type = entry->d_type;
            if (d_type == DT_UNKNOWN) {
                struct stat info;
                if (lstat(_path, &info) != 0) {
                    error = string("Error getting file information: ") + _path;
                    closedir(dir);
                    return false;
                }
                if (S_ISDIR(info.st_mode))
                    d_type = DT_DIR;
                else
                if (S_ISREG(info.st_mode))
                    d_type = DT_REG;
            }

            if (d_type == DT_DIR) {
                if (ignore_locked && access(_path, R_OK) != 0)
                    continue;
                if (folders_only) {
                    lst->push_back(string("/") + entry->d_name);
                    continue;
                }
                if (!GetFilesList(_path, lst, error, ignore_locked)) {
                    closedir(dir);
                    return false;
                }
            } else
            if (!folders_only && d_type == DT_REG)
                lst->push_back(_path);

        }
        closedir(dir);
        return true;
    }

    bool readFile(const string &filePath, list<string> &linesList)
    {
        std::ifstream file(filePath.c_str(), std::ios_base::in);
        if (!file.is_open()) {
            NS_Logger::WriteLog("An error occurred while opening: " + filePath);
            return false;
        }
        string line;
        while (std::getline(file, line))
            linesList.push_back(line);

        file.close();
        return true;
    }

    bool writeToFile(const string &filePath, list<string> &linesList)
    {
        std::ofstream file(filePath.c_str(), std::ios_base::out);
        if (!file.is_open()) {
            NS_Logger::WriteLog("An error occurred while writing: " + filePath);
            return false;
        }
        for (auto &line : linesList)
            file << line << std::endl;

        file.close();
        return true;
    }

    bool runProcess(const string &fileName, const string &args)
    {
        string src(fileName);
        replace(src, " ", "\\ ");
        string cmd = ("LD_LIBRARY_PATH=" + parentPath(src) + " " + src + " " + args + " &");
        return system(cmd.c_str()) == 0 ? true: false;
    }

    bool isProcessRunning(const string &fileName)
    {
        DIR *proc_dir = opendir("/proc");
        if (!proc_dir)
            return false;

        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if (entry->d_type != DT_DIR && entry->d_type != DT_UNKNOWN)
                continue;

            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", "/proc", entry->d_name);
            unsigned char d_type = entry->d_type;
            if (d_type == DT_UNKNOWN) {
                struct stat info;
                if (lstat(path, &info) != 0)
                    continue;
                if (S_ISDIR(info.st_mode))
                    d_type = DT_DIR;
            }

            if (d_type == DT_DIR && strtol(entry->d_name, NULL, 10) > 0) {
                if (strlcat(path, "/cmdline", sizeof(path)) >= sizeof(path))
                    continue;

                FILE* cmd_file_ptr = fopen(path, "r");
                if (!cmd_file_ptr)
                    continue;

                if (fgets(path, sizeof(path), cmd_file_ptr) == NULL) {
                    fclose(cmd_file_ptr);
                    continue;
                }
                fclose(cmd_file_ptr);

                if (strcmp(basename(path), fileName.c_str()) == 0) {
                    closedir(proc_dir);
                    return true;
                }
            }
        }

        closedir(proc_dir);
        return false;
    }

    bool fileExists(const string &filePath)
    {
        struct stat st;
        return lstat(filePath.c_str(), &st) == 0 && S_ISREG(st.st_mode);
    }

    bool dirExists(const string &dirName) {
        struct stat st;
        return lstat(dirName.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool dirIsEmpty(const string &dirName)
    {
        DIR *dir = opendir(dirName.c_str());
        if (!dir)
            return (errno == ENOENT);

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);
        return true;
    }

    bool makePath(const string &path, size_t root_offset) {
        size_t len = path.length();
        if (len == 0)
            return false;
        if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0 || errno == EEXIST)
            return true;
        if (len >= PATH_MAX || root_offset >= len)
            return false;
        char buf[PATH_MAX];
        strcpy(buf, path.c_str());
        if (buf[len - 1] == '/')
            buf[len - 1] = '\0';
        char *it = buf + root_offset;
        while (1) {
            while (*it != '\0' && *it != '/')
                it++;
            char tmp = *it;
            *it = '\0';
            if (mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
                *it = tmp;
                return false;
            }
            if (tmp == '\0')
                break;
            *it++ = tmp;
        }
        return true;
    }

    bool replaceFile(const string &oldFilePath, const string &newFilePath)
    {
        if (rename(oldFilePath.c_str(), newFilePath.c_str()) == 0)
            return true;
        if (errno == EXDEV) {
            errno = 0;
            return copyFile(oldFilePath, newFilePath) && unlink(oldFilePath.c_str()) == 0;
        }
        return false;
    }

    bool replaceFolder(const string &from, const string &to, bool remove_existing)
    {
        if (remove_existing && !dirIsEmpty(to) && !removeDirRecursively(to))
            return false;

        if (rename(from.c_str(), to.c_str()) == 0)
            return true;

        if (errno == EXDEV || errno == EEXIST || errno == ENOTEMPTY) {
            size_t srcLen = from.length(); // rename() will return a higher priority errno ENOENT
            size_t dstLen = to.length();   // if length == 0 or ENAMETOOLONG if lenght >= PATH_MAX

            bool can_use_rename = !(errno == EXDEV); // EXDEV has higher priority than EEXIST and ENOTEMPTY
            errno = 0;

            char dstPath[PATH_MAX];
            snprintf(dstPath, sizeof(dstPath), "%s", to.c_str());

            string src(from);
            char * const paths[] = {&src[0], NULL};
            FTS *fts = fts_open(paths, FTS_PHYSICAL | FTS_XDEV /*| FTS_NOSTAT*/, NULL);
            if (fts == NULL)
                return false;

            bool res = true;
            FTSENT *ent;
            while (res && (ent = fts_read(fts)) != NULL) {
                switch (ent->fts_info) {
                case FTS_DOT:       // "." or ".."
                    break;
                case FTS_D:         // preorder directory
                    if (strlcat(dstPath, ent->fts_path + srcLen, PATH_MAX) >= PATH_MAX) {
                        errno = ENAMETOOLONG;
                        res = false;
                        break;
                    }
                    if (can_use_rename && ent->fts_level != 0 && rename(ent->fts_path, dstPath) == 0) {
                        dstPath[dstLen] = '\0';
                        fts_set(fts, ent, FTS_SKIP);
                        // Ensure that we do not process FTS_DP
                        (void)(fts_read(fts));
                        break;
                    }

                    if (mkdir(dstPath, ent->fts_statp->st_mode) != 0 && errno != EEXIST)
                        res = false;
                    dstPath[dstLen] = '\0';
                    break;
                case FTS_DP:		// postorder directory
                    if (rmdir(ent->fts_path) != 0)
                        res = false;
                    break;
                case FTS_F:			// regular file
                case FTS_SL:		// symbolic link
                case FTS_SLNONE:	// symbolic link without target
                case FTS_DEFAULT:   // file type not described by any other value
                    if (strlcat(dstPath, ent->fts_path + srcLen, PATH_MAX) >= PATH_MAX) {
                        errno = ENAMETOOLONG;
                        res = false;
                        break;
                    }
                    if (can_use_rename) {
                        if (rename(ent->fts_path, dstPath) != 0)
                            res = false;
                    } else
                    if (!copyFile(ent->fts_path, dstPath) || unlink(ent->fts_path) != 0)
                        res = false;
                    dstPath[dstLen] = '\0';
                    break;
                case FTS_NSOK:      // stat(2) information was not requested
                case FTS_NS:		// stat(2) information was not available
                case FTS_DC:		// directory that causes cycles
                case FTS_DNR:		// unreadable directory
                case FTS_ERR:
                default:
                    res = false;
                    break;
                }
            }

            if (fts_close(fts) != 0)
                return false;

            return res;
        }

        return false;
    }

    bool removeFile(const string &filePath)
    {
        return unlink(filePath.c_str()) == 0;
    }

    bool removeDirRecursively(const string &dir)
    {
        string _dir = dir;
        char * const paths[] = {&_dir[0], NULL};
        FTS *fts = fts_open(paths, FTS_PHYSICAL | FTS_XDEV /*| FTS_NOSTAT*/, NULL);
        if (fts == NULL)
            return false;

        bool res = true;
        FTSENT *ent;
        while (res && (ent = fts_read(fts)) != NULL) {
            switch (ent->fts_info) {
            case FTS_DOT:       // "." or ".."
                break;
            case FTS_D:         // preorder directory
                break;
            case FTS_DP:		// postorder directory
                if (rmdir(ent->fts_path) != 0)
                    res = false;
                break;
            case FTS_F:			// regular file
            case FTS_SL:		// symbolic link
            case FTS_SLNONE:	// symbolic link without target
            case FTS_DEFAULT:   // file type not described by any other value
                if (unlink(ent->fts_path) != 0)
                    res = false;
                break;
            case FTS_NSOK:      // stat(2) information was not requested
            case FTS_NS:		// stat(2) information was not available
            case FTS_DC:		// directory that causes cycles
            case FTS_DNR:		// unreadable directory
            case FTS_ERR:
            default:
                res = false;
                break;
            }
        }

        if (fts_close(fts) != 0)
            return false;

        return res;
    }

    string parentPath(const string &path)
    {
        size_t len = path.length();
        if (len > 1) {
            const char *buf = path.c_str();
            const char *it = buf + len - 1;
            while (*it == '/') {
                if (it == buf)
                    return "";
                it--;
            }
            while (*it != '/') {
                if (it == buf)
                    return "";
                it--;
            }
            if (it == buf)
                return "/";
            return string(buf, it - buf);
        }
        return "";
    }

    string tempPath()
    {
        const char *path = getenv("TMP");
        if (!path)
            path = getenv("TEMP");
        if (!path)
            path = getenv("TMPDIR");
        if (!path)
            path = "/tmp";
        return string(path);
    }

    string appPath()
    {
        char path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        return (count > 0) ? parentPath(string(path, count)) : "";
    }

   string getFileHash(const string &fileName)
   {
       FILE *file = fopen(fileName.c_str(), "rb");
       if (!file)
           return "";

       int bytes;
       unsigned char data[BUFSIZ];
       unsigned char digest[MD5_DIGEST_LENGTH];
       MD5_CTX mdContext;
       MD5_Init(&mdContext);
       while ((bytes = fread(data, 1, sizeof(data), file)) != 0)
           MD5_Update(&mdContext, data, bytes);

       MD5_Final(digest, &mdContext);
       fclose(file);

       std::ostringstream oss;
       for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
           oss.fill('0');
           oss.width(2);
           oss << std::hex << static_cast<const int>(digest[i]);
       }
       return oss.str();
   }

//    bool verifyEmbeddedSignature(const string &fileName)
//    {
//        return false;
//    }
}

namespace NS_Logger
{
    bool allow_write_log = false;

    void AllowWriteLog()
    {
        allow_write_log = true;
    }

    void WriteLog(const string &log, bool showMessage)
    {
        if (allow_write_log) {
            string filpPath(NS_File::tempPath() + "/oo_service_log.txt");
            std::ofstream file(filpPath.c_str(), std::ios::app);
            if (!file.is_open()) {
                return;
            }
            file << log << string("\n") << std::endl;
            file.close();
        }
        if (showMessage)
            NS_Utils::ShowMessage(log);
    }
}
