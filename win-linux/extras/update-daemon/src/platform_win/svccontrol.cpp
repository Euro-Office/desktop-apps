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

#include "svccontrol.h"
#include "classes/translator.h"
#include "platform_win/utils.h"
#include <aclapi.h>
#include <tchar.h>
#include <sstream>

#define SVC_ERROR ((DWORD)0xC0020001L)

BOOL GetServiceHandle(SC_HANDLE &schSCManager, SC_HANDLE &schService, DWORD dwDesiredAccess)
{
    // Get a handle to the SCM database.
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!schSCManager) {
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        return FALSE;
    }

    // Get a handle to the service.
    schService = OpenService(schSCManager, SERVICE_NAME, dwDesiredAccess);
    if (!schService) {
        CloseServiceHandle(schSCManager);
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        return FALSE;
    }
    return TRUE;
}

BOOL __stdcall StopDependentServices(SC_HANDLE schSCManager, SC_HANDLE schService)
{
    DWORD i;
    DWORD dwBytesNeeded;
    DWORD dwCount;

    LPENUM_SERVICE_STATUS   lpDependencies = NULL;
    ENUM_SERVICE_STATUS     ess;
    SC_HANDLE               hDepService;
    SERVICE_STATUS_PROCESS  ssp;

    DWORD dwStartTime = GetTickCount();
    DWORD dwTimeout = 30000; // 30-second time-out

    // Pass a zero-length buffer to get the required buffer size.
    if (EnumDependentServices(schService, SERVICE_ACTIVE, lpDependencies, 0,
                                &dwBytesNeeded, &dwCount)) {
         // If the Enum call succeeds, then there are no dependent
         // services, so do nothing.
         return TRUE;
    } else {
        if (GetLastError() != ERROR_MORE_DATA)
            return FALSE; // Unexpected error

        // Allocate a buffer for the dependencies.
        lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);
        if (!lpDependencies)
            return FALSE;

        __try {
            // Enumerate the dependencies.
            if (!EnumDependentServices(schService, SERVICE_ACTIVE, lpDependencies, dwBytesNeeded,
                                          &dwBytesNeeded, &dwCount))
                return FALSE;

            for (i = 0; i < dwCount; i++) {
                ess = *(lpDependencies + i);
                // Open the service.
                hDepService = OpenService(schSCManager, ess.lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
                if (!hDepService)
                   return FALSE;

                __try {
                    // Send a stop code.
                    if (!ControlService(hDepService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp))
                        return FALSE;

                    // Wait for the service to stop.
                    while (ssp.dwCurrentState != SERVICE_STOPPED)
                    {
                        Sleep( ssp.dwWaitHint );
                        if (!QueryServiceStatusEx(hDepService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                                                    sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
                            return FALSE;

                        if (ssp.dwCurrentState == SERVICE_STOPPED)
                            break;

                        if (GetTickCount() - dwStartTime > dwTimeout)
                            return FALSE;
                    }
                }
                __finally
                {
                    // Always release the service handle.
                    CloseServiceHandle(hDepService);
                }
            }
        }
        __finally
        {
            // Always free the enumeration buffer.
            HeapFree(GetProcessHeap(), 0, lpDependencies);
        }
    }
    return TRUE;
}

namespace SvcControl
{
    VOID SvcInstall()
    {
        TCHAR szUnquotedPath[MAX_PATH];
        if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH)) {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            return;
        }

        // In case the path contains a space, it must be quoted so that
        // it is correctly interpreted. For example,
        // "d:\my share\myservice.exe" should be specified as
        // ""d:\my share\myservice.exe"".
        TCHAR szPath[MAX_PATH];
        StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);

        // Get a handle to the SCM database.
        SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (schSCManager == NULL) {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            return;
        }

        SC_HANDLE schService = CreateService(
            schSCManager,              // SCM database
            SERVICE_NAME,              // name of service
            SERVICE_NAME,              // service name to display
            SERVICE_ALL_ACCESS,        // desired access
            SERVICE_WIN32_OWN_PROCESS, // service type
            SERVICE_AUTO_START,        // start type // SERVICE_DEMAND_START
            SERVICE_ERROR_NORMAL,      // error control type
            szPath,                    // path to service's binary
            NULL,                      // no load ordering group
            NULL,                      // no tag identifier
            NULL,                      // no dependencies
            NULL,                      // LocalSystem account
            NULL);                     // no password

        if (schService == NULL) {
            CloseServiceHandle(schSCManager);
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            return;
        }
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    VOID __stdcall DoStartSvc()
    {
        SC_HANDLE schSCManager, schService;
        if (!GetServiceHandle(schSCManager, schService, SERVICE_ALL_ACCESS))
            return;

        // Check the status in case the service is not stopped.
        DWORD dwBytesNeeded;
        SERVICE_STATUS_PROCESS ssStatus;
        if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus,
                                    sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto cleanup;
        }

        // Check if the service is already running. It would be possible
        // to stop the service here, but for simplicity this example just returns.
        if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
        {
            NS_Logger::WriteLog(L"Cannot start the service because it is already running");
            goto cleanup;
        }

        // Save the tick count and initial checkpoint.
        DWORD dwOldCheckPoint;
        DWORD dwStartTickCount;
        dwStartTickCount = GetTickCount();
        dwOldCheckPoint = ssStatus.dwCheckPoint;

        // Wait for the service to stop before attempting to start it.
        DWORD dwWaitTime;
        while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
        {
            // Do not wait longer than the wait hint. A good interval is
            // one-tenth of the wait hint but not less than 1 second
            // and not more than 10 seconds.

            dwWaitTime = ssStatus.dwWaitHint / 10;
            if( dwWaitTime < 1000 )
                dwWaitTime = 1000;
            else
            if ( dwWaitTime > 10000 )
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

            // Check the status until the service is no longer stop pending.
            if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus,
                                        sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
            {
                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
                goto cleanup;
            }

            if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
                // Continue to wait and check.
                dwStartTickCount = GetTickCount();
                dwOldCheckPoint = ssStatus.dwCheckPoint;
            } else {
                if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                    printf("Timeout waiting for service to stop\n");
                    goto cleanup;
                }
            }
        }

        // Attempt to start the service.
        if (!StartService(schService, 0, NULL))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto cleanup;
        } else
            printf("Service start pending...\n");

        // Check the status until the service is no longer start pending.
        if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus,
                                    sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto cleanup;
        }

        // Save the tick count and initial checkpoint.
        dwStartTickCount = GetTickCount();
        dwOldCheckPoint = ssStatus.dwCheckPoint;

        while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
        {
            // Do not wait longer than the wait hint. A good interval is
            // one-tenth the wait hint, but no less than 1 second and no
            // more than 10 seconds.

            dwWaitTime = ssStatus.dwWaitHint / 10;
            if( dwWaitTime < 1000 )
                dwWaitTime = 1000;
            else
            if ( dwWaitTime > 10000 )
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

            // Check the status again.
            if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus,
                                        sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
            {
                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
                break;
            }

            if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
                // Continue to wait and check.
                dwStartTickCount = GetTickCount();
                dwOldCheckPoint = ssStatus.dwCheckPoint;
            } else {
                if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                    // No progress made within the wait hint.
                    break;
                }
            }
        }

        // Determine whether the service is running.
        if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
            NS_Logger::WriteLog(L"Service started successfully.");
        } else {
            NS_Logger::WriteLog(wstring(L"Service not started.") +
                        L"\nCurrent State: " + to_wstring(ssStatus.dwCurrentState) +
                        L"\nExit Code: " + to_wstring(ssStatus.dwWin32ExitCode) +
                        L"\nCheck Point: " + to_wstring(ssStatus.dwCheckPoint) +
                        L"\nWait Hint: " + to_wstring(ssStatus.dwWaitHint));
        }

    cleanup:
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

//    VOID __stdcall DoUpdateSvcDacl(LPTSTR pTrusteeName) // Updates the service DACL to grant control access to the Guest account
//    {
//        SC_HANDLE schSCManager, schService;
//        if (!GetServiceHandle(schSCManager, schService, READ_CONTROL | WRITE_DAC))
//            return;

//        BOOL   bDaclPresent   = FALSE;
//        BOOL   bDaclDefaulted = FALSE;
//        PACL   pacl           = NULL;
//        PACL   pNewAcl        = NULL;

//        // Get the current security descriptor.
//        PSECURITY_DESCRIPTOR psd = NULL;
//        DWORD  dwBytesNeeded  = 0;
//        DWORD  dwSize         = 0;
//        if (!QueryServiceObjectSecurity(schService,
//                                        DACL_SECURITY_INFORMATION,
//                                        &psd,  // using NULL does not work on all versions
//                                        0,
//                                        &dwBytesNeeded))
//        {
//            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
//                dwSize = dwBytesNeeded;
//                psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
//                if (psd == NULL) {
//                    // Note: HeapAlloc does not support GetLastError.
//                    NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//                    goto dacl_cleanup;
//                }

//                if (!QueryServiceObjectSecurity(schService, DACL_SECURITY_INFORMATION, psd,
//                                                    dwSize, &dwBytesNeeded))
//                {
//                    NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//                    goto dacl_cleanup;
//                }

//            } else {
//                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//                goto dacl_cleanup;
//            }
//        }

//        // Get the DACL.
//        if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted))
//        {
//            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//            goto dacl_cleanup;
//        }

//        // Build the ACE.
//        EXPLICIT_ACCESS ea;
//        BuildExplicitAccessWithName(&ea, pTrusteeName, SERVICE_START | SERVICE_STOP | READ_CONTROL | DELETE,
//                                        SET_ACCESS, NO_INHERITANCE);

//        if (SetEntriesInAcl(1, &ea, pacl, &pNewAcl) != ERROR_SUCCESS) {
//            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//            goto dacl_cleanup;
//        }

//        // Initialize a new security descriptor.
//        SECURITY_DESCRIPTOR sd;
//        if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
//        {
//            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//            goto dacl_cleanup;
//        }

//        // Set the new DACL in the security descriptor.
//        if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE)) {
//            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//            goto dacl_cleanup;
//        }

//        // Set the new DACL for the service object.
//        if (!SetServiceObjectSecurity(schService, DACL_SECURITY_INFORMATION, &sd))
//        {
//            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
//            goto dacl_cleanup;
//        } else
//            printf("Service DACL updated successfully\n");

//    dacl_cleanup:
//        CloseServiceHandle(schSCManager);
//        CloseServiceHandle(schService);

//        if (pNewAcl != NULL)
//            LocalFree((HLOCAL)pNewAcl);
//        if (psd != NULL)
//            HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
//    }

    VOID __stdcall DoStopSvc()
    {
        SC_HANDLE schSCManager, schService;
        if (!GetServiceHandle(schSCManager, schService, SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS))
            return;

        DWORD dwStartTime = GetTickCount();
        DWORD dwTimeout = 30000; // 30-second time-out

        // Make sure the service is not already stopped.
        SERVICE_STATUS_PROCESS ssp;
        DWORD dwBytesNeeded;
        if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                                    sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto stop_cleanup;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto stop_cleanup;
        }

        // If a stop is pending, wait for it.
        DWORD dwWaitTime;
        while (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
            printf("Service stop pending...\n");

            // Do not wait longer than the wait hint. A good interval is
            // one-tenth of the wait hint but not less than 1 second
            // and not more than 10 seconds.

            dwWaitTime = ssp.dwWaitHint / 10;

            if (dwWaitTime < 1000)
                dwWaitTime = 1000;
            else
            if (dwWaitTime > 10000)
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

            if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                                        sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
            {
                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
                goto stop_cleanup;
            }

            if (ssp.dwCurrentState == SERVICE_STOPPED) {
                printf("Service stopped successfully.\n");
                goto stop_cleanup;
            }

            if (GetTickCount() - dwStartTime > dwTimeout) {
                printf("Service stop timed out.\n");
                goto stop_cleanup;
            }
        }

        // If the service is running, dependencies must be stopped first.
        StopDependentServices(schSCManager, schService);

        // Send a stop code to the service.
        if (!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto stop_cleanup;
        }

        // Wait for the service to stop.
        while (ssp.dwCurrentState != SERVICE_STOPPED)
        {
            Sleep(ssp.dwWaitHint);
            if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                                        sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
            {
                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
                goto stop_cleanup;
            }

            if (ssp.dwCurrentState == SERVICE_STOPPED)
                break;

            if (GetTickCount() - dwStartTime > dwTimeout) {
                printf("Wait timed out.\n");
                goto stop_cleanup;
            }
        }

    stop_cleanup:
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    VOID __stdcall DoQuerySvc() // Retrieves and displays the current service configuration.
    {
        SC_HANDLE schSCManager, schService;
        if (!GetServiceHandle(schSCManager, schService, SERVICE_QUERY_CONFIG))
            return;

        std::wstringstream wss;
        LPSERVICE_DESCRIPTION lpsd = NULL;

        // Get the configuration information.
        DWORD dwBytesNeeded = 0, cbBufSize = 0;
        LPQUERY_SERVICE_CONFIG lpsc = NULL;
        if (!QueryServiceConfig(schService, NULL, 0, &dwBytesNeeded))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                cbBufSize = dwBytesNeeded;
                lpsc = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, cbBufSize);
            } else {
                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
                goto cleanup;
            }
        }

        if (!QueryServiceConfig(schService, lpsc, cbBufSize, &dwBytesNeeded))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto cleanup;
        }

        if (!QueryServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &dwBytesNeeded))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                cbBufSize = dwBytesNeeded;
                lpsd = (LPSERVICE_DESCRIPTION)LocalAlloc(LMEM_FIXED, cbBufSize);
            } else {
                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
                goto cleanup;
            }
        }

        if (!QueryServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)lpsd, cbBufSize, &dwBytesNeeded))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            goto cleanup;
        }

        // Print the configuration information.
        wss << L"Service configuration:\n"
            << L"  Type: 0x" << std::hex << lpsc->dwServiceType << L"\n"
            << L"  Start Type: 0x" << std::hex << lpsc->dwStartType << L"\n"
            << L"  Error Control: 0x" << std::hex << lpsc->dwErrorControl << L"\n"
            << L"  Binary path: " << lpsc->lpBinaryPathName << L"\n"
            << L"  Account: " << lpsc->lpServiceStartName << L"\n";
        if (lpsd->lpDescription != NULL && lstrcmp(lpsd->lpDescription, TEXT("")) != 0)
            wss << L"  Description: " << lpsd->lpDescription << L"\n";
        if (lpsc->lpLoadOrderGroup != NULL && lstrcmp(lpsc->lpLoadOrderGroup, TEXT("")) != 0)
            wss << L"  Load order group: " << lpsc->lpLoadOrderGroup << L"\n";
        if (lpsc->dwTagId != 0)
            wss << L"  Tag ID: " << std::dec << lpsc->dwTagId << L"\n";
        if (lpsc->lpDependencies != NULL && lstrcmp(lpsc->lpDependencies, TEXT("")) != 0)
            wss << L"  Dependencies: " << lpsc->lpDependencies << L"\n";

        LocalFree(lpsc);
        LocalFree(lpsd);

    cleanup:
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        NS_Logger::WriteLog(wss.str(), true);
    }

    VOID __stdcall DoDisableSvc()
    {
        SC_HANDLE schSCManager, schService;
        if (!GetServiceHandle(schSCManager, schService, SERVICE_CHANGE_CONFIG))
            return;

        // Change the service start type.
        if (!ChangeServiceConfig(
            schService,        // handle of service
            SERVICE_NO_CHANGE, // service type: no change
            SERVICE_DISABLED,  // service start type
            SERVICE_NO_CHANGE, // error control: no change
            NULL,              // binary path: no change
            NULL,              // load order group: no change
            NULL,              // tag ID: no change
            NULL,              // dependencies: no change
            NULL,              // account name: no change
            NULL,              // password: no change
            NULL) )            // display name: no change
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    VOID __stdcall DoEnableSvc()
    {
        SC_HANDLE schSCManager, schService;
        if (!GetServiceHandle(schSCManager, schService, SERVICE_CHANGE_CONFIG))
            return;

        // Change the service start type.
        if (!ChangeServiceConfig(
                schService,            // handle of service
                SERVICE_NO_CHANGE,     // service type: no change
                SERVICE_DEMAND_START,  // service start type
                SERVICE_NO_CHANGE,     // error control: no change
                NULL,                  // binary path: no change
                NULL,                  // load order group: no change
                NULL,                  // tag ID: no change
                NULL,                  // dependencies: no change
                NULL,                  // account name: no change
                NULL,                  // password: no change
                NULL) )                // display name: no change
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    VOID __stdcall DoUpdateSvcDesc(LPTSTR szDesc) // Updates the service description
    {
        SC_HANDLE schSCManager, schService;
        if (!GetServiceHandle(schSCManager, schService, SERVICE_CHANGE_CONFIG))
            return;

        // Change the service description.
        SERVICE_DESCRIPTION sd;
        sd.lpDescription = szDesc;

        if (!ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd))
        {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    VOID __stdcall DoDeleteSvc() // Deletes a service from the SCM database
    {
        SC_HANDLE schSCManager, schService;
        if (!GetServiceHandle(schSCManager, schService, DELETE))
            return;

        // Delete the service.
        if (!DeleteService(schService))
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    VOID __stdcall SvcReportEvent(LPCTSTR errorDescription)
    {
        HANDLE hEventSource = RegisterEventSource(NULL, SERVICE_NAME);
        if (hEventSource != NULL) {
            TCHAR Buffer[80];
            StringCchPrintfW(Buffer, 80, TEXT("%s"), errorDescription);
            LPCTSTR lpszStrings[2];
            lpszStrings[0] = SERVICE_NAME;
            lpszStrings[1] = Buffer;

            ReportEvent(hEventSource,        // event log handle
                        EVENTLOG_ERROR_TYPE, // event type
                        0,                   // event category
                        SVC_ERROR,           // event identifier
                        NULL,                // no security identifier
                        2,                   // size of lpszStrings array
                        0,                   // no binary data
                        lpszStrings,         // array of strings
                        NULL);               // no binary data

            DeregisterEventSource(hEventSource);
        }
    }
}
