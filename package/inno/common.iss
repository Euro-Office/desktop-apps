; -- Installer Common --

#ifndef BRANDING_DIR
#define BRANDING_DIR '.'
#endif
#include BRANDING_DIR + '\defines.iss'

#ifndef PACKAGE_EDITION
#define PACKAGE_EDITION 'Community'
#endif
#ifndef VERSION
#define VERSION '0.0.0.0'
#endif
#define sAppVerShort Copy(VERSION,1,RPos('.',VERSION)-1)
#ifndef ARCH
#define ARCH 'x64'
#endif
#ifndef BUILD_DIR
#define BUILD_DIR '..\build\' + ARCH
#endif
#ifndef OUTPUT_DIR
#define OUTPUT_DIR '.'
#endif
#ifndef OUTPUT_FILE
#if PACKAGE_EDITION == 'Community'
#define OUTPUT_FILE sPackageName + '-' + VERSION + '-' + ARCH
#else
#define OUTPUT_FILE sPackageName + '-' + PACKAGE_EDITION + '-' + VERSION + '-' + ARCH
#endif
#endif

#if FileExists(BRANDING_DIR + '\branding.iss')
#include BRANDING_DIR + '\branding.iss'
#endif

#define sUpgradeCode                 "607FEE744E0B34C449B45E9F419BB297"

[Setup]
AppName                   ={#sAppName}
AppVerName                ={#sAppName} {#sAppVerShort}
AppVersion                ={#VERSION}
AppId                     ={#APP_REG_UNINST_KEY}
VersionInfoVersion        ={#VERSION}

AppPublisher              = {#sAppPublisher}
AppPublisherURL           = {#sAppPublisherURL}
AppSupportURL             = {#sAppSupportURL}
AppCopyright              = {#sAppCopyright}
AppComments               = {cm:defprogAppDescription}

DefaultGroupName          = {#sCompanyName}
;UsePreviousAppDir         =no
DirExistsWarning          =no
#if Int(DecodeVer(PREPROCVER,1)) >= 6
DefaultDirName            ={commonpf}\{#APP_PATH}
#else
DefaultDirName            ={pf}\{#APP_PATH}
#endif
DisableProgramGroupPage   = yes
DisableWelcomePage        = no
DEPCompatible             = no
ASLRCompatible            = no
DisableDirPage            = auto
AllowNoIcons              = yes
AlwaysShowDirOnReadyPage  = yes
UninstallDisplayIcon      = {app}\app.ico
#if PACKAGE_EDITION == "Community" | PACKAGE_EDITION == "XP"
UninstallDisplayName      = {#sAppName} {#sAppVerShort} ({#ARCH})
#else
UninstallDisplayName      = {#sAppName} ({#PACKAGE_EDITION}) {#sAppVerShort} ({#ARCH})
#endif
OutputDir                 ={#OUTPUT_DIR}
PrivilegesRequired        =admin
AppMutex                  ={code:getAppMutex}
ChangesAssociations       =yes
ChangesEnvironment        =yes
SetupMutex                =ASC

#if Ver < EncodeVer(6,0,0) & ARCH == "x64"
ArchitecturesAllowed              = x64
ArchitecturesInstallIn64BitMode   = x64
#elif Ver >= EncodeVer(6,0,0) & ARCH == "x64"
ArchitecturesAllowed              = x64compatible
ArchitecturesInstallIn64BitMode   = x64compatible
#elif ARCH == "arm64"
ArchitecturesAllowed              = arm64
ArchitecturesInstallIn64BitMode   = arm64
#endif

#ifdef _WIN_XP
MinVersion                        = 5.0
OnlyBelowVersion                  = 6.1
#endif
OutputBaseFileName                ={#OUTPUT_FILE}

#ifdef SIGN
SignTool                  =byparam $p
#endif

SetupIconFile={#BRANDING_DIR}\..\..\win-linux\extras\projicons\res\icons\desktopeditors.ico
WizardStyle=classic dynamic
WizardSizePercent=100
WizardImageFile={#BRANDING_DIR}\res\WizImage-Light-*.png
WizardImageFileDynamicDark={#BRANDING_DIR}\res\WizImage-Dark-*.png
WizardSmallImageFile={#BRANDING_DIR}\res\WizSmallImage-Light-*.png
WizardSmallImageFileDynamicDark={#BRANDING_DIR}\res\WizSmallImage-Dark-*.png
#if PACKAGE_EDITION == "Enterprise"
LicenseFile={#BRANDING_DIR}\..\common\license\commercial\LICENSE.rtf
#else
LicenseFile={#BRANDING_DIR}\..\common\license\opensource\LICENSE.rtf
#endif

SolidCompression=yes
Compression=lzma2/ultra64
LZMAUseSeparateProcess=yes

[Languages]
#ifdef _ONLYOFFICE
Name: en; MessagesFile: compiler:Default.isl;
Name: ru; MessagesFile: compiler:Languages\Russian.isl;
#else
Name: ru; MessagesFile: compiler:Languages\Russian.isl;
Name: en; MessagesFile: compiler:Default.isl;
#endif
Name: bg; MessagesFile: compiler:Languages\Bulgarian.isl;
Name: ca; MessagesFile: compiler:Languages\Catalan.isl;
Name: cs; MessagesFile: compiler:Languages\Czech.isl;
Name: el; MessagesFile: compiler:Languages\Greek.isl;
;Name: et; MessagesFile: compiler:Languages\Estonian.isl;
Name: fi; MessagesFile: compiler:Languages\Finnish.isl;
;Name: lt; MessagesFile: compiler:Languages\Lithuanian.isl;
Name: lo; MessagesFile: compiler:Default.isl;
Name: nl; MessagesFile: compiler:Languages\Dutch.isl;
Name: de; MessagesFile: compiler:Languages\German.isl;
Name: fr; MessagesFile: compiler:Languages\French.isl;
Name: es; MessagesFile: compiler:Languages\Spanish.isl;
Name: pt_BR; MessagesFile: compiler:Languages\BrazilianPortuguese.isl;
Name: pt_PT; MessagesFile: compiler:Languages\Portuguese.isl;
Name: id; MessagesFile: compiler:Languages\Indonesian.isl;
Name: it_IT; MessagesFile: compiler:Languages\Italian.isl;
Name: pl; MessagesFile: compiler:Languages\Polish.isl;
Name: ro; MessagesFile: compiler:Languages\Romanian.isl;
Name: sk; MessagesFile: compiler:Languages\Slovak.isl;
Name: sl; MessagesFile: compiler:Languages\Slovenian.isl;
Name: sv; MessagesFile: compiler:Languages\Swedish.isl;
Name: tr; MessagesFile: compiler:Languages\Turkish.isl;
#if Int(DecodeVer(PREPROCVER,1)) < 6
Name: vi; MessagesFile: compiler:Languages\Vietnamese.islu;
Name: hy_AM; MessagesFile: compiler:Languages\Armenian.islu;
#else
Name: vi; MessagesFile: compiler:Languages\Vietnamese.isl;
Name: hy_AM; MessagesFile: compiler:Languages\Armenian.isl;
#endif
Name: zh_CN; MessagesFile: compiler:Languages\ChineseSimplified.isl;
;Name: hy_AM; MessagesFile: compiler:Languages\Armenian.islu;
;Name: hr; MessagesFile: compiler:Languages\Croatian.isl;
Name: da; MessagesFile: compiler:Languages\Danish.isl;
;Name: hi; MessagesFile: compiler:Languages\Hindi.islu;
Name: hu; MessagesFile: compiler:Languages\Hungarian.isl;
;Name: ga_IE; MessagesFile: compiler:Default.isl;
Name: ja; MessagesFile: compiler:Languages\Japanese.isl;
Name: ko; MessagesFile: compiler:Languages\Korean.isl;
Name: lv; MessagesFile: compiler:Languages\Latvian.isl;
Name: no; MessagesFile: compiler:Languages\Norwegian.isl;
Name: uk; MessagesFile: compiler:Languages\Ukrainian.isl;
Name: be; MessagesFile: compiler:Languages\Belarusian.isl;
Name: gl; MessagesFile: compiler:Languages\Galician.isl;
Name: si; MessagesFile: compiler:Languages\Sinhala.islu;
Name: zh_TW; MessagesFile: compiler:Languages\ChineseTraditional.isl;
Name: ar_SA; MessagesFile: compiler:Languages\Arabic.isl;
Name: sr_Latn_RS; MessagesFile: compiler:Languages\SerbianLatin.isl;
Name: sr_Cyrl_RS; MessagesFile: compiler:Languages\SerbianCyrillic.isl;
Name: en_GB; MessagesFile: compiler:Languages\EnglishBritish.isl;
Name: he; MessagesFile: compiler:Languages\Hebrew.isl;
Name: sq; MessagesFile: compiler:Languages\Albanian.isl;
#if Ver >= EncodeVer(6,1,1)
Name: ur; MessagesFile: compiler:Languages\Urdu.isl;
#endif

[LangOptions]
en.LanguageName=English (United States)
lo.LanguageName=ພາສາລາວ
;ga_IE.LanguageName=Gaeilge
ar_SA.LanguageName=الْعَرَبِيَّة
#if Ver >= EncodeVer(6,1,1)
ur.RightToLeft=yes
#endif

[CustomMessages]
#include '_messages.iss'

[Dirs]
Name: {commonappdata}\{#APP_PATH}\webdata\cloud; Flags: uninsalwaysuninstall;


[Files]
Source: "vc_redist.{#ARCH}.exe"; DestDir: {app}; Flags: deleteafterinstall; \
  AfterInstall: installVCRedist(ExpandConstant('{app}\vc_redist.{#ARCH}.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); \
  Check: not CheckVCRedist;

Source: "{#BUILD_DIR}\desktop\*"; DestDir: {app}; Flags: ignoreversion recursesubdirs;
#if defined(_WIN_XP) | defined(EMBED_HELP)
Source: "{#BUILD_DIR}\help\*"; DestDir: {app}; Flags: ignoreversion recursesubdirs;
#endif
Source: "{#BUILD_DIR}\desktop\*.exe"; DestDir: {app}; Flags: recursesubdirs signonce;
Source: "{#BUILD_DIR}\desktop\*.dll"; DestDir: {app}; Flags: recursesubdirs signonce;
Source: "package.config"; DestDir: {app}\converter;
#if PACKAGE_EDITION == "Enterprise"
Source: "{#BRANDING_DIR}\..\common\license\commercial\LICENSE.txt"; DestDir: {app}; DestName: "EULA.txt";
#else
Source: "{#BRANDING_DIR}\..\common\license\opensource\LICENSE.txt"; DestDir: {app};
#endif
Source: "{#BRANDING_DIR}\..\common\license\3dparty\3DPARTYLICENSE"; DestDir: {app};

[InstallDelete]
Type: filesandordirs; Name: {app}\editors\sdkjs-plugins
Type: files; Name: "{commondesktop}\{#sOldAppIconName}.lnk"; Tasks: desktopicon;
Type: files; Name: "{group}\{#sOldAppIconName}.lnk";

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon,{#sAppName}}; GroupDescription: {cm:AdditionalIcons};
;Name: fileassoc; Description: {cm:AssociateCaption};   GroupDescription: {cm:AssociateDescription};


[Icons]
;Name: {commondesktop}\{#sAppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon;
Name: {commondesktop}\{#sAppIconName}; FileName: {app}\{#iconsExe}; WorkingDir: {app}; Tasks: desktopicon; IconFilename: {app}\app.ico; AppUserModelID: {#APP_USER_MODEL_ID};
Name: {group}\{#sAppIconName};         Filename: {app}\{#iconsExe}; WorkingDir: {app}; IconFilename: {app}\app.ico; AppUserModelID: {#APP_USER_MODEL_ID};
Name: {group}\{cm:Uninstall}; IconFilename: {app}\{#iconsExe}; IconIndex: 25; Filename: {uninstallexe}; WorkingDir: {app};
Name: "{group}\{cm:jumpDOCX}"; IconFilename: "{app}\{#iconsExe}"; IconIndex: 14; Filename: "{app}\{#iconsExe}"; Parameters: "--new:word";
Name: "{group}\{cm:jumpXLSX}"; IconFilename: "{app}\{#iconsExe}"; IconIndex: 15; Filename: "{app}\{#iconsExe}"; Parameters: "--new:cell";
Name: "{group}\{cm:jumpPPTX}"; IconFilename: "{app}\{#iconsExe}"; IconIndex: 16; Filename: "{app}\{#iconsExe}"; Parameters: "--new:slide";
#ifdef _ONLYOFFICE
Name: "{group}\{cm:jumpDOCXF}"; IconFilename: "{app}\{#iconsExe}"; IconIndex: 17; Filename: "{app}\{#iconsExe}"; Parameters: "--new:form";
#endif

[Run]
;Filename: {app}\{#NAME_EXE_OUT}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent;
Filename: {app}\{#iconsExe}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent runasoriginaluser;
;Filename: http://www.onlyoffice.com/remove-portal-feedback-form.aspx; Description: Visit website; Flags: postinstall shellexec nowait
;Filename: ms-settings:defaultapps; Description: {cm:runOpenDefaultApps}; Flags:postinstall shellexec nowait unchecked; MinVersion: 10.0.10240;

[Ini]
;Filename: {app}\opts; Section: app; Key: lang; String: {language};


[Registry]
;Root: HKLM; Subkey: {#APP_REG_PATH};  Flags: uninsdeletekey;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: AppPath;    ValueData: {app};               Flags: uninsdeletevalue;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: locale;     ValueData: {code:getAppPrevLang}; Flags: uninsdeletevalue;
Root: HKCU; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: locale;     ValueData: {code:getAppPrevLang}; Flags: uninsdeletevalue;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: qword;    ValueName: timestamp;  ValueData: {code:getPosixTime}; Flags: uninsdeletevalue;
Root: HKLM; Subkey: "{#APP_REG_PATH}"; ValueType: "string"; ValueName: "PackageArch";    ValueData: "{#ARCH}";            Flags: uninsdeletevalue;
Root: HKLM; Subkey: "{#APP_REG_PATH}"; ValueType: "string"; ValueName: "PackageEdition"; ValueData: "{#PACKAGE_EDITION}"; Flags: uninsdeletevalue;
Root: HKLM; Subkey: "{#APP_REG_PATH}"; ValueType: "string"; ValueName: "PackageType";    ValueData: "inno";               Flags: uninsdeletevalue;

Root: HKLM; Subkey: Software\Classes\{#ASSOC_PROG_ID};                      Flags: uninsdeletekey
Root: HKLM; Subkey: Software\Classes\{#ASSOC_PROG_ID};                      ValueType: string; ValueName:; ValueData: {#ASSOC_APP_FRIENDLY_NAME};
Root: HKLM; Subkey: Software\Classes\{#ASSOC_PROG_ID}\DefaultIcon;          ValueType: string; ValueName:; ValueData: "{app}\{#iconsExe},0";
Root: HKLM; Subkey: Software\Classes\{#ASSOC_PROG_ID}\shell\open\command;   ValueType: string; ValueName:; ValueData: """{app}\{#iconsExe}"" ""%1""";
Root: HKLM; Subkey: Software\Classes\{#ASSOC_PROG_ID}\shell\open;           ValueType: string; ValueName: FriendlyAppName; ValueData: {#ASSOC_APP_FRIENDLY_NAME};

#ifdef _ONLYOFFICE
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}"; ValueType: "string"; ValueData: "URL:{#sAppName} Protocol"; Flags: uninsdeletekey;
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}"; ValueType: "string"; ValueName: "URL Protocol"; ValueData: "";
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}\DefaultIcon"; ValueType: "string"; ValueData: "{app}\{#iconsExe},0";
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}\Shell\Open\Command"; ValueType: "string"; ValueData: """{app}\{#iconsExe}"" ""%1""";
#endif

[UninstallDelete]
Type: filesandordirs; Name: {commonappdata}\{#APP_PATH}\*;  AfterInstall: RefreshEnvironment;
Type: filesandordirs; Name: "{app}\..\{#UPD_PATH}";
Type: files; Name: "{app}\svcrestart.bat";

[Code]
#include '_code.iss'

#ifdef PREPROCSAVE
#expr SaveToFile(AddBackslash(SourcePath) + "desktop_preprocessed.iss")
#endif
