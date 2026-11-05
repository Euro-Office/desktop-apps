# SPDX-FileCopyrightText: 2026 Euro-Office contributors
# SPDX-License-Identifier: AGPL-3.0-only

<#
.SYNOPSIS
Configures and builds the desktop editors on Windows (win-linux\) with MSVC
(Visual Studio 2022) and Ninja. DesktopEditors builds through this script.
Linux and macOS: build.sh.

.DESCRIPTION
Core isn't built as part of this project: it links a core package. That is
the prebuilt one for the core checkout when the build cache has it, else one
built by core\scripts\build.ps1. A core checkout with local changes is always
built (see core\scripts\core_package.py).

The desktop build is in <BuildDir>\desktop, a core built here in
<BuildDir>\core, core's third-party libraries in -ThirdPartyDir (default
<BuildDir>\third_party). Needs VCPKG_ROOT. CORE_PYTHON sets the Python for
the build scripts (default: python). EO_CORE_CACHE: auto (default), off
(always build core) or require (fail without a prebuilt core). Uses sccache
when it is on PATH; -Ccache builds core through ccache instead. Arguments after
the named ones go to cmake.

.EXAMPLE
scripts\build.ps1 -Arch x64 -BuildDir build "-DABOUT_PAGE_APP_NAME=Euro-Office"
#>
# Not positional: arguments without a name go to cmake
[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('x64', 'arm64')]
    [string]$Arch = $(if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64') { 'arm64' } else { 'x64' }),
    [string]$BuildDir = 'build',
    [string]$Core = '',
    [string]$CorePackage = '',
    [string]$ThirdPartyDir = '',
    [switch]$Tests,
    [switch]$Ccache,
    [string]$SdkVersion = '10.0.19041.0',
    [Parameter(ValueFromRemainingArguments)]
    [string[]]$CMakeArgs = @()
)

$ErrorActionPreference = 'Stop'
$Apps = Split-Path -Parent $PSScriptRoot
if (-not $Core) { $Core = Join-Path $Apps '..\core' }
$Core = (Resolve-Path $Core).Path

function Assert-LastExit([string]$What) {
    if ($LASTEXITCODE -ne 0) { throw "$What failed with exit code $LASTEXITCODE" }
}

. "$Core\scripts\msvc-env.ps1"

if (-not $env:VCPKG_ROOT) { throw 'VCPKG_ROOT must point at vcpkg' }
if (-not $env:VCINSTALLDIR) { Import-VcVars -Arch $Arch -SdkVersion $SdkVersion }

$BuildDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($BuildDir)
$python = if ($env:CORE_PYTHON) { $env:CORE_PYTHON } else { 'python' }
# Shared by the core and the desktop build, so each dependency is set up once
$thirdParty = if ($ThirdPartyDir) { $ThirdPartyDir } else { Join-Path $BuildDir 'third_party' }

if (-not $CorePackage) {
    $prebuilt = Join-Path $BuildDir 'core-prebuilt'
    & $python "$Core\scripts\core_package.py" fetch --core $Core `
        --platform "windows-$Arch" --product desktop --dest $prebuilt
    switch ($LASTEXITCODE) {
        0 { $CorePackage = $prebuilt }
        3 {
            & "$Core\scripts\build.ps1" -Arch $Arch -BuildDir (Join-Path $BuildDir 'core') `
                -SdkVersion $SdkVersion -Ccache:$Ccache "-DEO_CORE_3RD_PARTY_DIR=$thirdParty"
            $CorePackage = Join-Path $BuildDir 'core\package'
        }
        default { throw "core_package.py fetch failed with exit code $LASTEXITCODE" }
    }
}
$CorePackage = (Resolve-Path $CorePackage).Path

$features = if ($Tests) { 'desktop-editors;tests' } else { 'desktop-editors' }
$configureArgs = @(
    '-G', 'Ninja', '-S', "$Apps\win-linux", '-B', "$BuildDir\desktop",
    '-DCMAKE_BUILD_TYPE=Release',
    "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake",
    '-DVCPKG_MANIFEST_MODE=ON',
    "-DVCPKG_MANIFEST_DIR=$Core",
    "-DVCPKG_MANIFEST_FEATURES=$features",
    "-DEO_BUILD_TESTS=$(if ($Tests) { 'ON' } else { 'OFF' })",
    "-DEO_CORE_PREBUILT_DIR=$CorePackage",
    "-DEO_CORE_3RD_PARTY_DIR=$thirdParty"
)
if ($env:CORE_PYTHON) { $configureArgs += "-DPYTHON_BIN=$env:CORE_PYTHON" }
# sccache only caches with debug info embedded in the objects (/Z7)
if (Get-Command sccache -ErrorAction SilentlyContinue) {
    $configureArgs += @(
        '-DCMAKE_C_COMPILER_LAUNCHER=sccache',
        '-DCMAKE_CXX_COMPILER_LAUNCHER=sccache',
        '-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded'
    )
}

cmake @configureArgs @CMakeArgs
Assert-LastExit 'CMake configure'
cmake --build "$BuildDir\desktop" --parallel
Assert-LastExit 'CMake build'
