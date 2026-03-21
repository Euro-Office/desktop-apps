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

#define NTDDI_VERSION NTDDI_WIN7
#define WIN32_LEAN_AND_MEAN
#define STRICT_TYPED_ITEMIDS
#define MAX_TASK_NUM 4
#define ICON_OFFSET 14

#include "jumplist.h"
#include "resource.h"
#include <QtGlobal>
#include <Windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <strsafe.h>

#include <objectarray.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shlobj.h>


HRESULT _CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl, int index)
{
    WCHAR szAppPath[MAX_PATH];
    if (GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath)) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    IShellLink *psl;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
            hr = psl->SetPath(szAppPath);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetArguments(pszArguments);
                if (SUCCEEDED(hr))
                {
                    hr = psl->SetIconLocation(szAppPath, index + ICON_OFFSET);
                    Q_UNUSED(hr);
                    IPropertyStore *pps;
                    hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                    if (SUCCEEDED(hr))
                    {
                        PROPVARIANT propvar;
                        hr = InitPropVariantFromString(pszTitle, &propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->SetValue(PKEY_Title, propvar);
                            if (SUCCEEDED(hr))
                            {
                                hr = pps->Commit();
                                if (SUCCEEDED(hr))
                                {
                                    hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                                }
                            }
                            PropVariantClear(&propvar);
                        }
                        pps->Release();
                    }
                }
            }
        psl->Release();
    }
    return hr;
}

HRESULT _AddTasksToList(ICustomDestinationList *pcdl)
{
    WCHAR szAppPath[MAX_PATH];
    if (GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath)) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr))
    {
        struct {
            PCWSTR arg;
            int stringId;
        }
        tasks[MAX_TASK_NUM] = {
            {L"--new:word",  IDS_JUMP_DOCX},
            {L"--new:cell",  IDS_JUMP_XLSX},
            {L"--new:slide", IDS_JUMP_PPTX},
            {L"--new:form",  IDS_NEW_PDF},
        };

        IShellLink * psl;
        for (int i = 0; i < MAX_TASK_NUM; i++) {
            WCHAR szResPath[MAX_PATH];
            swprintf_s(szResPath, ARRAYSIZE(szResPath), L"@%s,-%d", szAppPath, tasks[i].stringId);

            hr = _CreateShellLink(tasks[i].arg, szResPath, &psl, i);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            IObjectArray * poa;
            hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
            if (SUCCEEDED(hr))
            {
                hr = pcdl->AddUserTasks(poa);
                poa->Release();
            }
        }
        poc->Release();
    }
    return hr;
}

void CreateJumpList()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        ICustomDestinationList *pcdl;
        hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
        if (SUCCEEDED(hr)) {
            pcdl->SetAppID(TEXT(APP_USER_MODEL_ID));
            UINT cMinSlots;
            IObjectArray *poaRemoved;
            hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
            if (SUCCEEDED(hr)) {
                hr = _AddTasksToList(pcdl);
                if (SUCCEEDED(hr))
                    hr = pcdl->CommitList();
                poaRemoved->Release();
            }
            pcdl->Release();
        }
        CoUninitialize();
    }
    Q_UNUSED(hr);
}

void DeleteJumpList()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        ICustomDestinationList *pcdl;
        hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
        if (SUCCEEDED(hr)) {
            hr = pcdl->DeleteList(NULL);
            pcdl->Release();
        }
        CoUninitialize();
    }
    Q_UNUSED(hr);
}

void ClearHistory()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IApplicationDestinations *pad;
        hr = CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pad));
        if (SUCCEEDED(hr)) {
            hr = pad->RemoveAllDestinations();
            pad->Release();
        }
        CoUninitialize();
    }
    Q_UNUSED(hr);
}
