; Inno Setup script - LR FX Gen 2 (Windows 64-bit)
; Memasang:
;   1. Aplikasi Standalone  -> C:\Program Files\LR FX Gen 2\  (+ shortcut Desktop & Start Menu)
;   2. Plugin VST3          -> C:\Program Files\Common Files\VST3\LR FX Gen 2.vst3
; Dipanggil dari workflow GitHub Actions:
;   ISCC /DAppVersion=1.1.0 /DArtDir=<folder build\ReverbFX_artefacts\Release> /DRepoDir=<root repo> installer\LR-FX-Gen2.iss

#ifndef AppVersion
  #define AppVersion "1.1.0"
#endif
#ifndef ArtDir
  #define ArtDir "..\build\ReverbFX_artefacts\Release"
#endif
#ifndef RepoDir
  #define RepoDir ".."
#endif

#define AppName "LR FX Gen 2"

[Setup]
AppId={{6C1F2B7A-4D3E-4F5A-9B1C-7A2E8D0C5F31}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=LR PRO
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputDir={#RepoDir}\dist
OutputBaseFilename=LR-FX-Gen-2-Setup
SetupIconFile={#RepoDir}\Assets\Logo\r_plugin_icon.ico
UninstallDisplayIcon={app}\{#AppName}.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
CloseApplications=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; Aplikasi standalone
Source: "{#ArtDir}\Standalone\{#AppName}.exe"; DestDir: "{app}"; Flags: ignoreversion
; Plugin VST3 (bundle folder) ke folder VST3 standar
Source: "{#ArtDir}\VST3\{#AppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#AppName}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppName}.exe"
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppName}.exe"
Name: "{autoprograms}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\{#AppName}.exe"; Description: "Jalankan {#AppName}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#AppName}.vst3"
