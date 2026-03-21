; -- Installer Help --

#if str(ARCH) == "x64"
  #define sWinArch "x64"
  #define sPlatformFull "win_64"
#elif str(ARCH) == "x86"
  #define sWinArch "x86"
  #define sPlatformFull "win_32"
#endif

#ifndef sBrandingFolder
  #define sBrandingFolder "..\..\.."
#endif

#include sBrandingFolder + "\win-linux\package\windows\defines.iss"

#ifndef VERSION
  #define VERSION "0.0.0.0"
#endif
#define VERSION_SHORT Copy(VERSION,1,RPos('.',VERSION)-1)

#ifndef sOutputFileName
  #define sOutputFileName sPackageName + "-Help-" + VERSION + "-" + ARCH
#endif

[Setup]
AppName={#sAppName} Help
AppVersion={#VERSION}
AppVerName={#sAppName} Help {#VERSION_SHORT}
AppCopyright={#sAppCopyright}
; AppMutex=
AppPublisher={#sAppPublisher}
AppPublisherURL={#sAppPublisherURL}
AppSupportURL={#sAppSupportURL}
; AppComments=

PrivilegesRequired=admin

AlwaysShowDirOnReadyPage=yes
DefaultDirName={code:GetInstallPath}
DisableDirPage=yes
DirExistsWarning=no

SetupIconFile={#sBrandingFolder}\win-linux\extras\projicons\res\desktopeditors.ico

WizardImageFile={#sBrandingFolder}\win-linux\package\windows\data\dialogpicture*.bmp
WizardSmallImageFile={#sBrandingFolder}\win-linux\package\windows\data\dialogicon*.bmp

UninstallDisplayName={#sAppName} Help {#VERSION_SHORT} ({#ARCH})
UninstallDisplayIcon={app}\app.ico

VersionInfoVersion={#VERSION}

OutputDir=.\
OutputBaseFileName={#sOutputFileName}

#if str(ARCH) == "x64"
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#endif

#ifdef ENABLE_SIGNING
SignTool=byparam $p
#endif

SolidCompression=yes
Compression=lzma2/ultra64
LZMAUseSeparateProcess=yes

[Languages]
#ifdef _ONLYOFFICE
Name: en;    MessagesFile: compiler:Default.isl;
Name: be;    MessagesFile: compiler:Languages\Belarusian.isl;
Name: bg;    MessagesFile: compiler:Languages\Bulgarian.isl;
Name: ca;    MessagesFile: compiler:Languages\Catalan.isl;
Name: cs;    MessagesFile: compiler:Languages\Czech.isl;
Name: da;    MessagesFile: compiler:Languages\Danish.isl;
Name: de;    MessagesFile: compiler:Languages\German.isl;
Name: el;    MessagesFile: compiler:Languages\Greek.isl;
Name: es;    MessagesFile: compiler:Languages\Spanish.isl;
Name: fi;    MessagesFile: compiler:Languages\Finnish.isl;
Name: fr;    MessagesFile: compiler:Languages\French.isl;
Name: gl;    MessagesFile: compiler:Languages\Galician.isl;
Name: hu;    MessagesFile: compiler:Languages\Hungarian.isl;
Name: hy_AM; MessagesFile: compiler:Languages\Armenian.isl;
Name: id;    MessagesFile: compiler:Languages\Indonesian.isl;
Name: it_IT; MessagesFile: compiler:Languages\Italian.isl;
Name: ja;    MessagesFile: compiler:Languages\Japanese.isl;
Name: ko;    MessagesFile: compiler:Languages\Korean.isl;
Name: lo;    MessagesFile: compiler:Default.isl;
Name: lv;    MessagesFile: compiler:Languages\Latvian.isl;
Name: nl;    MessagesFile: compiler:Languages\Dutch.isl;
Name: no;    MessagesFile: compiler:Languages\Norwegian.isl;
Name: pl;    MessagesFile: compiler:Languages\Polish.isl;
Name: pt_BR; MessagesFile: compiler:Languages\BrazilianPortuguese.isl;
Name: pt_PT; MessagesFile: compiler:Languages\Portuguese.isl;
Name: ro;    MessagesFile: compiler:Languages\Romanian.isl;
Name: ru;    MessagesFile: compiler:Languages\Russian.isl;
Name: sk;    MessagesFile: compiler:Languages\Slovak.isl;
Name: sl;    MessagesFile: compiler:Languages\Slovenian.isl;
Name: sv;    MessagesFile: compiler:Languages\Swedish.isl;
Name: tr;    MessagesFile: compiler:Languages\Turkish.isl;
Name: uk;    MessagesFile: compiler:Languages\Ukrainian.isl;
Name: vi;    MessagesFile: compiler:Languages\Vietnamese.isl;
Name: zh_CN; MessagesFile: compiler:Languages\ChineseSimplified.isl;
#else
Name: ru;    MessagesFile: compiler:Languages\Russian.isl;
Name: en;    MessagesFile: compiler:Default.isl;
#endif

[Files]
Source: "{#DEPLOY_PATH}\editors\web-apps\apps\common\main\resources\help\*"; \
  DestDir: "{app}\editors\web-apps\apps\common\main\resources\help"; \
  Flags: ignoreversion recursesubdirs
Source: "{#DEPLOY_PATH}\editors\web-apps\apps\documenteditor\main\resources\help\*"; \
  DestDir: "{app}\editors\web-apps\apps\documenteditor\main\resources\help"; \
  Flags: ignoreversion recursesubdirs
Source: "{#DEPLOY_PATH}\editors\web-apps\apps\presentationeditor\main\resources\help\*"; \
  DestDir: "{app}\editors\web-apps\apps\presentationeditor\main\resources\help"; \
  Flags: ignoreversion recursesubdirs
Source: "{#DEPLOY_PATH}\editors\web-apps\apps\spreadsheeteditor\main\resources\help\*"; \
  DestDir: "{app}\editors\web-apps\apps\spreadsheeteditor\main\resources\help";  \
  Flags: ignoreversion recursesubdirs

[Code]
var
  InstallPath: string;

function GetHKLM: Integer;
begin
  if Is64BitInstallMode then
    Result := HKLM64
  else
    Result := HKLM;
end;

function GetInstallPath(Param: string): string;
begin
  RegQueryStringValue(GetHKLM(), '{#APP_REG_PATH}', 'AppPath', InstallPath);
  Result := InstallPath;
end;

function InitializeSetup(): Boolean;
begin
  Result := True;
  if not RegValueExists(GetHKLM(), '{#APP_REG_PATH}', 'AppPath') then
  begin
    Result := False;
    MsgBox('Error {#APP_REG_PATH}\AppPath not found', mbError, MB_OK);
  end;
end;
