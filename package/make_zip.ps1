param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$CompanyName = "Euro-Office",
    [string]$ProductName = "DesktopEditors",
    [string]$BuildDir
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not $BuildDir) {
    $BuildDir = "build\$Arch"
}
if (-not (Test-Path "$BuildDir")) {
    Write-Error "Path `"$BuildDir`" does not exist"
}
$ZipFile = switch ($Target) {
    "commercial" { "$CompanyName-$ProductName-Enterprise-$Version-$Arch.zip" }
    "xp"         { "$CompanyName-$ProductName-XP-$Version-$Arch.zip" }
    default      { "$CompanyName-$ProductName-$Version-$Arch.zip" }
}

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
CompanyName = $CompanyName
ProductName = $ProductName
BuildDir    = $BuildDir
ZipFile     = $ZipFile
"@

####

Write-Host "`n[ Create archive ]"

New-Item -ItemType Directory -Force -Path "zip" | Out-Null
Push-Location "zip"

if (Test-Path "$ZipFile") {
    Write-Host "DELETE: $ZipFile"
    Remove-Item -Force -LiteralPath "$ZipFile"
}

Write-Host "7z a -y $ZipFile ..\$BuildDir\desktop\*"
& 7z a -y "$ZipFile" "..\$BuildDir\desktop\*"
if ($LastExitCode -ne 0) { throw }

if ($Target -eq "standalone" -or $Target -eq "xp") {
    Write-Host "7z a -y $ZipFile ..\$BuildDir\help\*"
    & 7z a -y "$ZipFile" "..\$BuildDir\help\*"
    if ($LastExitCode -ne 0) { throw }
}

Pop-Location
