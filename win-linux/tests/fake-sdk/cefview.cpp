/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

// Fake of desktop-sdk's include/cefview.h

#include "cefview.h"
#include "fakesdk_p.h"

using FakeSdk::record;
using FakeSdk::recordEvent;
using FakeSdk::state;

class CCefView_Private
{
public:
    CCefViewWidgetImpl* widget = nullptr;
    CAscApplicationManager* manager = nullptr;
    std::wstring url;
    bool modified = false;
};

CCefView::CCefView(CCefViewWidgetImpl* parent, int nId)
    : m_nId(nId), m_eWrapperType(cvwtSimple), m_pInternal(new CCefView_Private)
{
    m_pInternal->widget = parent;
}

CCefView::~CCefView()
{
    state().views.erase(m_nId);
    delete m_pInternal;
}

void CCefView::load(const std::wstring& url)
{
    record("CCefView::load", url);
    m_pInternal->url = url;
}

void CCefView::reload() { record("CCefView::reload", m_pInternal->url); }
std::wstring CCefView::GetUrl() { return m_pInternal->url; }
std::wstring CCefView::GetOriginalUrl() { return m_pInternal->url; }
std::wstring CCefView::GetUrlAsLocal() { return m_pInternal->url; }
void CCefView::focus(bool) {}
void CCefView::resizeEvent() {}
void CCefView::moveEvent() {}
bool CCefView::isDoubleResizeEvent() { return false; }

void CCefView::Apply(NSEditorApi::CAscMenuEvent* event)
{
    recordEvent("CCefView::Apply", event);
}

NSEditorApi::CAscMenuEvent* CCefView::ApplySync(NSEditorApi::CAscMenuEvent* event)
{
    recordEvent("CCefView::ApplySync", event);
    return nullptr;
}

NSEditorApi::CAscCefMenuEvent* CCefView::CreateCefEvent(int nType)
{
    NSEditorApi::CAscCefMenuEvent* event = new NSEditorApi::CAscCefMenuEvent(nType);
    event->put_SenderId(m_nId);
    return event;
}

bool CCefView::StartDownload(const std::wstring& sUrl)
{
    record("CCefView::StartDownload", sUrl);
    return true;
}

void CCefView::SetExternalCloud(const std::wstring& sProviderId)
{
    record("CCefView::SetExternalCloud", sProviderId);
}

CAscApplicationManager* CCefView::GetAppManager() { return m_pInternal->manager; }
void CCefView::SetAppManager(CAscApplicationManager* manager) { m_pInternal->manager = manager; }
CCefViewWidgetImpl* CCefView::GetWidgetImpl() { return m_pInternal->widget; }
void CCefView::OnDestroyWidgetImpl() { m_pInternal->widget = nullptr; }
int CCefView::GetId() { return m_nId; }
CefViewWrapperType CCefView::GetType() { return m_eWrapperType; }
void CCefView::SetModified(bool bIsModified) { m_pInternal->modified = bIsModified; }
bool CCefView::GetModified() { return m_pInternal->modified; }
bool CCefView::IsPresentationReporter() { return false; }
double CCefView::GetDeviceScale() { return 1; }
int CCefView::GetPrintPageOrientation(const int&) { return 0; }
bool CCefView::IsDestroy() { return state().views.count(m_nId) == 0; }

void CCefView::SetParentWidgetInfo(const std::wstring& json)
{
    record("CCefView::SetParentWidgetInfo", json);
}

int CCefView::GetRecentId() { return -1; }

CCefViewEditor::CCefViewEditor(CCefViewWidgetImpl* parent, int nId)
    : CCefView(parent, nId), m_eType(AscEditorType::etUndefined)
{
    m_eWrapperType = cvwtEditor;
}

CCefViewEditor::~CCefViewEditor() {}
void CCefViewEditor::SetEditorType(AscEditorType eType) { m_eType = eType; }
AscEditorType CCefViewEditor::GetEditorType() { return m_eType; }

void CCefViewEditor::OpenLocalFile(const std::wstring& sFilePath, const int&, const std::wstring&)
{
    record("CCefViewEditor::OpenLocalFile", sFilePath);
}

void CCefViewEditor::CreateLocalFile(const AscEditorType& type, const std::wstring& sName, const std::wstring&)
{
    record("CCefViewEditor::CreateLocalFile", std::to_wstring((int)type) + L" " + sName);
}

void CCefViewEditor::CreateLocalFile(const AscEditorType& type, const int&, const std::wstring& sName)
{
    record("CCefViewEditor::CreateLocalFile", std::to_wstring((int)type) + L" " + sName);
}

bool CCefViewEditor::OpenRecoverFile(const int& nId)
{
    record("CCefViewEditor::OpenRecoverFile", std::to_wstring(nId));
    return true;
}

bool CCefViewEditor::OpenRecentFile(const int& nId)
{
    record("CCefViewEditor::OpenRecentFile", std::to_wstring(nId));
    return true;
}

bool CCefViewEditor::CheckCloudCryptoNeedBuild() { return false; }
bool CCefViewEditor::IsBuilding() { return false; }
bool CCefViewEditor::IsSaveLocked() { return false; }
std::wstring CCefViewEditor::GetLocalFilePath() { return GetUrl(); }

int CCefViewEditor::GetFileFormat(const std::wstring& sFilePath)
{
    auto it = state().fileFormats.find(sFilePath);
    return it == state().fileFormats.end() ? 0 : it->second;
}
