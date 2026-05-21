param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$CompanyName = "Euro-Office",
    [string]$ProductName = "DesktopEditors",
    [string]$BuildDir,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com",
    [switch]$Debug
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not $BuildDir) {
    $BuildDir = "build\$Arch"
}
$MsiFile = switch ($Target) {
    "commercial" { "$CompanyName-$ProductName-Enterprise-$Version-$Arch.msi" }
    default      { "$CompanyName-$ProductName-$Version-$Arch.msi" }
}
$VersionShort = "$($Version.Major).$($Version.Minor).$($Version.Build)"
$MsiBuild = switch ($Arch) {
    "x64" { "MsiBuild64" }
    "x86" { "MsiBuild32" }
    "arm64" { "MsiBuildARM64" }
}
$LanguageCodes = @(
    1033, # en              English (United States)
    1025, # ar              Arabic (Saudi Arabia)
    1059, # be              Belarusian
    1026, # bg              Bulgarian
    1027, # ca              Catalan
    1029, # cs              Czech
    1030, # da              Danish
    1031, # de              German
    1032, # el              Greek
    2057, # en_GB           English (United Kingdom)
    3082, # es              Spanish (Modern Sort)
    1035, # fi              Finnish
    1036, # fr              French
    1110, # gl              Galician
    1037, # he              Hebrew
    1038, # hu              Hungarian
    1057, # id              Indonesian
    1040, # it              Italian
    1041, # ja              Japanese
    1042, # ko              Korean
    1062, # lv              Latvian
    1044, # nb              Norwegian (Bokmal)
    1043, # nl              Dutch
    1045, # pl              Polish
    2070, # pt              Portuguese (Portugal)
    1046, # pt_BR           Portuguese (Brazil)
    1048, # ro              Romanian
    1049, # ru              Russian
    1051, # sk              Slovak
    1060, # sl              Slovenian
    1052, # sq              Albanian
    3098, # sr_SP_Cyrillic  Serbian (Cyrillic)
    2074, # sr_SP_Latin     Serbian (Latin)
    1053, # sv              Swedish
    1055, # tr              Turkish
    1058, # uk              Ukrainian
    1056, # ur              Urdu
    1066, # vi              Vietnamese
    2052, # zh              Chinese (Simplified)
    1028  # zh_TW           Chinese (Traditional)
)
$AssociationList = @(
    "doc", "dot", "docx", "dotx", "docm", "dotm",
    "xls", "xlt", "xlsx", "xltx", "xlsm", "xltm", "xlsb",
    "ppt", "pot", "pps", "pptx", "potx", "ppsx", "pptm", "potm", "ppsm",
    "vsdx", "vstx", "vssx", "vsdm", "vstm", "vssm",
    "odt", "ott", "ods", "ots", "odp", "otp", "fodt", "fods", "fodp",
    "pages", "numbers", "key",
    "djvu", "fb2", "pdf", "rtf", "xps", "oxps",
    "epub", "html", "xml",
    "csv", "txt",
    "docxf"
)
$LicensePath = "common\license"
$PluginManagerPath = "editors\sdkjs-plugins\{AA2EA9B6-9EC2-415F-9762-634EE8D9A95E}"
$MD5 = New-Object -TypeName System.Security.Cryptography.MD5CryptoServiceProvider
$ASCII = New-Object -TypeName System.Text.ASCIIEncoding
[guid]$GUID = $MD5.ComputeHash($ASCII.GetBytes(`
    "$CompanyName $ProductName $VersionShort $Arch"))
$ProductCode = "{" + $GUID.ToString().ToUpper() + "}"

Write-Host @"
Version     = $Version
Arch        = $Arch
CompanyName = $CompanyName
ProductName = $ProductName
BuildDir    = $BuildDir
Sign        = $Sign
MsiFile     = $MsiFile
MsiBuild    = $MsiBuild
ProductCode = $ProductCode
"@

####

Write-Host "`n[ Get Advanced Installer path ]"

if ($env:ADVINSTPATH) {
    $AdvInstPath = $env:ADVINSTPATH
} else {
    $RegPath = "HKLM:\SOFTWARE\WOW6432Node\Caphyon\Advanced Installer"
    $AdvInstPath = (Get-ItemProperty $RegPath)."InstallRoot" + "bin\x86"
}
$AdvInstPath
$env:Path = "$AdvInstPath;$env:Path"

####

Push-Location "advinst"

Write-Host "`n[ Create package.config ]"

Write-Host "WRITE: ..\$BuildDir\desktop\converter\package.config"
Write-Output "package=msi" `
    | Out-File -Encoding ASCII "..\$BuildDir\desktop\converter\package.config"

####

Write-Host "`n[ Create Advanced Installer config ]"

$AdvInstConfig = @()
if (-not $Sign) {
    $AdvInstConfig += "ResetSig"
}
if ($Arch -eq "x86") {
    $AdvInstConfig += `
        "SetComponentAttribute -feature_name MainFeature -unset -64bit_component", `
        "SetComponentAttribute -feature_name Files -unset -64bit_component", `
        "SetComponentAttribute -feature_name Registry -unset -64bit_component", `
        "SetComponentAttribute -feature_name PluginManager -unset -64bit_component", `
        "SetComponentAttribute -feature_name UpdateService -unset -64bit_component", `
        "SetComponentAttribute -feature_name RegFileTypeAssociations -unset -64bit_component"
    $AssociationList | % {$AdvInstConfig += `
            "SetComponentAttribute -feature_name FA_$($_.ToUpper()) -unset -64bit_component"}
}
$LanguageCodes | % {$AdvInstConfig += "SetProductCode -langid $_ -guid $ProductCode"}
$AdvInstConfig += `
    "SetProperty Version=$VersionShort", `
    "SetVersion $Version -noprodcode", `
    "SetCurrentFeature MainFeature", `
    "UpdateFile APPDIR\DesktopEditors.exe ..\$BuildDir\desktop\DesktopEditors.exe", `
    "UpdateFile APPDIR\updatesvc.exe ..\$BuildDir\desktop\updatesvc.exe", `
    "NewSync APPDIR ..\$BuildDir\desktop -existingfiles keep -feature Files", `
    "NewSync APPDIR\$PluginManagerPath ..\$BuildDir\desktop\$PluginManagerPath -existingfiles delete -feature PluginManager", `
    "AddFile APPDIR ..\$LicensePath\3dparty\3DPARTYLICENSE"
if ($Target -ne "commercial") {
    $AdvInstConfig += `
        "AddFile APPDIR ..\$LicensePath\opensource\LICENSE.txt"
} else {
    Copy-Item -Force `
        -Path "..\$LicensePath\commercial\LICENSE.txt" `
        -Destination "..\$LicensePath\commercial\EULA.txt"
    $AdvInstConfig += `
        "SetProperty Edition=Enterprise", `
        "SetProperty AI_PRODUCTNAME_ARP=`"[|AppName] ([|Edition]) [|Version] ([|Arch])`"", `
        "SetEula -rtf `"$("..\$LicensePath\commercial\LICENSE.rtf" | Resolve-Path)`"", `
        "SetPackageName `"$MsiFile`" -buildname $MsiBuild", `
        "AddFile APPDIR ..\$LicensePath\commercial\EULA.txt"
}
if ($Debug) {
    $AdvInstConfig += "Save"
}
$AdvInstConfig += `
    "Rebuild -buildslist $MsiBuild"
$AdvInstConfig = ";aic", $AdvInstConfig
$AdvInstConfig
Write-Output $AdvInstConfig | Out-File -Encoding UTF8 "DesktopEditors.aic"

####

Write-Host "`n[ Build Advanced Installer project ]"

Write-Host "AdvancedInstaller.com /execute DesktopEditors.aip DesktopEditors.aic"
& AdvancedInstaller.com /? | Select-Object -First 1
& AdvancedInstaller.com /execute DesktopEditors.aip DesktopEditors.aic
if ($LastExitCode -ne 0) { throw }

####

Write-Host "`n[ Fix Summary Information Properties ]"

$Template = ";$($LanguageCodes -join ','),0"
$Template = switch ($Arch) {
    "x64" { "x64" + $Template }
    "x86" { "Intel" + $Template }
    "arm64" { "Arm64" + $Template }
}

Write-Host "MsiInfo $MsiFile /p $Template"
& MsiInfo $MsiFile /p $Template
if ($LastExitCode -ne 0) { throw }

if ($Sign) {
    Write-Host "signtool sign /a /n $CertName /t $TimestampServer /v $MsiFile"
    & signtool sign /a /n $CertName /t $TimestampServer /v $MsiFile
    if ($LastExitCode -ne 0) { throw }
}

Pop-Location
