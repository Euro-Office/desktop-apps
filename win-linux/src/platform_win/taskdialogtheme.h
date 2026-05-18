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
// TaskDialogTheme.h  —  Win32 TaskDialog dark-mode support (public API)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <windows.h>
#include <commctrl.h>

// ─── TaskDialogTheme API ─────────────────────────────────────────────────────────────

namespace TaskDialogTheme
{
    // Per-dialog theme override passed to AllowForTaskDialog.
    enum class Theme
    {
        System, // follow OS dark-mode preference  (default — existing behaviour)
        Dark,   // force dark regardless of OS setting
        Light,  // force light regardless of OS setting
    };

   
    // True when dark mode is active.
     bool IsActive();

    // True on Win11 builds where DarkMode_TaskDialog UxTheme classes exist.
    bool HasNativeTaskDialogTheme();

    // Sets DWMWA_USE_IMMERSIVE_DARK_MODE and DarkMode_Explorer window theme
    // on a top-level window (dark title bar + chrome).
    void EnableForTLW(HWND hwnd, bool dark = true);

    // Applies SetWindowTheme to any child control.
    // Called only from the dark path — no IsActive() guard.
    void AllowForWindow(HWND hwnd, const wchar_t* themeClass = nullptr);

    // Main entry point. Call from TDN_CREATED and TDN_NAVIGATED.
    //
    // theme defaults to System (fully backward-compatible).
    //
    //   Theme::System  — resolves to dark/light via IsActive() at call time.
    //   Theme::Dark    — forces dark for this dialog regardless of OS setting.
    //   Theme::Light   — forces light: removes all dark subclasses so the dialog
    //                    uses native light rendering.
    //
    // Subclass presence encodes the dark/light state for this dialog:
    //   subclasses attached    → dialog is dark
    //   subclasses not present → dialog is light
    //
    // TDN_NAVIGATED: call again with the same theme to re-apply after page nav.
    // Dark→light transition: call with Theme::Light (or Theme::System when OS
    //   is light) — all dark subclasses are removed automatically.
    void AllowForTaskDialog(HWND hwndTaskDialog, TASKDIALOGCONFIG* pConfig, Theme theme = Theme::System);

    // Call from TDN_DESTROYED to free per-dialog state.
    // Also called automatically from TaskDialogMainSubclassProc WM_DESTROY.
    void RemoveFromTaskDialog(HWND hwnd);
}
