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

#include <QDir>
#include <QSettings>
#ifndef __OS_WIN_XP
# include <Shobjidl.h>
#endif
#include "filechooser.h"
#include "utils.h"

#define IsOpeningMode(mode) (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER)

using std::wstring;
#ifndef __OS_WIN_XP
using std::vector;
using std::pair;
typedef vector<IShellItem *> VectorShellItems;
typedef vector<pair<wstring, wstring>> specvector;

auto itemsFromItemArray(IShellItemArray * items)
{
    VectorShellItems out;
    DWORD itemCount = 0;
    if (FAILED(items->GetCount(&itemCount)) || itemCount == 0)
        return out;

    out.reserve(itemCount);
    for (DWORD i = 0; i < itemCount; ++i) {
        IShellItem * item = nullptr;
        if (SUCCEEDED(items->GetItemAt(i, &item)))
            out.push_back(item);
    }

    return out;
}

auto stringToFilters(const wstring& wstr) -> specvector {
    specvector v;

    if ( !wstr.empty() ) {
        auto _parce_filter_string = [](const wstring& fullstr, size_t start, size_t stop) -> pair<wstring, wstring> {
            wstring filter_name = fullstr.substr(start, stop - start);
            size_t mid = filter_name.find(L"(") + 1;
            wstring filter_pattern = filter_name.substr(mid, filter_name.find(L")", mid) - mid);

            std::replace(begin(filter_pattern), end(filter_pattern), ' ', ';');
            if (filter_name.length() > 255 && mid > 2)
                filter_name = filter_name.substr(0, mid - 2) + L" (*.*)";
            return make_pair(filter_name, filter_pattern);
        };

        wstring _delim = L";;";
        size_t _curr = wstr.find(_delim),
            _prev = 0;
        while ( _curr != wstring::npos ) {
            v.push_back(_parce_filter_string(wstr,_prev,_curr));

            _prev = _curr + 2;
            _curr = wstr.find(_delim, _prev);
        }

        v.push_back(_parce_filter_string(wstr, _prev, wstr.size()));
    }

    return v;
}
#endif

QStringList Win::openWinFileChooser(QWidget *parent, Mode mode, const QString &title, const QString &file, const QString &path,
                                        const QString &filter, QString *sel_filter, bool sel_multiple)
{
    const int pos = file.lastIndexOf('/');
    const wstring _file = (pos != -1) ? file.mid(pos + 1).toStdWString() : file.toStdWString();
//    _file = _file.left(_file.lastIndexOf("."));
    const wstring _path = (path.isEmpty() && pos != -1) ? QDir::toNativeSeparators(file.mid(0, pos)).toStdWString() :
                              QDir::toNativeSeparators(path).toStdWString();

    QStringList filenames;
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : nullptr;
    WindowHelper::toggleLayoutDirection(parent_hwnd);
#ifndef __OS_WIN_XP
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileDialog *pDialog;
        hr = CoCreateInstance(IsOpeningMode(mode) ? CLSID_FileOpenDialog : CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER,
                              IsOpeningMode(mode) ? IID_IFileOpenDialog : IID_IFileSaveDialog, (void**)&pDialog);
        if (SUCCEEDED(hr)) {
            pDialog->SetTitle(title.toStdWString().c_str());
            FILEOPENDIALOGOPTIONS dwFlags = FOS_PATHMUSTEXIST;
            hr = pDialog->GetOptions(&dwFlags);
            if (SUCCEEDED(hr)) {
                if (sel_multiple && mode != Win::Mode::SAVE)
                    dwFlags |= FOS_ALLOWMULTISELECT;

                QSettings _r("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", QSettings::NativeFormat);
                if ( _r.value("Hidden", 1) == 1 )
                    dwFlags |= FOS_FORCESHOWHIDDEN;

                if (mode == Win::Mode::FOLDER)
                    dwFlags |= FOS_PICKFOLDERS;
                else
                if (mode == Win::Mode::OPEN)
                    dwFlags |= FOS_FILEMUSTEXIST;
                else
                if (mode == Win::Mode::SAVE)
                    dwFlags &= ~FOS_OVERWRITEPROMPT;

                pDialog->SetOptions(dwFlags);
            }

            specvector filters{stringToFilters(filter.toStdWString())};
            uint typeIndex = 1;
            COMDLG_FILTERSPEC * specOpenTypes = new COMDLG_FILTERSPEC[filters.size()];
            for (uint i{0}; i < filters.size(); ++i) {
                specvector::const_reference iter = filters.at(i);
                specOpenTypes[i] = {iter.first.c_str(), iter.second.c_str()};
                if (sel_filter) {
                    if ( !sel_filter->isEmpty() && iter.first.find(sel_filter->toStdWString()) == 0 )
                        typeIndex = i + 1;
                }
            }

            pDialog->SetFileTypes(filters.size(), specOpenTypes);
            delete [] specOpenTypes;
            pDialog->SetFileTypeIndex(typeIndex);

            if (!_path.empty()) {
                IShellItem * pItem = nullptr;
                hr = SHCreateItemFromParsingName(_path.c_str(), nullptr, IID_PPV_ARGS(&pItem));
                if (SUCCEEDED(hr)) {
                    pDialog->SetFolder(pItem);
                    pItem->Release();
                }
            }

            if (!_file.empty() && mode == Win::Mode::SAVE) {
                pDialog->SetFileName(_file.c_str());
                pDialog->SetDefaultExtension(L"");
            }

            hr = pDialog->Show(parent_hwnd);
            if (SUCCEEDED(hr)) {
                if (IsOpeningMode(mode)) {
                    IShellItemArray * items = nullptr;
                    hr = ((IFileOpenDialog*)pDialog)->GetResults(&items);
                    if (SUCCEEDED(hr) && items) {
                        VectorShellItems iarray = itemsFromItemArray(items);
                        for (IShellItem * item : iarray) {
                            PWSTR pszFilePath;
                            hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            if (SUCCEEDED(hr)) {
                                filenames.append(QString::fromStdWString(pszFilePath));
                                CoTaskMemFree(pszFilePath);
                            }
                        }

                        for (IShellItem * item : iarray)
                            item->Release();
                        items->Release();
                    }
                } else {
                    IShellItem * item;
                    hr = pDialog->GetResult(&item);
                    if (SUCCEEDED(hr) && item) {
                        PWSTR pszFilePath;
                        hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        if (SUCCEEDED(hr)) {
                            filenames.append(QString::fromStdWString(pszFilePath).replace('\\', '/'));
                            CoTaskMemFree(pszFilePath);
                        }
                        item->Release();
                    }
                }
                if (mode != Win::Mode::FOLDER) {
                    uint typeIndex;
                    pDialog->GetFileTypeIndex(&typeIndex);
                    if (sel_filter && typeIndex > 0 && typeIndex <= filters.size()) {
                        specvector::const_reference iter = filters.at(typeIndex - 1);
                        *sel_filter = QString::fromStdWString(iter.first);
                    }
                }
            }
            pDialog->Release();
        }
        CoUninitialize();
    }
#else
#endif
    WindowHelper::toggleLayoutDirection(parent_hwnd);
    return filenames;
}
