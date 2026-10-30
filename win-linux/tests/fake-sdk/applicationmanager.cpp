/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

// Fake of desktop-sdk's include/applicationmanager.h and cefapplication.h

#include "applicationmanager.h"
#include "fakesdk_p.h"

using FakeSdk::record;
using FakeSdk::recordEvent;
using FakeSdk::state;

static CAscApplicationManager* s_manager = nullptr;
static CAscDpiChecker* s_dpiChecker = nullptr;

// Same steps as the real SDK
double Core_GetMonitorScale(const unsigned int& xDpi, const unsigned int& yDpi)
{
    double scale = (xDpi + yDpi) / (2 * 96.0);
    scale = 0.25 * (int)((scale + 0.125) / 0.25);
    return scale > 5 ? 5 : scale < 1 ? 1 : scale;
}

CAscApplicationSettings::CAscApplicationSettings()
    : use_system_fonts(true), sign_support(true), pass_support(true), protect_support(true),
      macroses_support(true), plugins_support(true), templates_image_width(0), templates_image_height(0)
{
}

CAscApplicationSettings::~CAscApplicationSettings() {}

void CAscApplicationSettings::SetUserDataPath(std::wstring sPath)
{
    app_data_path = sPath;
}

CUserSettings::CUserSettings() : m_pInternal(nullptr) {}
CUserSettings::~CUserSettings() {}

std::wstring CUserSettings::Get(const std::wstring& name)
{
    auto it = state().userSettings.find(name);
    return it == state().userSettings.end() ? L"" : it->second;
}

void CUserSettings::Set(const std::wstring& name, const std::wstring& value)
{
    state().userSettings[name] = value;
}

CAscDpiChecker::CAscDpiChecker(CAscApplicationManager* pManager) : m_pManager(pManager) {}
CAscDpiChecker::~CAscDpiChecker() {}

int CAscDpiChecker::GetWindowDpi(WindowHandleId, unsigned int* x, unsigned int* y)
{
    *x = *y = 96;
    return 0;
}

int CAscDpiChecker::GetMonitorDpi(int, unsigned int* x, unsigned int* y)
{
    *x = *y = 96;
    return 0;
}

int CAscDpiChecker::GetWidgetImplDpi(CCefViewWidgetImpl*, unsigned int* x, unsigned int* y)
{
    *x = *y = 96;
    return 0;
}

double CAscDpiChecker::GetScale(unsigned int x, unsigned int y)
{
    return Core_GetMonitorScale(x, y);
}

CAscApplicationManager::CAscApplicationManager() : m_pInternal(nullptr)
{
    s_manager = this;
}

CAscApplicationManager::~CAscApplicationManager()
{
    if (s_manager == this)
        s_manager = nullptr;
}

void CAscApplicationManager::StartSpellChecker() { record("CAscApplicationManager::StartSpellChecker"); }
void CAscApplicationManager::StopSpellChecker() {}
void CAscApplicationManager::StartKeyboardChecker() {}
void CAscApplicationManager::CheckKeyboard() {}

std::vector<std::pair<std::string, std::string>> CAscApplicationManager::GetKeyboardLayoutList() const
{
    return {};
}

bool CAscApplicationManager::IsPlatformKeyboardSupport() { return false; }
int CAscApplicationManager::GetPlatformKeyboardLayout() { return -1; }
bool CAscApplicationManager::IsInitFonts() { return true; }
void CAscApplicationManager::CheckFonts(bool) { record("CAscApplicationManager::CheckFonts"); }

void CAscApplicationManager::SetEventListener(NSEditorApi::CAscCefMenuEventListener* listener)
{
    state().listener = listener;
}

NSEditorApi::CAscCefMenuEventListener* CAscApplicationManager::GetEventListener()
{
    return state().listener;
}

void CAscApplicationManager::Apply(NSEditorApi::CAscMenuEvent* event)
{
    recordEvent("CAscApplicationManager::Apply", event);
}

NSEditorApi::CAscMenuEvent* CAscApplicationManager::ApplySync(NSEditorApi::CAscMenuEvent* event)
{
    recordEvent("CAscApplicationManager::ApplySync", event);
    return nullptr;
}

CCefView* CAscApplicationManager::CreateCefView(CCefViewWidgetImpl* parent)
{
    CCefView* view = new CCefView(parent, GenerateNextViewId());
    view->SetAppManager(this);
    state().views[view->GetId()] = view;
    return view;
}

CCefViewEditor* CAscApplicationManager::CreateCefEditor(CCefViewWidgetImpl* parent)
{
    CCefViewEditor* view = new CCefViewEditor(parent, GenerateNextViewId());
    view->SetAppManager(this);
    state().views[view->GetId()] = view;
    return view;
}

CCefView* CAscApplicationManager::GetViewById(int nId)
{
    return FakeSdk::view(nId);
}

CCefView* CAscApplicationManager::GetViewByUrl(const std::wstring& url)
{
    for (const auto& view : state().views) {
        if (view.second->GetUrl() == url)
            return view.second;
    }
    return nullptr;
}

CCefView* CAscApplicationManager::GetViewByRecentId(int) { return nullptr; }

std::vector<int> CAscApplicationManager::GetViewsId()
{
    std::vector<int> ids;
    for (const auto& view : state().views)
        ids.push_back(view.first);
    return ids;
}

void CAscApplicationManager::CloseApplication() { record("CAscApplicationManager::CloseApplication"); }
void CAscApplicationManager::StartSaveDialog(const std::wstring&, unsigned int) {}

void CAscApplicationManager::EndSaveDialog(const std::wstring& sPath, unsigned int)
{
    record("CAscApplicationManager::EndSaveDialog", sPath);
}

void CAscApplicationManager::Logout(std::wstring strUrl)
{
    record("CAscApplicationManager::Logout", strUrl);
}

void CAscApplicationManager::CancelDownload(unsigned int nDownloadId)
{
    record("CAscApplicationManager::CancelDownload", std::to_wstring(nDownloadId));
}

bool CAscApplicationManager::IsExternalEventLoop() { return false; }
IExternalMessageLoop* CAscApplicationManager::GetExternalMessageLoop() { return nullptr; }
void CAscApplicationManager::ExternalMessageLoop_OnExecute(void*) {}
void CAscApplicationManager::ExternalMessageLoop_OnTimeout() {}
void CAscApplicationManager::InitAdditionalEditorParams(std::wstring&) {}

CAscDpiChecker* CAscApplicationManager::GetDpiChecker()
{
    if (!s_dpiChecker && s_manager)
        s_dpiChecker = s_manager->InitDpiChecker();
    return s_dpiChecker;
}

double CAscApplicationManager::GetMonitorScaleByWindow(const WindowHandleId&, unsigned int& nDpiX, unsigned int& nDpiY)
{
    nDpiX = nDpiY = 96;
    return 1;
}

void CAscApplicationManager::SetEventToAllMainWindows(NSEditorApi::CAscMenuEvent* event)
{
    recordEvent("CAscApplicationManager::SetEventToAllMainWindows", event);
}

void CAscApplicationManager::SetEventToAllWindows(NSEditorApi::CAscMenuEvent* event)
{
    recordEvent("CAscApplicationManager::SetEventToAllWindows", event);
}

NSAscCrypto::CAscKeychain* CAscApplicationManager::GetKeychainEngine() { return nullptr; }

void CAscApplicationManager::DestroyCefView(int nId, bool)
{
    record("CAscApplicationManager::DestroyCefView", std::to_wstring(nId));
    // The view stays alive: the app's widget still points to it
    state().views.erase(nId);
}

void CAscApplicationManager::OnTerminateRequest() {}

int CAscApplicationManager::GetFileFormatByExtentionForSave(const std::wstring&) { return 0; }

CUserSettings* CAscApplicationManager::GetUserSettings()
{
    return new CUserSettings();
}

void CAscApplicationManager::SetRecentPin(const int& nId, const bool& bIsPin)
{
    record("CAscApplicationManager::SetRecentPin", std::to_wstring(nId) + (bIsPin ? L" pin" : L" unpin"));
}

void CAscApplicationManager::SetRendererProcessVariable(const std::wstring& sVariable)
{
    record("CAscApplicationManager::SetRendererProcessVariable", sVariable);
}

void CAscApplicationManager::UpdatePlugins(const std::wstring& sJson)
{
    record("CAscApplicationManager::UpdatePlugins", sJson);
}

bool CAscApplicationManager::IsUseSystemScaling() { return false; }
std::wstring CAscApplicationManager::GetExternalSchemeName() { return L"euro-office"; }

bool CAscApplicationManager::InstallPluginFromStore(const std::wstring& sName)
{
    record("CAscApplicationManager::InstallPluginFromStore", sName);
    return true;
}

bool CAscApplicationManager::RemoveRecentByViewId(const int& viewId)
{
    record("CAscApplicationManager::RemoveRecentByViewId", std::to_wstring(viewId));
    return true;
}

int CAscApplicationManager::CallCommand(const std::wstring& command)
{
    record("CAscApplicationManager::CallCommand", command);
    return 0;
}

CertificateFileStatus CAscApplicationManager::CheckCertificateFile(const std::wstring& sFile, const std::string&)
{
    record("CAscApplicationManager::CheckCertificateFile", sFile);
    return CertificateFileStatus::Unsupported;
}

CertificateFileStatus CAscApplicationManager::CheckPrivateKeyFile(const std::wstring& sFile, const std::string&)
{
    record("CAscApplicationManager::CheckPrivateKeyFile", sFile);
    return CertificateFileStatus::Unsupported;
}

int CAscApplicationManager::GenerateNextViewId()
{
    return ++state().lastViewId;
}

CAscDpiChecker* CAscApplicationManager::InitDpiChecker()
{
    return new CAscDpiChecker(this);
}

CApplicationCEF::CApplicationCEF() : m_pInternal(nullptr) {}
CApplicationCEF::~CApplicationCEF() {}
int CApplicationCEF::Init_CEF(CAscApplicationManager*, int, char**) { return 0; }
void CApplicationCEF::Close() {}
int CApplicationCEF::RunMessageLoop(bool&) { return 0; }
void CApplicationCEF::DoMessageLoopEvent() {}
bool CApplicationCEF::ExitMessageLoop() { return true; }
bool CApplicationCEF::IsChromiumSubprocess() { return false; }
void CApplicationCEF::Prepare(int, char**) {}
