; Windows x64 installer. Stage the Release executable and runtime DLLs with windeployqt first.
#ifndef AppStageDir
  #error "Pass /DAppStageDir=<absolute path to the staged Windows application>."
#endif

#define MyAppVersion "1.1.1"
#define MyAppName "KeyPresser Hardware"
#define LegacyAppName "KeyPresser硬件版"
#define MyAppPublisher "Finn Wang"
#define MyAppURL "https://github.com/FinnWang-CN/KeyPresserHardware"
#define MyAppExeName "KeyPresserHardware.exe"
#define MyAppAssocExt ".kphset"
#define MyAppAssocKey LegacyAppName + "File" + MyAppAssocExt

[Setup]
AppId={{914F10FC-2228-489E-950F-F1613009863D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={autopf}\{#LegacyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequiredOverridesAllowed=commandline
ChangesAssociations=yes
DisableProgramGroupPage=yes
OutputDir=dist
OutputBaseFilename=KeyPresserHardware-Setup-{#MyAppVersion}-x64
SetupIconFile=keypresser.ico
SolidCompression=yes
WizardStyle=modern dynamic
WizardSmallImageFile=

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[CustomMessages]
english.SupportAuthor=Support the author
english.ScanWithWeChat=Scan this code in WeChat.
chinesesimplified.SupportAuthor=赞赏作者
chinesesimplified.ScanWithWeChat=请打开微信扫一扫。

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#AppStageDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppStageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "{#MyAppExeName}"
Source: "Arduino\keypresser.ino"; DestDir: "{app}\Arduino\keypresser"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "png\wechat-support.png"; Flags: dontcopy
Source: "png\support-heart.png"; Flags: dontcopy

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName} settings"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

[Code]
procedure ShowSupportCode(Sender: TObject);
var
  Dialog: TSetupForm;
  Instructions: TNewStaticText;
  CodeImage: TBitmapImage;
  CloseButton: TNewButton;
begin
  if not FileExists(ExpandConstant('{tmp}\wechat-support.png')) then
    ExtractTemporaryFile('wechat-support.png');

  Dialog := CreateCustomForm(ScaleX(320), ScaleY(365), False, True);
  try
    Dialog.Caption := CustomMessage('SupportAuthor');
    Dialog.Position := poOwnerFormCenter;
    Instructions := TNewStaticText.Create(Dialog);
    Instructions.Parent := Dialog;
    Instructions.Left := ScaleX(12);
    Instructions.Top := ScaleY(12);
    Instructions.Caption := CustomMessage('ScanWithWeChat');

    CodeImage := TBitmapImage.Create(Dialog);
    CodeImage.Parent := Dialog;
    CodeImage.Left := ScaleX(12);
    CodeImage.Top := ScaleY(34);
    CodeImage.Width := ScaleX(296);
    CodeImage.Height := ScaleX(296);
    CodeImage.Stretch := True;
    CodeImage.PngImage.LoadFromFile(ExpandConstant('{tmp}\wechat-support.png'));

    CloseButton := TNewButton.Create(Dialog);
    CloseButton.Parent := Dialog;
    CloseButton.SetBounds(Dialog.ClientWidth - ScaleX(90),
      Dialog.ClientHeight - ScaleY(32), ScaleX(78), ScaleY(24));
    CloseButton.Caption := SetupMessage(msgButtonOK);
    CloseButton.Default := True;
    CloseButton.Cancel := True;
    CloseButton.ModalResult := mrOk;
    Dialog.ShowModal;
  finally
    Dialog.Free;
  end;
end;

procedure InitializeWizard;
var
  SupportButton: TNewButton;
  SupportIcon: TBitmapImage;
begin
  SupportButton := TNewButton.Create(WizardForm);
  SupportButton.Parent := WizardForm.MainPanel;
  SupportButton.SetBounds(WizardForm.MainPanel.ClientWidth - ScaleX(142),
    (WizardForm.MainPanel.ClientHeight - ScaleY(26)) div 2, ScaleX(130), ScaleY(26));
  SupportButton.Anchors := [akTop, akRight];
  SupportButton.Caption := CustomMessage('SupportAuthor');
  SupportButton.OnClick := @ShowSupportCode;

  ExtractTemporaryFile('support-heart.png');
  SupportIcon := TBitmapImage.Create(WizardForm);
  SupportIcon.Parent := WizardForm.MainPanel;
  SupportIcon.SetBounds(SupportButton.Left - ScaleX(27),
    SupportButton.Top + ScaleY(2), ScaleX(22), ScaleX(22));
  SupportIcon.Anchors := [akTop, akRight];
  SupportIcon.Stretch := True;
  SupportIcon.PngImage.LoadFromFile(ExpandConstant('{tmp}\support-heart.png'));
  SupportIcon.Cursor := crHand;
  SupportIcon.Hint := SupportButton.Caption;
  SupportIcon.ShowHint := True;
  SupportIcon.OnClick := @ShowSupportCode;

  WizardForm.PageNameLabel.Width := SupportIcon.Left - WizardForm.PageNameLabel.Left - ScaleX(8);
  WizardForm.PageDescriptionLabel.Width := SupportIcon.Left - WizardForm.PageDescriptionLabel.Left - ScaleX(8);
end;
