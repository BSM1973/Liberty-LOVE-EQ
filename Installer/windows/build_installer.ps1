# Windows x64 LOVE EQ installer, same Inno Setup pipeline as Pedal Legends.
[CmdletBinding()]
param([string]$Version = "0.1.0")
$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$artifactRoot = Join-Path $root "build/LoveEQ_artefacts/Release"
$vst3Folder = Join-Path $artifactRoot "VST3"
$standaloneFolder = Join-Path $artifactRoot "Standalone"
if (-not (Test-Path -LiteralPath $vst3Folder -PathType Container)) {
    throw "Missing LOVE EQ VST3 output: $vst3Folder"
}
$bundles = @(Get-ChildItem -LiteralPath $vst3Folder -Directory -Filter "*.vst3")
if ($bundles.Count -ne 1) {throw "Expected one VST3 bundle, found $($bundles.Count)"}
$bundle = $bundles[0]
$binary = @(Get-ChildItem -LiteralPath $bundle.FullName -Recurse -File -Filter "*.vst3")
if ($binary.Count -ne 1) {throw "Expected one compiled VST3 file, found $($binary.Count)"}
$exeList = @(Get-ChildItem -LiteralPath $standaloneFolder -File -Filter "*.exe")
if ($exeList.Count -ne 1) {throw "Expected one Windows Standalone executable, found $($exeList.Count)"}
$standalone = $exeList[0]
$output = Join-Path $root "release-output"
New-Item -ItemType Directory -Force -Path $output | Out-Null
$env:LOVE_EQ_VERSION = $Version
$env:LOVE_EQ_VST3_BUNDLE = $bundle.FullName
$env:LOVE_EQ_STANDALONE_EXE = $standalone.FullName
$env:LOVE_EQ_INSTALLER_OUTPUT = $output
$paths = @()
if ($env:ProgramFiles) { $paths += Join-Path $env:ProgramFiles "Inno Setup 6/ISCC.exe" }
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
if ($programFilesX86) { $paths += Join-Path $programFilesX86 "Inno Setup 6/ISCC.exe" }
$iscc = $null
foreach($candidate in $paths) {if(Test-Path -LiteralPath $candidate){$iscc=$candidate;break}}
if (-not $iscc) {
    $cmd=Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($cmd) {$iscc=$cmd.Source}
}
if (-not $iscc) {
    choco install innosetup --yes --no-progress --limit-output
    if($LASTEXITCODE -ne 0){throw "Inno Setup installation failed"}
    foreach($candidate in $paths){if(Test-Path -LiteralPath $candidate){$iscc=$candidate;break}}
}
if (-not $iscc){throw "Cannot find Inno Setup 6 ISCC.exe"}
$iss = Join-Path $PSScriptRoot "LoveEQ.iss"
& $iscc $iss
if($LASTEXITCODE -ne 0){throw "Inno Setup compilation failed"}
$installer = Join-Path $output "LOVE-EQ-v$Version-Windows-x64-Setup.exe"
if(-not (Test-Path -LiteralPath $installer -PathType Leaf)){throw "Missing $installer"}
if((Get-Item -LiteralPath $installer).Length -lt 100000){throw "Suspiciously small .exe installer"}
Get-FileHash -Algorithm SHA256 -LiteralPath $installer | Format-List
Write-Host "LOVE EQ Windows VST3 + Standalone installer: $installer"
