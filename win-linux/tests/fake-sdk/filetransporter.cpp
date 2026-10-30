/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

// Fake of core's CFileDownloader, which the app uses directly for the
// editor's avatar (Qt Network HTTPS isn't usable in the app). It records the
// URL and never downloads.

#include "Network/FileTransporter/include/FileTransporter.h"
#include "fakesdk_p.h"

using FakeSdk::record;

namespace NSNetwork
{
    namespace NSFileTransport
    {
        class CFileTransporter_private
        {
        public:
            std::wstring url;
            std::wstring filePath;
        };

        CFileDownloader::CFileDownloader(std::wstring sFileUrl, bool)
            : m_pInternal(new CFileTransporter_private)
        {
            m_pInternal->url = sFileUrl;
        }

        CFileDownloader::~CFileDownloader() { delete m_pInternal; }
        void CFileDownloader::SetSession(CSession*) {}
        void CFileDownloader::SetFilePath(const std::wstring& sFilePath) { m_pInternal->filePath = sFilePath; }
        std::wstring CFileDownloader::GetFilePath() { return m_pInternal->filePath; }
        bool CFileDownloader::IsFileDownloaded() { return false; }
        void CFileDownloader::SetFileUrl(const std::wstring& sFileUrl, bool) { m_pInternal->url = sFileUrl; }
        bool CFileDownloader::DownloadSync() { record("CFileDownloader::Start", m_pInternal->url); return false; }
        void CFileDownloader::DownloadAsync() { record("CFileDownloader::Start", m_pInternal->url); }
        void CFileDownloader::Start(int) { record("CFileDownloader::Start", m_pInternal->url); }
        int CFileDownloader::GetPriority() { return 0; }
        void CFileDownloader::Suspend() {}
        void CFileDownloader::Resume() {}
        void CFileDownloader::Stop() {}
        void CFileDownloader::StopNoJoin() {}
        void CFileDownloader::Cancel() {}
        int CFileDownloader::IsRunned() { return 0; }
        void CFileDownloader::SetEvent_OnProgress(std::function<void(int)>) {}
        void CFileDownloader::SetEvent_OnComplete(std::function<void(int)>) {}
    }
}
