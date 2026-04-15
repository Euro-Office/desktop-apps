param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$CompanyName = "ONLYOFFICE",
    [string]$ProductName = "DesktopEditors",
    [string]$SourceDir,
    [string]$BuildDir,
    [switch]$Sign
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not $SourceDir) {
    $BuildPrefix = switch ($Arch) {
        "x64" { "win_64" + $(if ($Target -eq "xp") { "_xp" }) }
        "x86" { "win_32" + $(if ($Target -eq "xp") { "_xp" }) }
        "arm64" { "win_arm64" }
    }
    $SourceDir = "$PSScriptRoot\..\..\build_tools\out\" `
        + "$BuildPrefix\$CompanyName\$ProductName" | Resolve-Path
}
if (-not (Test-Path "$SourceDir")) {
    Write-Error "Path `"$SourceDir`" does not exist"
}
if (-not $BuildDir) {
    $BuildDir = "build\$Arch"
}

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
CompanyName = $CompanyName
ProductName = $ProductName
SourceDir   = $SourceDir
BuildDir    = $BuildDir
Sign        = $Sign
"@

####

Write-Host "`n[ Prepare build directory ]"

if (Test-Path "$BuildDir") {
    Write-Host "REMOVE DIR: $BuildDir"
    Remove-Item -Force -Recurse -LiteralPath "$BuildDir"
}

Write-Host "CREATE DIR: $BuildDir\desktop"
New-Item -ItemType Directory -Force -Path "$BuildDir\desktop" | Out-Null

Write-Host "CREATE DIR: $BuildDir\help"
New-Item -ItemType Directory -Force -Path "$BuildDir\help" | Out-Null

Write-Host "COPY: $SourceDir\* > $BuildDir\desktop\"
Copy-Item -Force -Recurse `
    -Path "$SourceDir\*" `
    -Destination "$BuildDir\desktop\"

Write-Host "MOVE: $BuildDir\desktop\editors\web-apps\apps\*\main\resources\help\* > $BuildDir\help\"
Get-ChildItem -Directory `
    -Path "$BuildDir\desktop\editors\web-apps\apps\*\main\resources\help" `
    | ForEach-Object {
        $src = $_.FullName | Resolve-Path -Relative
        $dst = $src.Replace("$BuildDir\desktop", "$BuildDir\help")

        Write-Host "MOVE: $src > $dst"
        New-Item -ItemType Directory -Force -Path "$dst\.." | Out-Null
        Move-Item -Path "$src" -Destination "$dst"
    }

# "$BuildDir\desktop: {0:0.00} MB" -f ((Get-ChildItem -Recurse `
#     -Path "$BuildDir\desktop" | Measure-Object -Property Length -Sum).Sum / 1MB)
# "$BuildDir\help: {0:0.00} MB" -f ((Get-ChildItem -Recurse `
#     -Path "$BuildDir\help" | Measure-Object -Property Length -Sum).Sum / 1MB)

####

Write-Host "`n[ Sign files ]"

if ($Sign) {
    Set-Location "$BuildDir\desktop"

    # Sign
    Get-ChildItem *.exe, *.dll -Recurse | ForEach-Object {
        & "$env:WORKSPACE\documents-pipeline\scripts\Sign.ps1" -File $_
        if ($LastExitCode -ne 0) { throw }
    }

    # Verify
    Get-ChildItem *.exe, *.dll -Recurse | % { Get-AuthenticodeSignature $_ }

    # VLC plugin cache
    if (
            (($Arch -like "x??") -and ($env:PROCESSOR_ARCHITECTURE -eq "AMD64")) -or
            (($Arch -eq "arm64") -and ($env:PROCESSOR_ARCHITECTURE -eq "ARM64"))
    )
    {
        Write-Host ".\vlc-cache-gen $PWD\plugins"
        & .\vlc-cache-gen "$PWD\plugins"
        if ($LastExitCode -ne 0) { throw }

        Write-Host "DELETE: vlc-cache-gen.exe"
        Remove-Item -Force -LiteralPath "vlc-cache-gen.exe"
    }

    Set-Location $PSScriptRoot
}
