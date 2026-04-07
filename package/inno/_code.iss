#define UNINSTALL_USE_CLEAR_PAGE
#if 0
[Code]
#endif
////////////////////
// Utils          //
////////////////////

function StartsWith(SubStr, S: String): Boolean;
begin
   Result := Pos(SubStr, S) = 1;
end;

function StringReplace(S, oldSubString, newSubString: String) : String;
var
  stringCopy : String;
begin
  stringCopy := S; //Prevent modification to the original string
  StringChange(stringCopy, oldSubString, newSubString);
  Result := stringCopy;
end;

function GetCommandlineParam(inParamName: String) : String;
var
   paramNameAndValue: String;
   i: Integer;
begin
   Result := '';

   for i:= 1 to ParamCount do begin
     paramNameAndValue := Lowercase(ParamStr(i));
     if StartsWith(inParamName, paramNameAndValue) then begin
       Result := StringReplace(paramNameAndValue, inParamName + ':', '');
       break;
     end;
   end;
end;

function CheckCommandlineParam(inpn: String) : Boolean;
var
   i: Integer;
begin
   Result := false;

   for i:= 1 to ParamCount do begin
     if Lowercase(inpn) = Lowercase(ParamStr(i)) then begin
       Result := true;
       break;
     end;
   end;
end;

function CheckVCRedist(): Boolean;
var
  Version: Integer;
begin
#ifndef _WIN_XP
  Version := PackVersionComponents(14, 32, 31332, 0);
#if ARCH == "arm64"
  Result := IsMsiProductInstalled('{DC9BAE42-810B-423A-9E25-E4073F1C7B00}', Version);
#else
  if Is64BitInstallMode then
    Result := IsMsiProductInstalled('{36F68A90-239C-34DF-B58C-64B30153CE35}', Version)
  else
    Result := IsMsiProductInstalled('{65E5BD06-6392-3027-8C26-853107D3CF1A}', Version);
#endif
#else
  Version := PackVersionComponents(14, 27, 29114, 0);
  if Is64BitInstallMode then
    Result := IsMsiProductInstalled('{C146EF48-4D31-3C3D-A2C5-1E91AF8A0A9B}', Version)
  else
    Result := IsMsiProductInstalled('{F899BAD3-98ED-308E-A905-56B5338963FF}', Version);
#endif
end;

function ReadBinFile(fileName: String; list: TStringList): Boolean;
var
  fs: TFileStream;
  buff: String;
  len: Word;
  ch: Char;
begin
  Result := False;
  if not FileExists(fileName) then
    Exit;
  list.Clear;
  try
    fs := TFileStream.Create(fileName, fmOpenRead);
  except
    Exit;
  end;
  while fs.Position < fs.Size do begin
    SetLength(buff, 1);
    try
      fs.ReadBuffer(buff, SizeOf(len));
    except
      fs.Free;
      Exit;
    end;
    len := Ord(buff[1]);
    SetLength(buff, len);
    try
      fs.ReadBuffer(buff, len * SizeOf(ch));
    except
      fs.Free;
      Exit;
    end;
    list.Add(buff);
  end;
  Result := True;
  fs.Free;
end;

procedure RemoveExtraFiles();
var
  i: Integer;
  appPath, path: String;
  files: TStringList;
begin
  files := TStringList.Create;
  appPath := ExpandConstant('{app}');
  if ReadBinFile(appPath + '\unins000.bin', files) then begin
    for i := 0 to files.Count - 1 do begin
      if DeleteFile(appPath + files[i]) then begin
        path := ExtractFileDir(files[i]);
        while (path <> '\') do begin
          if not RemoveDir(appPath + path) then
            break;
          path := ExtractFileDir(path);
        end;
      end;
    end;
  end;
  files.Free;
  DeleteFile(appPath + '\unins000.bin');
end;

////////////////////
// Associate Page //
////////////////////

type
  TKeyValue = record
    Key: string;
    Value: string;
  end;
  TArrayOfValues = array of TKeyValue;

var
  OnAudioClick: Boolean;
  ChlbAudio: TNewCheckListBox;
  AudioExtEnabled: Array of Boolean;
  AudioExts: Array of String;
  AChecked: Boolean;
  associatePage: TWizardPage;
  isFullAssociation: Boolean;

procedure initExtensions;
begin
  SetArrayLength(AudioExts, {#DimOf(FA_ARR)});
  SetArrayLength(AudioExtEnabled, {#DimOf(FA_ARR)});

#sub LoopAudioExt
  AudioExts[{#i}] := '{#FA_ARR[i]}';
#endsub
#for {i = 0; i < DimOf(FA_ARR); i++} LoopAudioExt
end;

procedure ChlbAudioClickCheck(Sender: TObject);
var
  i: Integer;
begin
  if not OnAudioClick then
  begin
    OnAudioClick := True;
    if ChlbAudio.Checked[2] then
    begin
      if not AChecked then
      begin
        AChecked := True;
        for i := 0 to {#DimOf(FA_ARR)} - 1 do
        begin
          ChlbAudio.ItemEnabled[i + 3] := True;
          ChlbAudio.Checked[i + 3] := AudioExtEnabled[i];
        end;
      end
      else
      begin
        for i := 0 to {#DimOf(FA_ARR)} - 1 do
          AudioExtEnabled[i] := ChlbAudio.Checked[i + 3];
      end;
    end
    else
    begin
      AChecked := False;
      for i := 0 to {#DimOf(FA_ARR)} - 1 do
      begin
        ChlbAudio.ItemEnabled[i + 3] := False;
//        ChlbAudio.Checked[i + 3] := ArrAudio[i];
      end;
    end;
    OnAudioClick := False;
    ChlbAudio.Repaint;
  end;
end;

procedure InitializeAssociatePage;
var
  lblAudio: TLabel;
  i: Integer;
  version: TWindowsVersion;
  createPage: Boolean;
  paramSkip: string;

  labelDesc, labelPath: TNewStaticText;
begin
  initExtensions();

  ChlbAudio  := nil;
  createPage := False;
  if not WizardSilent() then begin
    paramSkip := GetCommandlineParam('/skip');
    if (not Length(paramSkip) > 0) or (paramSkip <> 'associates') then begin
      createPage := True;
    end
  end;

  if createPage then begin
    associatePage := CreateCustomPage(wpSelectTasks, CustomMessage('AssociateCaption'), CustomMessage('AssociateDescription'));

    //GetWindowsVersionEx(version);
    //if version.Major < 10 then begin
      lblAudio          := TLabel.Create(associatePage);
      lblAudio.Parent   := associatePage.Surface;
      lblAudio.Caption  := ExpandConstant('{cm:AssociateAudio}');
      lblAudio.AutoSize := True;
      lblAudio.Width    := associatePage.SurfaceWidth;
      lblAudio.WordWrap := True;
      lblAudio.Left     := 0;
      lblAudio.Top      := 0;

      ChlbAudio         := TNewCheckListBox.Create(associatePage);
      ChlbAudio.Parent  := associatePage.Surface;
      ChlbAudio.Left    := 0;
      ChlbAudio.Top     := lblAudio.Top + lblAudio.Height + 4;
      ChlbAudio.Width   := associatePage.SurfaceWidth;
      ChlbAudio.Height  := associatePage.SurfaceHeight - ChlbAudio.Top - 4 - 3;

      ChlbAudio.AddRadioButton(ExpandConstant('{cm:AssociateDont}'), '', 0, False, True, nil);
      ChlbAudio.AddRadioButton(ExpandConstant('{cm:AssociateAll}'),  '', 0, False, True, nil);
      ChlbAudio.AddRadioButton(ExpandConstant('{cm:AssociateSel}'),  '', 0, True,  True, nil);

      AChecked := True;

#sub LoopCheckBox
      ChlbAudio.AddCheckBox('{#FA_ARR[i]}', '', 1, False, True, False, False, nil);
      AudioExtEnabled[{#i}] := True;
#endsub
#for {i = 0; i < DimOf(FA_ARR); i++} LoopCheckBox

      OnAudioClick := False;
      ChlbAudio.OnClickCheck := @ChlbAudioClickCheck;

      ChlbAudio.Checked[1] := True;
      ChlbAudioClickCheck(ChlbAudio);
    //end else begin
    //  labelDesc           := TNewStaticText.Create(associatePage);
    //  labelDesc.Parent    := associatePage.Surface;
    //  labelDesc.Width     := associatePage.SurfaceWidth;
    //  labelDesc.WordWrap  := True;
    //  labelDesc.Caption   := ExpandConstant('{cm:warnWin10FileAssociationDesc}');

    //  labelPath           := TNewStaticText.Create(associatePage);
    //  labelPath.Parent    := associatePage.Surface;
    //  labelPath.Top       := labelDesc.Top + labelDesc.Height + ScaleY(8);
    //  labelPath.Width     := associatePage.SurfaceWidth;
    //  labelPath.WordWrap  := True;
    //  labelPath.Caption   := ExpandConstant('{cm:warnWin10FileAssociationPath}');
    //  labelPath.Font.Style := [fsBold];
    //end
  end else begin
    associatePage := nil
  end;

  //vc_desctopiconshow := True;
  //WizardForm.TasksList.OnClickCheck := @OnTasksListClickCheck;
end;

function isAssociateExtension(index: Integer): Boolean;
begin
  if ChlbAudio = nil then begin
    if isFullAssociation then Result := True
    else Result := False
  end else
    Result := ChlbAudio.Checked[1] or (ChlbAudio.Checked[2] and ChlbAudio.Checked[index + 3]);
end;

procedure DoPostInstall();
begin
    isFullAssociation := CheckCommandlineParam('/FULLASSOCIATION');
    if (associatePage = nil) and isFullAssociation then begin
      initExtensions();
    end;
end;

{
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
begin
    MsgBox(MemoDirInfo, mbInformation, MB_OK);
    Result:=MemoUserInfoInfo + NewLine + MemoDirInfo + NewLine + MemoTypeInfo + NewLine + MemoComponentsInfo + NewLine + MemoGroupInfo + NewLine + MemoTasksInfo;
end;
}

////////////////////
// Uninstall Page //
////////////////////

#ifdef UNINSTALL_USE_CLEAR_PAGE
var
  IsClearData: Boolean;

procedure InitializeUninstallProgressForm();
var
  PageText: TNewStaticText;
  PageNameLabel: string;
  PageDescriptionLabel: string;
  CancelButtonEnabled: Boolean;
  CancelButtonModalResult: Integer;
  CheckBox: TNewCheckBox;
  UninstallFirstPage: TNewNotebookPage;
  UninstallNextButton: TNewButton;
begin
  IsClearData := False;

  if not UninstallSilent then
  begin
    { Create the first page and make it active }
    UninstallFirstPage := TNewNotebookPage.Create(UninstallProgressForm);
    UninstallFirstPage.Notebook := UninstallProgressForm.InnerNotebook;
    UninstallFirstPage.Parent := UninstallProgressForm.InnerNotebook;
    UninstallFirstPage.Align := alClient;

    PageText := TNewStaticText.Create(UninstallProgressForm);
    PageText.Parent := UninstallFirstPage;
    PageText.Top := UninstallProgressForm.StatusLabel.Top;
    PageText.Left := UninstallProgressForm.StatusLabel.Left;
    PageText.Width := UninstallProgressForm.StatusLabel.Width;
    PageText.Height := UninstallProgressForm.StatusLabel.Height;
    PageText.AutoSize := False;
    PageText.ShowAccelChar := False;
    PageText.Caption := ExpandConstant('{cm:UninstallPageLabel}');

    CheckBox := TNewCheckBox.Create(UninstallProgressForm);
    CheckBox.Parent := UninstallFirstPage;
    CheckBox.Top := PageText.Top + PageText.Height + ScaleY(8);
    CheckBox.Left := PageText.Left;
    CheckBox.Width := UninstallProgressForm.Width;
    CheckBox.Height := ScaleY(17);
    CheckBox.Caption := ' ' + ExpandConstant('{cm:UninstallOptionClearData}');

    UninstallProgressForm.InnerNotebook.ActivePage := UninstallFirstPage;

    PageNameLabel := UninstallProgressForm.PageNameLabel.Caption;
    PageDescriptionLabel := UninstallProgressForm.PageDescriptionLabel.Caption;

    { Create the second page }

    UninstallNextButton := TNewButton.Create(UninstallProgressForm);
    UninstallNextButton.Parent := UninstallProgressForm;
    UninstallNextButton.Left := UninstallProgressForm.CancelButton.Left - UninstallProgressForm.CancelButton.Width - ScaleX(10);
    UninstallNextButton.Top := UninstallProgressForm.CancelButton.Top;
    UninstallNextButton.Width := UninstallProgressForm.CancelButton.Width;
    UninstallNextButton.Height := UninstallProgressForm.CancelButton.Height;
    UninstallNextButton.Caption := ExpandConstant('{cm:Uninstall}');
    { Make the "Uninstall" button break the ShowModal loop }
    UninstallNextButton.ModalResult := mrOK;

    UninstallNextButton.TabOrder := UninstallProgressForm.CancelButton.TabOrder;
    UninstallProgressForm.CancelButton.TabOrder := UninstallNextButton.TabOrder + 1;

    { Run our wizard pages }
    //UpdateUninstallWizard;
    CancelButtonEnabled := UninstallProgressForm.CancelButton.Enabled
    UninstallProgressForm.CancelButton.Enabled := True;
    CancelButtonModalResult := UninstallProgressForm.CancelButton.ModalResult;
    UninstallProgressForm.CancelButton.ModalResult := mrCancel;

    if UninstallProgressForm.ShowModal = mrCancel then Abort;

    UninstallNextButton.Enabled := False;
    IsClearData := CheckBox.State = cbChecked;

    { Restore the standard page payout }
    UninstallProgressForm.CancelButton.Enabled := CancelButtonEnabled;
    UninstallProgressForm.CancelButton.ModalResult := CancelButtonModalResult;

    UninstallProgressForm.PageNameLabel.Caption := PageNameLabel;
    UninstallProgressForm.PageDescriptionLabel.Caption := PageDescriptionLabel;

    UninstallProgressForm.InnerNotebook.ActivePage := UninstallProgressForm.InstallingPage;
  end;
end;
#endif

////////////////////
// Common         //
////////////////////

const
  SMTO_ABORTIFHUNG = 2;
  WM_WININICHANGE = $001A;
  WM_SETTINGCHANGE = WM_WININICHANGE;
  WM_USER = $400;

type
  WPARAM = UINT_PTR;
  LPARAM = INT_PTR;
  LRESULT = INT_PTR;

var
  gHWND: Longint;
  isInstalled: Boolean;

procedure GetSystemTimeAsFileTime(var lpFileTime: TFileTime); external 'GetSystemTimeAsFileTime@kernel32.dll';

function GetHKLM: Integer; forward;

function CheckAppRegData(RegName: string; RegData: string): Boolean;
var
  Data: string;
begin
  Result := True;
  if RegQueryStringValue(HKLM, '{#APP_REG_PATH}', RegName, Data) then
    if (Trim(Data) = '') or (CompareText(RegData, Data) <> 0) then
      Result := False;
  // MsgBox(FmtMessage('App Reg: %1 - %2 - %3', [RegData, Data, IntToStr(CompareText(RegData, Data))]), mbInformation, MB_OK);
end;

function CheckUninstalledRegData(RegRoot: integer; RegData: string): Boolean;
var
  Data: string;
begin
  Result := True;
  if RegQueryStringValue(RegRoot,
    'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#APP_REG_UNINST_KEY}_is1',
    'DisplayName', Data) then
    if Pos(RegData, Data) = 0 then
      Result := False;
  // MsgBox(FmtMessage('Uninstalled Reg: %1 - %2 - %3', [RegData, Data, IntToStr(Pos(RegData, Data))]), mbInformation, MB_OK);
end;

function CheckInstalledProduct(): Boolean;
var
  Check: boolean;
begin
  Result := True;
  Check := True;

  Check := Check and CheckAppRegData('PackageType', 'inno');
  Check := Check and not IsMsiProductInstalled('{47EEF706-B0E4-4C43-944B-E5F914B92B79}', 0);
  if Result and not Check then
  begin
    MsgBox(ExpandConstant('{cm:ErrorMismatchInstalledType}'), mbCriticalError, MB_OK);
    Result := False;
  end;

  Check := Check and CheckAppRegData('PackageArch', '{#ARCH}');
  Check := Check and CheckUninstalledRegData(HKLM, '({#ARCH})');
#if ARCH == "x86"
  if IsWin64 and not Is64BitInstallMode then
    Check := Check and CheckUninstalledRegData(HKLM64, '({#ARCH})');
#else
  if IsWin64 and Is64BitInstallMode then
    Check := Check and CheckUninstalledRegData(HKLM32, '({#ARCH})');
#endif
  if Result and not Check then
  begin
    MsgBox(ExpandConstant('{cm:ErrorMismatchInstalledArch}'), mbCriticalError, MB_OK);
    Result := False;
  end;

  Check := Check and CheckAppRegData('PackageEdition', '{#PACKAGE_EDITION}');
  if Result and not Check then
  begin
    MsgBox(ExpandConstant('{cm:ErrorMismatchInstalledEdition}'), mbCriticalError, MB_OK);
    Result := False;
  end;
end;

function SendTextMessageTimeout(hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: PAnsiChar; fuFlags: UINT; uTimeout: UINT; out lpdwResult: DWORD): LRESULT;
  external 'SendMessageTimeoutA@user32.dll stdcall';

procedure InitializeWizard();
var
  paramSkip: string;
  path: string;
begin
  InitializeAssociatePage();

  if RegQueryStringValue(GetHKLM(), '{#APP_REG_PATH}', 'AppPath', path) and
        FileExists(path + '\{#NAME_EXE_OUT}') then
    isInstalled := false
  else isInstalled := true;
end;

function InitializeSetup(): Boolean;
var
  OutResult: Boolean;
  path, mess: string;
  regkey: integer;

  hWnd: Longint;
  msg: string;
begin
  gHWND := 0;
  OutResult := True;

  if not CheckInstalledProduct() then
  begin
    OutResult := False;
  end;

  if OutResult then begin
    if CheckCommandlineParam('/update') then
    begin
      gHWND := FindWindowByClassName('{#APPWND_CLASS_NAME}');
      if gHWND <> 0 then begin
        OutResult := (IDOK = MsgBox(ExpandConstant('{cm:UpdateAppRunning,{#sAppName}}'), mbInformation, MB_OKCANCEL));
        if OutResult then begin
          PostMessage(gHWND, WM_USER+254, 0, 0);
          Sleep(1000);

          while true do begin
            hWnd := FindWindowByClassName('{#APPWND_CLASS_NAME}');
            if hWnd <> 0 then begin
              msg := FmtMessage(SetupMessage(msgSetupAppRunningError), ['{#sAppName}']);
              if IDCANCEL = MsgBox(msg, mbError, MB_OKCANCEL) then begin
                OutResult := false;
                break;
              end;
            end else
              break;
          end;
        end;
      end;
    end;
  end;

  Result := OutResult;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  regValue, userPath: string;
  findRec: TFindRec;
  ErrorCode: Integer;
  version: TWindowsVersion;
begin
  if CurUninstallStep = usUninstall then
  begin
    GetWindowsVersionEx(version);
    if (version.Major > 6) or ((version.Major = 6) and (version.Minor >= 1)) then begin
      Exec(ExpandConstant('{app}\{#iconsExe}'), '--remove-jump-list', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ErrorCode);
      Exec(ExpandConstant('{app}\updatesvc.exe'), '--delete', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
    end;

    RegQueryStringValue(GetHKLM(), ExpandConstant('{#APP_REG_PATH}'), 'uninstall', regValue);

    if (regValue <> 'full') and
#ifndef UNINSTALL_USE_CLEAR_PAGE
        (MsgBox(ExpandConstant('{cm:WarningClearAppData}'), mbConfirmation, MB_YESNO) = IDYES)
#else
        IsClearData
#endif
            then regValue := 'soft';

    userPath := ExpandConstant('{localappdata}\{#sIntCompanyName}');
    if regValue = 'soft' then begin
      RegDeleteKeyIncludingSubkeys(GetHKLM(), 'Software\{#sIntCompanyName}');
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\{#sIntCompanyName}');

      // remove all app and user cashed data except of folders 'recover' and 'sdkjs-plugins'
      userPath := userPath + '\{#sIntProductName}';
      DelTree(userPath + '\*', False, True, False);

      userPath := userPath + '\data';
      if FindFirst(userPath + '\*', findRec) then begin
        try repeat
            if findRec.Attributes and FILE_ATTRIBUTE_DIRECTORY = 0 then
              DeleteFile(userPath + '\' + findRec.Name)
            else if (findRec.Name <> '.') and (findRec.Name <> '..') and
                (findRec.Name <> 'recover') and (findRec.Name <> 'sdkjs-plugins') then begin
              DelTree(userPath + '\' + findRec.Name, True, True, True);
            end;
          until not FindNext(findRec);
        finally
          FindClose(findRec);
        end;
      end;

    end else
    if regValue = 'full' then begin
      DelTree(userPath, True, True, True);
      RegDeleteKeyIncludingSubkeys(GetHKLM(), 'Software\{#sIntCompanyName}');
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\{#sIntCompanyName}');
    end;

    RegDeleteValue(HKEY_CLASSES_ROOT, 'Local Settings\Software\Microsoft\Windows\Shell\MuiCache', ExpandConstant('{app}\{#iconsExe}'));
  end else
  if CurUninstallStep = usPostUninstall then begin
    RemoveExtraFiles();
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  paramStore: string;
  ErrorCode: Integer;
  version: TWindowsVersion;
begin
  if CurStep = ssPostInstall then begin
    DoPostInstall();
    GetWindowsVersionEx(version);
    if (version.Major > 6) or ((version.Major = 6) and (version.Minor >= 1)) then begin
      Exec(ExpandConstant('{app}\{#iconsExe}'), '--create-jump-list', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ErrorCode);
      if CheckCommandlineParam('/noupdates') then begin
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, ExpandConstant('{#APP_REG_PATH}'), 'CheckForUpdates', 0);
      end else
        Exec(ExpandConstant('{app}\updatesvc.exe'), '--install "' + ExpandConstant('{cm:UpdateService}') + '."', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
    end;

    paramStore := GetCommandlineParam('/store');
    if Length(paramStore) > 0 then begin
      RegWriteStringValue(HKEY_LOCAL_MACHINE, ExpandConstant('{#APP_REG_PATH}'), 'Store', paramStore);
    end;

    paramStore := GetCommandlineParam('/uninst');
    if (Length(paramStore) > 0) and (paramStore = 'full') then begin
      RegWriteStringValue(HKEY_LOCAL_MACHINE, ExpandConstant('{#APP_REG_PATH}'), 'uninstall', paramStore);
    end;

    if CheckCommandlineParam('/disableplugins') then begin
      if DirExists(ExpandConstant('{app}\editors\sdkjs-plugins\') + '{AA2EA9B6-9EC2-415F-9762-634EE8D9A95E}') then
        DelTree(ExpandConstant('{app}\editors\sdkjs-plugins\') + '{AA2EA9B6-9EC2-415F-9762-634EE8D9A95E}', True, True, True);
    end;

    if CheckCommandlineParam('/noassocheck') then begin
      RegWriteStringValue(HKEY_LOCAL_MACHINE, ExpandConstant('{#APP_REG_PATH}'), 'ignoreAssocMsg', 'true');
    end;

  end else
  if CurStep = ssDone then begin
    // if not (gHWND = 0) then begin
    if CheckCommandlineParam('/update') and not CheckCommandlineParam('/nolaunch') then begin
      ShellExecAsOriginalUser('', ExpandConstant('{app}\{#iconsExe}'), '', '', SW_SHOW, ewNoWait, ErrorCode);
    end
  end else
    WizardForm.CancelButton.Enabled := isInstalled;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  path: string;
  ErrorCode: integer;
  version: TWindowsVersion;
begin
  GetWindowsVersionEx(version);
  if (version.Major > 6) or ((version.Major = 6) and (version.Minor >= 1)) then begin
    if FileExists(ExpandConstant('{app}\updatesvc.exe')) then
      Exec(ExpandConstant('{app}\updatesvc.exe'), '--delete', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
  end;

  path := ExpandConstant('{app}\editors\web-apps');
  if DirExists(path) then DelTree(path, true, true, true);

  path := ExpandConstant('{app}\editors\sdkjs');
  if DirExists(path) then DelTree(path, true, true, true)
end;

#ifndef _WIN_XP
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := (PageID = wpSelectDir) and not CheckCommandlineParam('/enabledirpage');
end;
#endif

function getAppMutex(P: String): String;
var
  hWnd: Longint;
begin
  if not CheckCommandlineParam('/update') then
    Result := '{#APP_MUTEX_NAME}'
  else
    Result := 'UPDATE';
end;

procedure installVCRedist(FileName, LabelCaption: String);
var
  Params:    String;
  ErrorCode: Integer;
begin
  if Length(LabelCaption) > 0 then WizardForm.StatusLabel.Caption := LabelCaption;

  Params := '/quiet /norestart';

  ShellExec('', FileName, Params, '', SW_SHOW, ewWaitUntilTerminated, ErrorCode);

  WizardForm.StatusLabel.Caption := SetupMessage(msgStatusExtractFiles);
end;

function GetHKLM: Integer;
begin
  if IsWin64 then
    Result := HKLM64
  else
    Result := HKEY_LOCAL_MACHINE;
end;

function getPosixTime: string;
var
  fileTime: TFileTime;
  fileTimeNano100: Int64;
begin
  //GetSystemTime(systemTime);

  // the current file time
  //SystemTimeToFileTime(systemTime, fileTime);
  GetSystemTimeAsFileTime(fileTime);

  // filetime in 100 nanosecond resolution
  fileTimeNano100 := Int64(fileTime.dwHighDateTime) shl 32 + fileTime.dwLowDateTime;

  //Log('The Value is: ' + IntToStr(fileTimeNano100/10000 - 11644473600000));

  //to milliseconds and unix windows epoche offset removed
  Result := IntToStr(fileTimeNano100/10000 - 11644473600000);
end;

function getAppPrevLang(param: string): string;
var
  lang: string;
begin
  if not (WizardSilent() and
        RegValueExists(GetHKLM(), '{#APP_REG_PATH}', 'locale') and
            RegQueryStringValue(GetHKLM(), '{#APP_REG_PATH}', 'locale', lang)) then
  begin
    lang := ExpandConstant('{cm:AppLocale}')
  end;

  result := lang;
end;
