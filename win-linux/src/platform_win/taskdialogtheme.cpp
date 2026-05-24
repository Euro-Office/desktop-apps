/**
 * MIT License
 *
 * Copyright (c) 2026 Mohmed abdel-fattah
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

///////////////////////////////////////////////////////////////////////////////
// DarkTaskDialog-Native  —  Win32 TaskDialog dark-mode support
//
// Rendering strategy derived from comctl32.dll UIFILE (resource 4255, 26100)
//
// Win10 path  (g_hasNativeTheme == false):
//   WM_PRINTCLIENT → BPBF_TOPDOWNDIB → pixel-swap panel colours →
//   FillRect + DrawIconEx          for MainIcon / FootnoteIcon
//   FillRect + DrawThemeBackground for ExpandoButton glyph (all states)
//   FillRect + DrawThemeBackground for Checkbox glyph    (all states)
//   FillRect + DrawThemeTextEx     for all text elements
//
// Win11 path  (g_hasNativeTheme == true):
//   no pixel-swap; DarkMode_* UxTheme handles panel backgrounds
//   DrawThemeText for all text elements
//
// Child-window theming (AllowForTaskDialog → EnumChildWindows):
//   "TaskDialog" DirectUI window      → SetWindowTheme("DarkMode_Explorer") + DirectUISubclassProc
//   "CCSysLink"  footnote link        → GetParent → WmCtColorSubclassProc(kFootnote brush)
//   CommandLink_ button               → SetWindowTheme("DarkMode_Explorer")
//                                       GetParent → WmCtColorSubclassProc(kPrimary brush)
//   CommandButton_ button             → SetWindowTheme("DarkMode_Explorer")
//                                       GetParent → WmCtColorSubclassProc(kSecondary brush)
//   RadioButton_ button               → SetWindowTheme("DarkMode_DarkTheme" or "DarkMode_Explorer")
//                                       GetParent → WmCtColorSubclassProc(kPrimary brush)
//   ProgressBar                       → SetWindowTheme("DarkMode_CopyEngine" or "DarkMode_Explorer")
//
// Theme override:
//   AllowForTaskDialog accepts TaskDialogTheme::Theme (System / Dark / Light).
//   Subclass presence encodes the dark/light state:
//     attached    → dialog is dark
//     not present → dialog is light (native rendering)
//   AllowForTaskDialog with a light-resolving Theme removes all subclasses.
///////////////////////////////////////////////////////////////////////////////

#include "taskdialogtheme.h"
#include <windowsx.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <shellapi.h>
#include <uiautomation.h>
#include <atlbase.h>
#include <string>
#include <vector>
#include <unordered_map>

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")

#ifndef TDLG_PRIMARYPANEL
#  define TDLG_PRIMARYPANEL          1
#  define TDLG_MAININSTRUCTIONPANE   2
#  define TDLG_CONTENTPANE           4   // UIFILE TaskDialogStyle part 4
#  define TDLG_EXPINFOPANE           6   // UIFILE TaskDialogStyle part 6
#  define TDLG_SECONDARYPANEL        8   // UIFILE TaskDialog part 8
#  define TDLG_EXPANDOTEXT          12   // UIFILE TaskDialogStyle part 12
#  define TDLG_EXPANDOBUTTON        13   // UIFILE TaskDialog part 13
#  define TDLG_VERIFICATIONTEXT     14   // UIFILE TaskDialogStyle part 14
#  define TDLG_FOOTNOTEPANE         15   // UIFILE TaskDialogStyle part 15
#  define TDLG_EXPANDEDFOOTERAREA   18   // UIFILE TaskDialogStyle part 18
#  define TDLG_RADIOBUTTONPANE      21   // UIFILE TaskDialogStyle part 21
#endif

// ExpandoButton glyph states — TaskDialog part 13, states 1–6
#ifndef TDLGEBS_NORMAL
#  define TDLGEBS_NORMAL          1
#  define TDLGEBS_HOVER           2
#  define TDLGEBS_PRESSED         3
#  define TDLGEBS_EXPANDEDNORMAL  4
#  define TDLGEBS_EXPANDEDHOVER   5
#  define TDLGEBS_EXPANDEDPRESSED 6
#endif

// ─── DarkColors ───────────────────────────────────────────────────────────────
// Matches Win11 25H2 DarkMode_DarkTheme resolved values.

namespace DarkColors
{
    constexpr COLORREF kPrimary = RGB(32, 32, 32);
    constexpr COLORREF kSecondary = RGB(44, 44, 44);
    constexpr COLORREF kFootnote = RGB(44, 44, 44);
    constexpr COLORREF kSeparator = RGB(77, 77, 77);

    constexpr COLORREF kTextNormal = RGB(255, 255, 255);
    constexpr COLORREF kTextInstruct = RGB(153, 235, 255); // MainInstruction accent
    constexpr COLORREF kTextContent = RGB(255, 255, 255);
    constexpr COLORREF kTextExpInfo = RGB(255, 255, 255);
    constexpr COLORREF kTextExpando = RGB(255, 255, 255);
    constexpr COLORREF kTextVerify = RGB(255, 255, 255);
    constexpr COLORREF kTextFootnote = RGB(224, 224, 224);
    constexpr COLORREF kTextFtrExp = RGB(224, 224, 224);
    constexpr COLORREF kTextRadio = RGB(255, 255, 255);
}

// ─── Global state ─────────────────────────────────────────────────────────────
// g_darkEnabled  — set by Init() from registry, or overridden by Init(bool).
// g_themeForced  — true when Init(bool) was used; suppresses live OS re-reads.
// g_hasNativeTheme — true on Win11 builds where DarkMode_* TaskDialog exists.

static bool g_hasNativeTheme = false;
static TaskDialogTheme::Theme g_theme = TaskDialogTheme::Theme::System;

// Subclass IDs
static constexpr UINT_PTR kMainSubclassId = 0xDEADBEEFul;
static constexpr UINT_PTR kDirectUISubclassId = 0xBADF00Dul;
static constexpr UINT_PTR kCtlColorId = 0xC0FFEE01ul;

// Forward declarations
static bool     IsDarkThemeActive(const wchar_t* dark, const wchar_t* base);
static bool     ReadOsDarkMode();
static LRESULT CALLBACK TaskDialogMainSubclassProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
static LRESULT CALLBACK DirectUISubclassProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
static LRESULT CALLBACK WmCtColorSubclassProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

// ─── UIAElementInfo ───────────────────────────────────────────────────────────
// Cached per-element data queried once per TDN_CREATED / TDN_NAVIGATED.

struct UIAElementInfo
{
    std::wstring automationId;
    std::wstring name;
    RECT         rect = {};
    LONG         legacyState = 0; // for VerificationCheckBox only
};

// ─── DirectUIState ────────────────────────────────────────────────────────────

struct DirectUIState
{
    // ── Two separate theme handles (see UIFILE analysis) ─────────────────────
    // "TaskDialog"      → panel bg, ExpandoButton glyph drawing
    // "TaskDialogStyle" → text colour / font queries for all text parts
    HTHEME hTD = nullptr;   // OpenThemeData("TaskDialog")
    HTHEME hTDS = nullptr;   // OpenThemeData("TaskDialogStyle")
    HTHEME hButton = nullptr;   // OpenThemeData("Button") for checkbox glyph
    bool   isDarkTheme = false; // a DarkMode_* TDS was resolved
    bool   themesOk = false;

    // ── Cached per-session brushes ────────────────────────────────────────────
    HBRUSH brPrimary = nullptr; // kPrimary   = RGB(32,32,32)
    HBRUSH brSecondary = nullptr; // kSecondary = RGB(44,44,44)
    HBRUSH brFootnote = nullptr; // kFootnote  = RGB(44,44,44) same value, separate object

    // ── UIA element cache (filled once per TDN_CREATED / TDN_NAVIGATED) ──────
    std::vector<UIAElementInfo> elements;
    bool elemsOk = false;

    // ── Mouse state (populated by WM_MOUSE* messages) ─────────────────────────
    bool tracking = false;
    bool pressing = false;
    int  hotIdx = -1;

    // ── Logical UI state ──────────────────────────────────────────────────────
    bool isExpanded = false;
    bool isChecked = false;
    bool defExpanded = false;
    bool defChecked = false;

    // ── Per-dialog theme override ─────────────────────────────────────────────
    // Stored so WM_THEMECHANGED / WM_SETTINGCHANGE inside DirectUISubclassProc
    // can decide whether an OS theme switch should tear down the dark subclasses.
    TaskDialogTheme::Theme theme = TaskDialogTheme::Theme::System;

    TASKDIALOGCONFIG* pCfg = nullptr;

    // ── Helpers ───────────────────────────────────────────────────────────────
    void CloseThemes()
    {
        if (hTD) { CloseThemeData(hTD);     hTD = nullptr; }
        if (hTDS) { CloseThemeData(hTDS);    hTDS = nullptr; }
        if (hButton) { CloseThemeData(hButton); hButton = nullptr; }
        themesOk = false;
    }
    void DestroyBrushes()
    {
        if (brPrimary) { DeleteObject(brPrimary);   brPrimary = nullptr; }
        if (brSecondary) { DeleteObject(brSecondary); brSecondary = nullptr; }
        if (brFootnote) { DeleteObject(brFootnote);  brFootnote = nullptr; }
    }
    void Destroy() { CloseThemes(); DestroyBrushes(); elements.clear(); elemsOk = false; }
};

static thread_local std::unordered_map<HWND, DirectUIState> gs_states;

static DirectUIState& GetState(HWND h) { return gs_states[h]; }

static void DestroyState(HWND h)
{
    auto it = gs_states.find(h);
    if (it != gs_states.end()) { it->second.Destroy(); gs_states.erase(it); }
}

// ─── OS dark-mode query ───────────────────────────────────────────────────────
// Reads the documented registry value written by Settings / Explorer.
// Returns true when the OS apps-use-dark-mode preference is ON.
// Called live (not cached) so Theme::System always reflects the current state.

static bool ReadOsDarkMode()
{
    DWORD value = 1, size = sizeof(value);
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value == 0; // 0 = dark, 1 = light
}

// ─── Theme helpers ────────────────────────────────────────────────────────────

static UINT _GetDpiForWindow(HWND hwnd)
{
    if (HMODULE hUser32 = GetModuleHandleA("user32")) {
        UINT(WINAPI *pGetDpiForWindow)(HWND) = NULL;
        *(FARPROC*)&pGetDpiForWindow = GetProcAddress(hUser32, "GetDpiForWindow");
        if (pGetDpiForWindow)
            return pGetDpiForWindow(hwnd);
    }
    return 96;
}

static HTHEME _OpenThemeDataForDpi(HWND hwnd, LPCWSTR pszClassList, UINT dpi)
{
    HTHEME hTheme = nullptr;
    if (HMODULE hUxTheme = LoadLibraryExA("uxtheme", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)) {
        HTHEME(WINAPI *pOpenThemeDataForDpi)(HWND, LPCWSTR, UINT) = NULL;
        *(FARPROC*)&pOpenThemeDataForDpi = GetProcAddress(hUxTheme, "OpenThemeDataForDpi");
        if (pOpenThemeDataForDpi)
            hTheme = pOpenThemeDataForDpi(hwnd, pszClassList, dpi);
        FreeLibrary(hUxTheme);
    }
    return hTheme;
}

static bool IsDarkThemeActive(const wchar_t* dark, const wchar_t* base)
{
    HTHEME hD = OpenThemeData(nullptr, dark);
    HTHEME hB = OpenThemeData(nullptr, base);
    bool a = (hD && hD != hB);
    if (hD) CloseThemeData(hD);
    if (hB) CloseThemeData(hB);
    return a;
}

static void RefreshThemes(HWND hwnd, DirectUIState& s)
{
    s.CloseThemes();
    const UINT dpi = _GetDpiForWindow(hwnd);

    auto Open = [&](const wchar_t* cls) -> HTHEME {
        HTHEME h = _OpenThemeDataForDpi(hwnd, cls, dpi);
        return h ? h : OpenThemeData(hwnd, cls);
        };

    // Detect available dark-theme variants (Win11 25H2 vs older builds)
    const bool hasDarkTheme = IsDarkThemeActive(L"DarkMode_DarkTheme::TaskDialog", L"TaskDialog");

    s.isDarkTheme = hasDarkTheme || g_hasNativeTheme;

    // "TaskDialog" panel/glyph handle
    if (hasDarkTheme)            s.hTD = Open(L"DarkMode_DarkTheme::TaskDialog");
    if (!s.hTD && g_hasNativeTheme) s.hTD = Open(L"DarkMode_Explorer::TaskDialog");
    if (!s.hTD)                  s.hTD = Open(L"TaskDialog"); // light fallback

    // "TaskDialogStyle" colour/font handle
    if (g_hasNativeTheme)  s.hTDS = Open(L"DarkMode_Explorer::TaskDialog");
    if (!s.hTDS)           s.hTDS = Open(L"TaskDialog"); // light fallback

    // Button theme for checkbox glyph
    s.hButton = Open(L"Button");
    s.themesOk = true;
}

static void EnsureBrushes(DirectUIState& s)
{
    if (!s.brPrimary)   s.brPrimary = CreateSolidBrush(DarkColors::kPrimary);
    if (!s.brSecondary) s.brSecondary = CreateSolidBrush(DarkColors::kSecondary);
    if (!s.brFootnote)  s.brFootnote = CreateSolidBrush(DarkColors::kFootnote);
}

// ─── ResolveDark ──────────────────────────────────────────────────────────────
// Converts the three-way Theme enum to a concrete bool at call time.
// Theme::System performs a live OS read (or respects Init(bool) forced value).

static bool ResolveDark(TaskDialogTheme::Theme t)
{
    switch (t)
    {
    case TaskDialogTheme::Theme::Dark: return true;
    case TaskDialogTheme::Theme::Light : return false;
    default: return ReadOsDarkMode();
    }
}

// ─── Text colour lookup ───────────────────────────────────────────────────────
// Query TMT_TEXTCOLOR from hTDS for the given UIFILE part.
// Falls back to DarkColors constants on Win10 where DarkMode_* TaskDialogStyle
// does not exist.

static COLORREF GetTextColor(const DirectUIState& s, int uifilePart)
{
    if (s.isDarkTheme && s.hTDS)
    {
        COLORREF c = RGB(255, 255, 255);
        if (SUCCEEDED(GetThemeColor(s.hTDS, uifilePart, 0, TMT_TEXTCOLOR, &c)))
            return c;
    }
    switch (uifilePart)
    {
    case TDLG_MAININSTRUCTIONPANE: return DarkColors::kTextInstruct;
    case TDLG_CONTENTPANE:         return DarkColors::kTextContent;
    case TDLG_EXPINFOPANE:         return DarkColors::kTextExpInfo;
    case TDLG_EXPANDOTEXT:         return DarkColors::kTextExpando;
    case TDLG_VERIFICATIONTEXT:    return DarkColors::kTextVerify;
    case TDLG_FOOTNOTEPANE:        return DarkColors::kTextFootnote;
    case TDLG_EXPANDEDFOOTERAREA:  return DarkColors::kTextFtrExp;
    case TDLG_RADIOBUTTONPANE:     return DarkColors::kTextRadio;
    default:                       return DarkColors::kTextNormal;
    }
}

// ─── UIA element cache ────────────────────────────────────────────────────────

static bool QueryElements(HWND hwnd, std::vector<UIAElementInfo>& out)
{
    out.clear();
    CComPtr<IUIAutomation> pAuto;
    if (FAILED(pAuto.CoCreateInstance(__uuidof(CUIAutomation)))) return false;

    CComPtr<IUIAutomationElement> pRoot;
    if (FAILED(pAuto->ElementFromHandle(hwnd, &pRoot))) return false;

    CComPtr<IUIAutomationTreeWalker> pWalker;
    pAuto->get_ContentViewWalker(&pWalker);

    CComPtr<IUIAutomationElement> pChild;
    pWalker->GetFirstChildElement(pRoot, &pChild);

    while (pChild)
    {
        UIAElementInfo info{};
        pChild->get_CurrentBoundingRectangle(&info.rect);
        ScreenToClient(hwnd, reinterpret_cast<POINT*>(&info.rect.left));
        ScreenToClient(hwnd, reinterpret_cast<POINT*>(&info.rect.right));

        { CComBSTR b; pChild->get_CurrentAutomationId(&b); if (b) info.automationId = (LPCWSTR)b; }
        { CComBSTR b; pChild->get_CurrentName(&b);         if (b) info.name = (LPCWSTR)b; }

        if (info.automationId == L"VerificationCheckBox")
        {
            VARIANT v; VariantInit(&v);
            if (SUCCEEDED(pChild->GetCurrentPropertyValue(
                UIA_LegacyIAccessibleStatePropertyId, &v)) && v.vt == VT_I4)
                info.legacyState = v.lVal;
            VariantClear(&v);
        }

        if (!info.automationId.empty() && !IsRectEmpty(&info.rect))
        {
            const std::wstring& id = info.automationId;
            if (id == L"MainIcon" || id == L"MainInstruction" ||
                id == L"ContentText" || id == L"ExpandedFooterText" ||
                id == L"ExpandoButton" || id == L"VerificationCheckBox" ||
                id == L"FootnoteText" || id == L"FootnoteIcon" ||
                id.find(L"RadioButton_") == 0 ||
                id.find(L"CommandLink_") == 0 ||
                id.find(L"CommandButton_") == 0)
            {
                out.push_back(std::move(info));
            }
        }

        CComPtr<IUIAutomationElement> pNext;
        pWalker->GetNextSiblingElement(pChild, &pNext);
        pChild = pNext;
    }
    return !out.empty();
}

static void RefreshElements(HWND hwnd, DirectUIState& s)
{
    s.elements.clear();
    QueryElements(hwnd, s.elements);

    for (const auto& el : s.elements)
        if (el.automationId == L"VerificationCheckBox")
            s.isChecked = (el.legacyState & STATE_SYSTEM_CHECKED) != 0;
    if (s.defChecked) s.isChecked = true;

    s.elemsOk = true;
}

static int HitTest(const std::vector<UIAElementInfo>& els, POINT pt)
{
    for (int i = 0; i < (int)els.size(); ++i)
        if (PtInRect(&els[i].rect, pt)) return i;
    return -1;
}

// ─── Pixel-swap pass ──────────────────────────────────────────────────────────
// Replace exact light-mode background colours with dark equivalents in the
// BPBF_TOPDOWNDIB surface produced by WM_PRINTCLIENT.

struct SwapRule { BYTE sR, sG, sB, dR, dG, dB; };

static void PixelSwap(RGBQUAD* px, int rw, int w, int h,
    const SwapRule* rules, int n)
{
    for (int y = 0; y < h; ++y)
    {
        RGBQUAD* row = px + (y * rw);
        for (int x = 0; x < w; ++x)
        {
            RGBQUAD& p = row[x];
            for (int r = 0; r < n; ++r)
            {
                if (p.rgbRed == rules[r].sR &&
                    p.rgbGreen == rules[r].sG &&
                    p.rgbBlue == rules[r].sB)
                {
                    p.rgbRed = rules[r].dR;
                    p.rgbGreen = rules[r].dG;
                    p.rgbBlue = rules[r].dB;
                    p.rgbReserved = 55;
                    break;
                }
            }
        }
    }

}

// ─── Icon helper ──────────────────────────────────────────────────────────────
static HICON ResolveIcon(const TASKDIALOGCONFIG* cfg, bool isMain)
{
    if (isMain)
    {
        if (cfg->dwFlags & TDF_USE_HICON_MAIN)   return cfg->hMainIcon;
    }
    else
    {
        if (cfg->dwFlags & TDF_USE_HICON_FOOTER) return cfg->hFooterIcon;
    }

    LPCWSTR res = isMain ? cfg->pszMainIcon : cfg->pszFooterIcon;
    if (!res || !IS_INTRESOURCE(res)) return nullptr;

    auto Stock = [](SHSTOCKICONID id) -> HICON {
        SHSTOCKICONINFO sii = { sizeof(sii) };
        return SUCCEEDED(SHGetStockIconInfo(id, SHGSI_ICON | SHGSI_LARGEICON, &sii))
            ? sii.hIcon : nullptr;
        };

    INT_PTR id = (INT_PTR)res;
    if (id == (INT_PTR)TD_WARNING_ICON)
        return Stock(SIID_WARNING);

    if (id == (INT_PTR)TD_ERROR_ICON)
        return Stock(SIID_ERROR);

    if (id == (INT_PTR)TD_INFORMATION_ICON)
        return Stock(SIID_INFO);

    if (id == (INT_PTR)TD_SHIELD_ICON)
        return Stock(SIID_SHIELD);

    if (cfg->hInstance)
    {
        return (HICON)LoadImage(cfg->hInstance, res, IMAGE_ICON,
            GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
            LR_DEFAULTCOLOR);
    }

    return nullptr;
}

// ─── PaintDirectUI ────────────────────────────────────────────────────────────
// Only called from DirectUISubclassProc WM_PAINT, which is only attached when
// dark — no dark/light guard needed here.
//
// Layer order:
//   1. Native render    — WM_PRINTCLIENT into BPBF_TOPDOWNDIB
//   2. Pixel swap       — replace light panel fills with dark equivalents 
//   3. Icon overdraw    — FillRect(dark panel bg) + DrawIconEx
//   4. Glyph overdraw   — FillRect(dark panel bg) + DrawThemeBackground
//   5. Text overdraw    — FillRect(dark panel bg) + DrawThemeTextEx / DrawThemeText

static void PaintDirectUI(HWND hwnd, HDC hdcWin, DirectUIState& s)
{
    if (!s.themesOk) RefreshThemes(hwnd, s);
    EnsureBrushes(s);

    RECT rc; GetClientRect(hwnd, &rc);

    HDC hdcBuf = hdcWin;
    HPAINTBUFFER hbp = BeginBufferedPaint(hdcWin, &rc, BPBF_TOPDOWNDIB, nullptr, &hdcBuf);
    if (!hbp)
    {
        DefSubclassProc(hwnd, WM_PRINTCLIENT, (WPARAM)hdcWin, PRF_CLIENT);
        return;
    }

    // ── 1. Native render ─────────────────────────────────────────────────────
    DefSubclassProc(hwnd, WM_PRINTCLIENT, (WPARAM)hdcBuf, PRF_CLIENT);

    // ── 2. Pixel swap (panel backgrounds) ────────────────────────────────────
    COLORREF bgPri = RGB(255, 255, 255);
    COLORREF bgSec = RGB(240, 240, 240);
    COLORREF bgFtn = RGB(240, 240, 240);
    {
        HTHEME hL = OpenThemeData(nullptr, L"TaskDialog");
        if (hL)
        {
            GetThemeColor(hL, TDLG_PRIMARYPANEL, 0, TMT_FILLCOLOR, &bgPri);
            GetThemeColor(hL, TDLG_SECONDARYPANEL, 0, TMT_FILLCOLOR, &bgSec);
            GetThemeColor(hL, TDLG_FOOTNOTEPANE, 0, TMT_FILLCOLOR, &bgFtn);
            CloseThemeData(hL);
        }
    }
    if (!g_hasNativeTheme)
    {
        RGBQUAD* pPx = nullptr; int rw = 0;
        if (SUCCEEDED(GetBufferedPaintBits(hbp, &pPx, &rw)))
        {
             RECT rcBuf = {};
            GetBufferedPaintTargetRect(hbp, &rcBuf);
            int w = rcBuf.right - rcBuf.left, h = rcBuf.bottom - rcBuf.top;

            const SwapRule rules[] = {
                { GetRValue(bgPri), GetGValue(bgPri), GetBValue(bgPri),
                  GetRValue(DarkColors::kPrimary),   GetGValue(DarkColors::kPrimary),   GetBValue(DarkColors::kPrimary)   },
                { GetRValue(bgSec), GetGValue(bgSec), GetBValue(bgSec),
                  GetRValue(DarkColors::kSecondary), GetGValue(DarkColors::kSecondary), GetBValue(DarkColors::kSecondary) },
                { 128, 128, 128,
                  GetRValue(DarkColors::kSeparator), GetGValue(DarkColors::kSeparator), GetBValue(DarkColors::kSeparator) },
                { 223, 223, 223,
                  GetRValue(DarkColors::kSeparator), GetGValue(DarkColors::kSeparator), GetBValue(DarkColors::kSeparator) },
            };
            PixelSwap(pPx, rw, w, h, rules, ARRAYSIZE(rules));
          
        }
    }

    // ── 3. Icon overdraw ─────────────────────────────────────────────────────
    if (s.pCfg)
    {
        for (const auto& el : s.elements)
        {
            if (IsRectEmpty(&el.rect)) continue;

            HICON  hIcon = nullptr;
            HBRUSH brBg = nullptr;

            if (el.automationId == L"MainIcon")
            {
                hIcon = ResolveIcon(s.pCfg, true);
                brBg = s.brPrimary;
            }
            else if (el.automationId == L"FootnoteIcon")
            {
                hIcon = ResolveIcon(s.pCfg, false);
                brBg = s.brFootnote;
            }

            if (!hIcon || !brBg) continue;

            if (!g_hasNativeTheme)
            {
                FillRect(hdcBuf, &el.rect, brBg);
                DrawIconEx(hdcBuf,
                    el.rect.left, el.rect.top,
                    hIcon,
                    el.rect.right - el.rect.left,
                    el.rect.bottom - el.rect.top,
                    0, nullptr, DI_NORMAL);
            }
        }
    }

    // ── 4. Glyph overdraw ────────────────────────────────────────────────────
    if (s.hTD || s.hButton)
    {
        for (int i = 0; i < (int)s.elements.size(); ++i)
        {
            const UIAElementInfo& el = s.elements[i];
            if (IsRectEmpty(&el.rect)) continue;

            const bool hot = (i == s.hotIdx);
            const bool press = hot && s.pressing;

            if (el.automationId == L"ExpandoButton" && s.hTD)
            {
                SIZE sz = {};
                GetThemePartSize(s.hTD, hdcBuf, TDLG_EXPANDOBUTTON,
                    TDLGEBS_NORMAL, nullptr, TS_TRUE, &sz);
                RECT rcGlyph = el.rect;
                rcGlyph.right = el.rect.left + sz.cx + 3;

                int st;
                if (press && s.isExpanded) st = TDLGEBS_EXPANDEDPRESSED;
                else if (press)                 st = TDLGEBS_PRESSED;
                else if (hot && s.isExpanded) st = TDLGEBS_EXPANDEDHOVER;
                else if (hot)                   st = TDLGEBS_HOVER;
                else if (s.isExpanded)          st = TDLGEBS_EXPANDEDNORMAL;
                else                            st = TDLGEBS_NORMAL;

                if (!g_hasNativeTheme)
                {
                    FillRect(hdcBuf, &rcGlyph, s.brSecondary);
                    DrawThemeBackground(s.hTD, hdcBuf, TDLG_EXPANDOBUTTON,
                        st, &rcGlyph, &el.rect);
                }
            }
            else if (el.automationId == L"VerificationCheckBox" && s.hButton)
            {
                SIZE cs = {};
                GetThemePartSize(s.hButton, hdcBuf, BP_CHECKBOX,
                    CBS_UNCHECKEDNORMAL, nullptr, TS_DRAW, &cs);
                const int mg = (el.rect.bottom - el.rect.top - cs.cy) / 3;
                RECT rcGlyph = {
                    el.rect.left + mg + 1,
                    el.rect.top + mg + 1,
                    el.rect.left + mg + 1 + cs.cx,
                    el.rect.bottom
                };

                int st;
                if (press && s.isChecked) st = CBS_CHECKEDPRESSED;
                else if (press)                st = CBS_UNCHECKEDPRESSED;
                else if (hot && s.isChecked) st = CBS_CHECKEDHOT;
                else if (hot)                  st = CBS_UNCHECKEDHOT;
                else if (s.isChecked)          st = CBS_CHECKEDNORMAL;
                else                           st = CBS_UNCHECKEDNORMAL;

                if (!g_hasNativeTheme)
                {
                    FillRect(hdcBuf, &rcGlyph, s.brSecondary);
                    DrawThemeBackground(s.hButton, hdcBuf, BP_CHECKBOX,
                        st, &rcGlyph, nullptr);
                }
            }
        }
    }

    // ── 5. Text overdraw ─────────────────────────────────────────────────────
    if (s.hTDS || s.hTD)
    {
        HTHEME hThm = s.hTDS ? s.hTDS : s.hTD;

        for (const auto& el : s.elements)
        {
            if (IsRectEmpty(&el.rect)) continue;

            RECT   rcText = el.rect;
            int    uiPart = 0;
            HBRUSH brBg = s.brPrimary;
            DWORD  dtFlags = DT_LEFT | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX;

            if (el.automationId == L"MainInstruction")
            {
                uiPart = TDLG_MAININSTRUCTIONPANE;
                brBg = s.brPrimary;
            }
            else if (el.automationId == L"ContentText")
            {
                uiPart = TDLG_CONTENTPANE;
                brBg = s.brPrimary;
            }
            else if (el.automationId == L"ExpandedFooterText")
            {
                uiPart = TDLG_EXPANDEDFOOTERAREA;
                brBg = s.brFootnote;
            }
            else if (el.automationId == L"FootnoteText")
            {
                uiPart = TDLG_FOOTNOTEPANE;
                brBg = s.brFootnote;
            }
            else if (el.automationId == L"ExpandoButton" && s.hTD)
            {
                SIZE sz = {};
                GetThemePartSize(s.hTD, hdcBuf, TDLG_EXPANDOBUTTON,
                    TDLGEBS_NORMAL, nullptr, TS_TRUE, &sz);
                MARGINS vtextMargins = {};
                GetThemeMargins(s.hTD, hdcBuf, TDLG_VERIFICATIONTEXT, 0,
                    TMT_CONTENTMARGINS, &el.rect, &vtextMargins);
                rcText.left += sz.cx + vtextMargins.cxLeftWidth - 2;
                rcText.top += 1;
                uiPart = TDLG_EXPANDOTEXT;
                brBg = s.brSecondary;
                dtFlags = DT_LEFT | DT_VCENTER | DT_NOPREFIX;
            }
            else if (el.automationId == L"VerificationCheckBox" && s.hButton && s.hTD)
            {
                SIZE cs = {};
                GetThemePartSize(s.hButton, hdcBuf, BP_CHECKBOX,
                    CBS_UNCHECKEDNORMAL, nullptr, TS_DRAW, &cs);
                MARGINS textMargins = {};
                GetThemeMargins(s.hTD, hdcBuf, TDLG_VERIFICATIONTEXT, 0,
                    TMT_CONTENTMARGINS, &el.rect, &textMargins);
                rcText.left = el.rect.left + cs.cx + textMargins.cxLeftWidth + 3;
                rcText.top += 5;
                uiPart = TDLG_VERIFICATIONTEXT;
                brBg = s.brSecondary;
                dtFlags = DT_LEFT | DT_VCENTER | DT_NOPREFIX;
            }

            if (uiPart == 0) continue;

            DTTOPTS opts = { sizeof(opts) };
            opts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
            opts.crText = GetTextColor(s, uiPart);

            if (!g_hasNativeTheme)
            {
                FillRect(hdcBuf, &rcText, brBg);
                DrawThemeTextEx(hThm, hdcBuf, uiPart, 0,
                    el.name.c_str(), -1, dtFlags, &rcText, &opts);
            }
            else
            {
                DrawThemeText(hThm, hdcBuf, uiPart, 0,
                    el.name.c_str(), -1, dtFlags, NULL, &rcText);
            }
        }
    }

    EndBufferedPaint(hbp, TRUE);
}

// ─── DirectUISubclassProc ─────────────────────────────────────────────────────
// Only attached when dark (subclass presence = dark invariant).
// WM_ERASEBKGND and WM_PAINT fire unconditionally — no IsActive() check needed.
//

static LRESULT CALLBACK DirectUISubclassProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uId, DWORD_PTR refData)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        // Subclass presence = dark → always suppress; WM_PAINT owns the surface.
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DirectUIState& s = GetState(hwnd);
        // TODO: move to UIA element cache instead of RefreshElements every paint
        s.isExpanded = (bool)(GetProp(GetParent(hwnd), L"IsExpanded"));
        RefreshElements(hwnd, s);
        PaintDirectUI(hwnd, hdc, s);
        EndPaint(hwnd, &ps);
        return 0;
    }

    // ── Mouse tracking ────────────────────────────────────────────────────────
    case WM_MOUSEMOVE:
    {
        DirectUIState& s = GetState(hwnd);
        if (!s.tracking)
        {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);
            s.tracking = true;
        }
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        int newHot = HitTest(s.elements, pt);
        if (newHot != s.hotIdx) { s.hotIdx = newHot; InvalidateRect(hwnd, nullptr, FALSE); }
        break;
    }
    case WM_MOUSELEAVE:
    {
        DirectUIState& s = GetState(hwnd);
        s.tracking = false; s.pressing = false;
        if (s.hotIdx != -1) { s.hotIdx = -1; InvalidateRect(hwnd, nullptr, FALSE); }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        DirectUIState& s = GetState(hwnd);
        s.pressing = true;
        RefreshElements(hwnd, s);
        break;
    }
    case WM_LBUTTONUP:
    {
        DirectUIState& s = GetState(hwnd);
        if (s.pressing) s.pressing = false;
        RefreshElements(hwnd, s);
        break;
    }

    case WM_DESTROY:
        DestroyState(hwnd);
        RemoveWindowSubclass(hwnd, DirectUISubclassProc, uId);
        break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ─── WmCtColorSubclassProc ────────────────────────────────────────────────────
// Applied to the unnamed CtrlNotifySink parents of:
//   CCSysLink (FootnoteTextLink)  — kFootnote brush
//   CommandLink_*                 — kPrimary  brush
//   CommandButton_*               — kSecondary brush
//   RadioButton_*                 — kPrimary  brush
//
// dwRefData: HBRUSH owned by this instance; freed in WM_DESTROY.

static LRESULT CALLBACK WmCtColorSubclassProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uId, DWORD_PTR dwRef)
{
    switch (msg)
    {

    case WM_ERASEBKGND:
    {
        if (!dwRef) break;
        HDC hdc = (HDC)wParam;
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)dwRef);
        SetTextColor(hdc, DarkColors::kTextNormal);
        return 1;
    }
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        COLORREF bg = DarkColors::kSecondary;
        if (dwRef)
        {
            LOGBRUSH lb = {};
            GetObject((HBRUSH)dwRef, sizeof(lb), &lb);
            if (lb.lbStyle == BS_SOLID) bg = lb.lbColor;
        }
        SetBkColor(hdc, bg);
        SetTextColor(hdc, DarkColors::kTextNormal);
        HBRUSH br = dwRef ? (HBRUSH)dwRef
            : (HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND);
        return (LRESULT)br;
    }
    case WM_DESTROY:
        if (dwRef) DeleteObject((HBRUSH)dwRef);
        RemoveWindowSubclass(hwnd, WmCtColorSubclassProc, uId);
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ─── RadioSubclassProc ─────────────────────────────────────────────────────────────
// 
// Apply RadioButton TextColor.
// RadioButton_*  kTextNormal if new Windows 11 25 H2 DarkMode_DarkTheme Not Found.
//
static LRESULT CALLBACK RadioSubclassProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uId, DWORD_PTR dwRef)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        HTHEME hTheme = OpenThemeData(NULL, L"TaskDialogStyle");
        int dpi = _GetDpiForWindow(hwnd);
        HTHEME hThemeBtn = _OpenThemeDataForDpi(hwnd, L"BUTTON", dpi);
        if (!hThemeBtn)
            hThemeBtn = OpenThemeData(NULL, L"Button");

        if (!hTheme || !hThemeBtn)
        {
            if (hTheme) CloseThemeData(hTheme);
            if (hThemeBtn) CloseThemeData(hThemeBtn);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DefSubclassProc(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, PRF_CLIENT);
            EndPaint(hwnd, &ps);
            return 0;
        }
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC hdcBuf = hdc;
        HPAINTBUFFER hbp = BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, nullptr, &hdcBuf);
        if (!hbp)
        {
            DefSubclassProc(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, PRF_CLIENT);
            CloseThemeData(hTheme);
            CloseThemeData(hThemeBtn);
            EndPaint(hwnd, &ps);
            return 0;
        }

        DefSubclassProc(hwnd, WM_PRINTCLIENT, (WPARAM)hdcBuf, PRF_CLIENT);
        wchar_t text[256] = {};
        GetWindowTextW(hwnd, text, _countof(text));
        SIZE gsize = {};
        GetThemePartSize(hThemeBtn, hdcBuf, BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL, &rcClient, TS_TRUE, &gsize);
        RECT rcText = { gsize.cx + 2, 0, rcClient.right, rcClient.bottom };
        DTTOPTS dots = {sizeof(dots)};
        dots.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
        dots.crText = DarkColors::kTextNormal;
        LOGFONT logFont = {};
        HRESULT hr = GetThemeFont(hTheme, hdcBuf, TDLG_RADIOBUTTONPANE, 0, TMT_FONT, &logFont);
        HFONT font = SUCCEEDED(hr) ? CreateFontIndirect(&logFont) : nullptr;
        HFONT oldFont = font ? SelectFont(hdcBuf, font) : nullptr;
        DrawThemeTextEx(hTheme, hdcBuf, TDLG_RADIOBUTTONPANE, 0, text, -1, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS, &rcText, &dots);
        CloseThemeData(hTheme);
        CloseThemeData(hThemeBtn);
        if (oldFont) SelectFont(hdcBuf, oldFont);
        if (font)    DeleteFont(font);
        EndBufferedPaint(hbp, TRUE);
        EndPaint(hwnd, &ps);
        return 0;
    }
    
    case WM_DESTROY:
        if (dwRef) DeleteObject((HBRUSH)dwRef);
        RemoveWindowSubclass(hwnd, RadioSubclassProc, uId);
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ─── TaskDialogMainSubclassProc ───────────────────────────────────────────────
// Only attached when dark (subclass presence = dark invariant).
//
// WM_CTLCOLORDLG:
// For collapsed,expanded Background erease during resize
//
// WM_SETTINGCHANGE
//   Just redraw. This subclass has no access to the per-dialog Theme — the
//   DirectUISubclassProc owns the teardown decision for Theme::System.
// 
// WM_DESTROY:
//   RemoveFromTaskDialog cleans up all dark subclasses.

static LRESULT CALLBACK TaskDialogMainSubclassProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uId, DWORD_PTR dwRef)
{
    switch (msg)
    {
    case WM_CTLCOLORDLG:
    {
        // collapsed, expanded Back Color 
        if (ResolveDark(g_theme))
        {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, DarkColors::kSecondary);
            SetTextColor(hdc, DarkColors::kTextNormal);
            static HBRUSH sBr = CreateSolidBrush(DarkColors::kSecondary);
            return (LRESULT)sBr;
        }
       
    }
    break;

    case WM_SETTINGCHANGE:
    {
        if (g_theme == TaskDialogTheme::Theme::System)
        {
            TaskDialogTheme::AllowForTaskDialog(hwnd, (TASKDIALOGCONFIG*)dwRef, g_theme);
        }
    }
        break;
    case WM_DESTROY:
        TaskDialogTheme::RemoveFromTaskDialog(hwnd);
        RemoveWindowSubclass(hwnd, TaskDialogMainSubclassProc, uId);
        break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ─── TaskDialogTheme namespace ───────────────────────────────────────────────────────

namespace TaskDialogTheme
{

   
   
    // ── IsActive ──────────────────────────────────────────────────────────────
    
    bool IsActive()
    {
        return ResolveDark(g_theme);
    }

    bool HasNativeTaskDialogTheme() { return g_hasNativeTheme; }

    // ── EnableForTLW ──────────────────────────────────────────────────────────
    // Sets o Remove  the dark title bar (DWMWA_USE_IMMERSIVE_DARK_MODE) 
    // 

    void EnableForTLW(HWND hwnd,bool dark)
    {
        static HMODULE hDwm = LoadLibraryW(L"dwmapi.dll");
        if (!hDwm) return;
        using pfnDwmSWA = HRESULT(WINAPI*)(HWND, DWORD, const void*, DWORD);
        static auto fn = (pfnDwmSWA)GetProcAddress(hDwm, "DwmSetWindowAttribute");
        if (fn)
        {
            BOOL use = dark;
            if (FAILED(fn(hwnd, 20, &use, sizeof(use)))) // DWMWA_USE_IMMERSIVE_DARK_MODE
                fn(hwnd, 19, &use, sizeof(use));          // pre-20348 fallback
        }
      
    }

    // ── AllowForWindow ────────────────────────────────────────────────────────
    // Thin wrapper around SetWindowTheme for internal use.
    // Called only from the dark path — no IsActive() guard.

    void AllowForWindow(HWND hwnd, const wchar_t* theme)
    {
        if (theme) SetWindowTheme(hwnd, theme, nullptr);
    }

    // ─── AllowForTaskDialog ───────────────────────────────────────────────────
    //
    // Entry point called from TDN_CREATED and TDN_NAVIGATED.
    // theme defaults to System (backward-compatible with existing call sites).
    //
    // ResolveDark(theme) decides the branch:
    //
    //   dark == false  → REMOVE path
    //     Strip all dark subclasses so the dialog reverts to native light
    //     rendering. Handles TDN_NAVIGATED dark→light transitions cleanly.
    //
    //   dark == true   → ATTACH path (existing behaviour)
    //     Walk child windows via UIA, attach subclasses, apply window themes.
    //     Subclass presence IS the dark flag — no SetProp / GetProp needed.

    void AllowForTaskDialog(HWND hwndTD, TASKDIALOGCONFIG* pCfg, TaskDialogTheme::Theme theme /*= Theme::System*/)
    {
        // Comment this to use Pixel-swap pass
        g_hasNativeTheme =
            IsDarkThemeActive(L"DarkMode_Explorer::TaskDialog", L"TaskDialog") ||
            IsDarkThemeActive(L"DarkMode_DarkTheme::TaskDialog", L"TaskDialog");

        g_theme = theme;
        const bool dark = ResolveDark(theme);

        // ── Remove path ───────────────────────────────────────────────────────
        if (!dark)
        {

            SetWindowTheme(hwndTD, NULL, NULL);
           
            EnumChildWindows(hwndTD, [](HWND hwndChild, LPARAM) -> BOOL
                {
                  
                    DWORD_PTR ex = 0;
                    if (GetWindowSubclass(hwndChild, DirectUISubclassProc, kDirectUISubclassId, &ex))
                    {
                        EnumChildWindows(hwndChild, [](HWND hwndDuiChild, LPARAM) -> BOOL
                            {

                                SetWindowTheme(hwndDuiChild, NULL, NULL);
                                SendMessage(hwndDuiChild, WM_SYSCOLORCHANGE, 0, 0);
                                DWORD_PTR ex1 = 0;
                                if (GetWindowSubclass(hwndDuiChild, WmCtColorSubclassProc, kCtlColorId, &ex1))
                                {
                                    if (ex1) DeleteObject((HBRUSH)ex1);
                                    RemoveWindowSubclass(hwndDuiChild, WmCtColorSubclassProc, kCtlColorId);
                                }
                                return TRUE;
                            }, 0);
                        SetWindowTheme(hwndChild, NULL, NULL);
                        RemoveWindowSubclass(hwndChild, DirectUISubclassProc, kDirectUISubclassId);
                        DestroyState(hwndChild);
                    }
                    if (GetWindowSubclass(hwndChild, RadioSubclassProc, kCtlColorId, &ex))
                    {
                        if (ex) DeleteObject((HBRUSH)ex);
                        RemoveWindowSubclass(hwndChild, RadioSubclassProc, kCtlColorId);
                    }
                    if (GetWindowSubclass(hwndChild, WmCtColorSubclassProc, kCtlColorId, &ex))
                    {
                        if (ex) DeleteObject((HBRUSH)ex);
                        RemoveWindowSubclass(hwndChild, WmCtColorSubclassProc, kCtlColorId);
                    }
                   
                    RedrawWindow(hwndChild, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                    return TRUE;
                }, 0);
            

            DWORD_PTR existing = 0;
            if (!GetWindowSubclass(hwndTD, TaskDialogMainSubclassProc, kMainSubclassId, &existing))
                SetWindowSubclass(hwndTD, TaskDialogMainSubclassProc,
                    kMainSubclassId, (DWORD_PTR)pCfg);
            EnableForTLW(hwndTD,FALSE);
            return;
        }

      

        struct EnumData
        {
            HWND hwndTD;
            bool found;
            TASKDIALOGCONFIG* pCfg;
            TaskDialogTheme::Theme theme;
        };
        EnumData data = { hwndTD, false, pCfg, theme };

        EnumChildWindows(hwndTD,
            [](HWND hwndChild, LPARAM lp) -> BOOL
            {
                EnumData* d = reinterpret_cast<EnumData*>(lp);

                HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                bool    comInit = SUCCEEDED(hr);

                CComPtr<IUIAutomation> pAuto;
                hr = pAuto.CoCreateInstance(__uuidof(CUIAutomation));
                if (FAILED(hr)) { if (comInit) CoUninitialize(); return TRUE; }

                CComPtr<IUIAutomationElement> pEl;
                if (FAILED(pAuto->ElementFromHandle(hwndChild, &pEl)))
                {
                    if (comInit) CoUninitialize(); return TRUE;
                }

                CComBSTR cls;
                pEl->get_CurrentClassName(&cls);

                // ── CCSysLink — footnote / content hyperlinks ─────────────────
                if (cls == L"CCSysLink")
                {
                    HWND hLink = nullptr;
                    if (SUCCEEDED(pEl->get_CurrentNativeWindowHandle((UIA_HWND*)&hLink)) && hLink)
                    {
                        HWND hParent = GetParent(hLink);
                        if (hParent)
                        {
                            CComBSTR bId;
                            pEl->get_CurrentAutomationId(&bId);
                            const std::wstring id = bId ? (LPCWSTR)bId : L"";

                            const bool isFootnote =
                                (id == L"FootnoteTextLink" ||
                                    id == L"ExpandedFooterTextLink" ||
                                    id.find(L"Footnote") != std::wstring::npos ||
                                    id.find(L"ExpandedFooter") != std::wstring::npos);

                            COLORREF bg = isFootnote && !g_hasNativeTheme
                                ? DarkColors::kFootnote : DarkColors::kPrimary;

                            if (id == L"ContentLink")
                                bg = g_hasNativeTheme ? DarkColors::kFootnote : DarkColors::kPrimary;

                         

                            DWORD_PTR ex = 0;
                            if (!GetWindowSubclass(hParent, WmCtColorSubclassProc, kCtlColorId, &ex))
                                SetWindowSubclass(hParent, WmCtColorSubclassProc,
                                    kCtlColorId, (DWORD_PTR)CreateSolidBrush(bg));
                        }
                    }
                    if (comInit) CoUninitialize();
                    return TRUE;
                }

                // ── "TaskDialog" — the DirectUI TaskPage window ───────────────
                if (cls != L"TaskDialog")
                {
                    if (comInit) CoUninitialize(); return TRUE;
                }

                HWND hDUI = nullptr;
                if (FAILED(pEl->get_CurrentNativeWindowHandle((UIA_HWND*)&hDUI)) || !hDUI)
                {
                    if (comInit) CoUninitialize(); return TRUE;
                }

                // Class background brush
                {
                    HBRUSH nb = CreateSolidBrush(DarkColors::kSecondary);
                    HBRUSH ob = (HBRUSH)SetClassLongPtr(
                        hDUI, GCLP_HBRBACKGROUND, (LONG_PTR)nb);
                    if (ob &&
                        ob != GetSysColorBrush(COLOR_WINDOW) &&
                        ob != GetSysColorBrush(COLOR_BTNFACE))
                        DeleteObject(ob);
                }

                // ── Walk TaskPage UIA children ────────────────────────────────
                CComPtr<IUIAutomationTreeWalker> pWalker;
                pAuto->get_ContentViewWalker(&pWalker);
                CComPtr<IUIAutomationElement> pChild;
                pWalker->GetFirstChildElement(pEl, &pChild);

                while (pChild)
                {
                    CONTROLTYPEID ct = 0;
                    pChild->get_CurrentControlType(&ct);

                    if (ct == UIA_ButtonControlTypeId ||
                        ct == UIA_RadioButtonControlTypeId ||
                        ct == UIA_ProgressBarControlTypeId ||
                        ct == UIA_HyperlinkControlTypeId ||
                        ct == UIA_ScrollBarControlTypeId || ct ==  UIA_PaneControlTypeId)
                    {
                        HWND hBtn = nullptr;
                        if (SUCCEEDED(pChild->get_CurrentNativeWindowHandle(
                            (UIA_HWND*)&hBtn)) && hBtn)
                        {
                            BSTR bstrClass = nullptr, bstrId = nullptr, bstrName = nullptr;
                            pChild->get_CurrentClassName(&bstrClass);
                            pChild->get_CurrentAutomationId(&bstrId);
                            pChild->get_CurrentName(&bstrName);

                            const std::wstring id = bstrId ? bstrId : L"";
                            const std::wstring name = bstrName ? bstrName : L"";

#ifdef _DEBUG
                            {
                                wchar_t dbg[512];
                                swprintf_s(dbg, L"[DarkMode] ControlTypeId=%lu AutomationId=%-24s Text=%s ClassName=%s\n",
                                    ct, id.c_str(), name.c_str(),
                                    bstrClass ? bstrClass : L"");
                                OutputDebugStringW(dbg);
                            }
#endif
                            HWND hP = GetParent(hBtn);

                            if (ct == UIA_ProgressBarControlTypeId)
                            {
                                const bool hasCopyEngine =
                                    IsDarkThemeActive(L"DarkMode_CopyEngine::Progress", L"Progress");
                                AllowForWindow(hBtn, hasCopyEngine
                                    ? L"DarkMode_CopyEngine" : L"DarkMode_Explorer");
                            }
                            else if (ct == UIA_RadioButtonControlTypeId ||
                                id.find(L"RadioButton_") == 0 ||
                                ct == UIA_HyperlinkControlTypeId)
                            {
                                DWORD_PTR ex = 0;
                                // Subclass for RadioButton text foreground when new Windows 11 25H2 DarkMode_DarkTheme is not available.
                                const bool hasDarkTheme = IsDarkThemeActive(L"DarkMode_DarkTheme::TaskDialog", L"TaskDialog");
                                if (hasDarkTheme)
                                {
                                    AllowForWindow(hBtn, L"DarkMode_DarkTheme");
                                }
                                else
                                {
                                    if (hBtn && !GetWindowSubclass(hBtn, RadioSubclassProc, kCtlColorId, &ex))
                                        SetWindowSubclass(hBtn, RadioSubclassProc, kCtlColorId,
                                            (DWORD_PTR)CreateSolidBrush(
                                                g_hasNativeTheme
                                                ? DarkColors::kSecondary
                                                : DarkColors::kPrimary));
                                }
                         
                                if (hP && !GetWindowSubclass(hP, WmCtColorSubclassProc, kCtlColorId, &ex))
                                    SetWindowSubclass(hP, WmCtColorSubclassProc, kCtlColorId,
                                        (DWORD_PTR)CreateSolidBrush(
                                            g_hasNativeTheme
                                            ? DarkColors::kSecondary
                                            : DarkColors::kPrimary));
                               

                            }
                            else if (id.find(L"CommandLink_") == 0)
                            {
                                AllowForWindow(hBtn, L"DarkMode_Explorer");
                                DWORD_PTR ex = 0;
                                if (hP && !GetWindowSubclass(hP, WmCtColorSubclassProc, kCtlColorId, &ex))
                                    SetWindowSubclass(hP, WmCtColorSubclassProc, kCtlColorId,
                                        (DWORD_PTR)CreateSolidBrush(
                                            g_hasNativeTheme
                                            ? DarkColors::kSecondary
                                            : DarkColors::kPrimary));
                            }
                            else if (id.find(L"CommandButton_") == 0)
                            {
                                AllowForWindow(hBtn, L"DarkMode_Explorer");
                                DWORD_PTR ex = 0;
                                if (hP && !GetWindowSubclass(hP, WmCtColorSubclassProc, kCtlColorId, &ex))
                                    SetWindowSubclass(hP, WmCtColorSubclassProc, kCtlColorId,
                                        (DWORD_PTR)CreateSolidBrush(DarkColors::kSecondary));
                            }
                            else
                            {
                                AllowForWindow(hBtn, L"DarkMode_Explorer");
                            }

                            if (bstrClass) SysFreeString(bstrClass);
                            if (bstrId)    SysFreeString(bstrId);
                            if (bstrName)  SysFreeString(bstrName);
                        }
                    }

                    CComPtr<IUIAutomationElement> pNext;
                    pWalker->GetNextSiblingElement(pChild, &pNext);
                    pChild = pNext;
                }

                // Window theme for TaskPage — must be set after children.
                TaskDialogTheme::AllowForWindow(hDUI, L"DarkMode_Explorer");

                // ── Store state and attach DirectUI subclass (idempotent) ──────
                {
                    DirectUIState& s = GetState(hDUI);
                    s.pCfg = d->pCfg;
                    s.theme = d->theme; // store per-dialog theme for message handlers

                    s.defExpanded = d->pCfg
                        ? (d->pCfg->dwFlags & TDF_EXPANDED_BY_DEFAULT) != 0 : false;
                    s.defChecked = d->pCfg
                        ? (d->pCfg->dwFlags & TDF_VERIFICATION_FLAG_CHECKED) != 0 : false;

                    s.isExpanded = (bool)(LONG_PTR)GetProp(d->hwndTD, L"IsExpanded");
                    s.isChecked = (bool)(LONG_PTR)GetProp(d->hwndTD, L"IsChecked");
                    if (s.defChecked) s.isChecked = true;

                    s.elemsOk = false;
                    RefreshElements(hDUI, s);

                    DWORD_PTR ex = 0;
                    if (!GetWindowSubclass(hDUI, DirectUISubclassProc, kDirectUISubclassId, &ex))
                        SetWindowSubclass(hDUI, DirectUISubclassProc,
                            kDirectUISubclassId, (DWORD_PTR)d->pCfg);
                   
                }

                d->found = true;
                if (comInit) CoUninitialize();
                return TRUE; // keep enumerating (CCSysLink may follow)
            },
            (LPARAM)&data);

        if (data.found)
        {
            // On Win11 apply theme to the outer dialog after TaskPage so all
            // child windows inherit the correct DarkMode_Explorer background heritage.
            if (g_hasNativeTheme)
            {
                AllowForWindow(hwndTD, L"DarkMode_Explorer");
            }
          
            EnableForTLW(hwndTD);
            // ── Attach path ───────────────────────────────────────────────────────
            // Attach outer subclass (WM_CTLCOLORDLG, WM_SETTINGCHANGE, WM_DESTROY).
            DWORD_PTR existing = 0;
            if (!GetWindowSubclass(hwndTD, TaskDialogMainSubclassProc, kMainSubclassId, &existing))
                SetWindowSubclass(hwndTD, TaskDialogMainSubclassProc,
                    kMainSubclassId, (DWORD_PTR)pCfg);

            // Full RedrawWindow The Window and Children.
            EnumChildWindows(hwndTD, [](HWND hwndDuiChild, LPARAM) -> BOOL
                {

                    SendMessage(hwndDuiChild, WM_SYSCOLORCHANGE, 0, 0);
                    return TRUE;
                }, 0);
           SendMessage(hwndTD, WM_THEMECHANGED, 0, 0);
        }
    }

    // ── RemoveFromTaskDialog ──────────────────────────────────────────────────
    // Strips all dark subclasses and frees per-dialog DirectUIState.
    // Called from:
    //   • TDN_DESTROYED callback
    //   • TaskDialogMainSubclassProc WM_DESTROY
    //   • AllowForTaskDialog remove path (Theme::Light or Theme::System→light)

    void RemoveFromTaskDialog(HWND hwndTD)
    {
        RemoveWindowSubclass(hwndTD, TaskDialogMainSubclassProc, kMainSubclassId);
        EnumChildWindows(hwndTD, [](HWND hwndChild, LPARAM) -> BOOL
            {
                DWORD_PTR ex = 0;
                if (GetWindowSubclass(hwndChild, DirectUISubclassProc, kDirectUISubclassId, &ex))
                {
                    RemoveWindowSubclass(hwndChild, DirectUISubclassProc, kDirectUISubclassId);
                    DestroyState(hwndChild);
                }
                if (GetWindowSubclass(hwndChild, WmCtColorSubclassProc, kCtlColorId, &ex))
                {
                    if (ex) DeleteObject((HBRUSH)ex);
                    RemoveWindowSubclass(hwndChild, WmCtColorSubclassProc, kCtlColorId);
                }
                SetWindowTheme(hwndChild, NULL, NULL);
                return TRUE;
            }, 0);
     
    }

} // namespace TaskDialogTheme
