; LOVE EQ - Windows x64 VST3 and Standalone, same builder as Pedal Legends.
#define ProductVersion GetEnv("LOVE_EQ_VERSION")
#define PluginSource GetEnv("LOVE_EQ_VST3_BUNDLE")
#define StandaloneSource GetEnv("LOVE_EQ_STANDALONE_EXE")
#define InstallerOutput GetEnv("LOVE_EQ_INSTALLER_OUTPUT")
[Setup]
AppId=LibertyMusicSoftware.LoveEQ
AppName=LOVE EQ
AppVersion={#ProductVersion}
AppVerName=LOVE EQ {#ProductVersion}
AppPublisher=Liberty Music Software
AppPublisherURL=https://libertymusicsoftware.com
AppSupportURL=https://libertymusicsoftware.com
DefaultDirName={autopf}\Liberty Music Software\LOVE EQ
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
OutputDir={#InstallerOutput}
OutputBaseFilename=LOVE-EQ-v{#ProductVersion}-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
SetupLogging=yes
Uninstallable=yes
UsePreviousAppDir=no
[Files]
Source: "{#PluginSource}\*"; DestDir: "{commoncf64}\VST3\LOVE EQ.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#StandaloneSource}"; DestDir: "{app}"; DestName: "LOVE EQ.exe"; Flags: ignoreversion
[Icons]
Name: "{autoprograms}\Liberty Music Software\LOVE EQ"; Filename: "{app}\LOVE EQ.exe"
[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"
