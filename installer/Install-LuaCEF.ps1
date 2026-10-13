#Requires -Version 5.1
[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [string] $GamePath,
    [string] $SourcePath = $PSScriptRoot,
    [string] $RuntimePath = [IO.Path]::Combine(
        [Environment]::GetFolderPath('LocalApplicationData'), 'LuaCEF', 'runtime'),
    [switch] $Portable
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($GamePath)) {
    Add-Type -AssemblyName System.Windows.Forms
    $picker = New-Object System.Windows.Forms.FolderBrowserDialog
    $picker.Description = 'Select the GTA San Andreas folder containing gta_sa.exe'
    $picker.ShowNewFolderButton = $false
    try {
        if ($picker.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
            throw 'Installation cancelled.'
        }
        $GamePath = $picker.SelectedPath
    } finally {
        $picker.Dispose()
    }
}

$GamePath = (Resolve-Path -LiteralPath $GamePath).ProviderPath
$SourcePath = (Resolve-Path -LiteralPath $SourcePath).ProviderPath
if (-not (Test-Path -LiteralPath (Join-Path $GamePath 'gta_sa.exe') -PathType Leaf)) {
    throw "gta_sa.exe was not found in: $GamePath"
}

$runtimeSource = Join-Path $SourcePath 'luacef'
$pluginSource = Join-Path $SourcePath 'luacef.asi'
$moduleSource = Join-Path $SourcePath 'moonloader\lib\luacef.lua'
$requiredFiles = @(
    $pluginSource, $moduleSource,
    (Join-Path $runtimeSource 'cef-host.exe'),
    (Join-Path $runtimeSource 'cef-subprocess.exe'),
    (Join-Path $runtimeSource 'libcef.dll'),
    (Join-Path $runtimeSource 'chrome_elf.dll'),
    (Join-Path $runtimeSource 'icudtl.dat'),
    (Join-Path $runtimeSource 'resources.pak')
)
foreach ($requiredFile in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $requiredFile -PathType Leaf)) {
        throw "Incomplete LuaCEF package. Missing file: $requiredFile"
    }
}
$localesSource = Join-Path $runtimeSource 'locales'
if (-not (Test-Path -LiteralPath $localesSource -PathType Container) -or
    @(Get-ChildItem -LiteralPath $localesSource -Filter '*.pak' -File).Count -eq 0) {
    throw "Incomplete LuaCEF package. Missing CEF locales in: $localesSource"
}

if ($Portable) {
    $RuntimePath = Join-Path $GamePath 'luacef'
}
if ($RuntimePath -notmatch '^(?:[A-Za-z]:[\\/]|\\\\[^\\/]+[\\/][^\\/]+)') {
    throw 'RuntimePath must be an absolute directory path.'
}
$RuntimePath = [IO.Path]::GetFullPath($RuntimePath)
if ($RuntimePath -eq [IO.Path]::GetPathRoot($RuntimePath)) {
    throw 'RuntimePath must not be a drive or network share root.'
}
$RuntimePath = $RuntimePath.TrimEnd('\', '/')
$runtimeSource = [IO.Path]::GetFullPath($runtimeSource).TrimEnd('\', '/')
if ($RuntimePath -ne $runtimeSource -and
    ($RuntimePath.StartsWith($runtimeSource + '\', [StringComparison]::OrdinalIgnoreCase) -or
     $runtimeSource.StartsWith($RuntimePath + '\', [StringComparison]::OrdinalIgnoreCase))) {
    throw 'Source and destination runtime directories must not contain each other.'
}

$description = 'Copy the runtime, ASI plugin and MoonLoader module'
if (-not $Portable) {
    $description += ' and set the user environment variable LUACEF_HOST_PATH'
}
if (-not $PSCmdlet.ShouldProcess($GamePath, $description)) {
    return
}

New-Item -ItemType Directory -Path $RuntimePath -Force | Out-Null
if ($RuntimePath -ne $runtimeSource) {
    foreach ($runtimeItem in Get-ChildItem -LiteralPath $runtimeSource -Force) {
        if ($runtimeItem.Name -eq 'cache' -or $runtimeItem.Extension -eq '.log') {
            continue
        }
        Copy-Item -LiteralPath $runtimeItem.FullName -Destination $RuntimePath -Recurse -Force
    }
}
$pluginDestination = Join-Path $GamePath 'luacef.asi'
if ($pluginSource -ne $pluginDestination) {
    Copy-Item -LiteralPath $pluginSource -Destination $pluginDestination -Force
}
$moduleDirectory = Join-Path $GamePath 'moonloader\lib'
New-Item -ItemType Directory -Path $moduleDirectory -Force | Out-Null
$moduleDestination = Join-Path $moduleDirectory 'luacef.lua'
if ($moduleSource -ne $moduleDestination) {
    Copy-Item -LiteralPath $moduleSource -Destination $moduleDestination -Force
}

if (-not $Portable) {
    [Environment]::SetEnvironmentVariable('LUACEF_HOST_PATH', $RuntimePath, 'User')
    $env:LUACEF_HOST_PATH = $RuntimePath

    if (-not ([System.Management.Automation.PSTypeName]'LuaCEF.Installer.EnvironmentNotification').Type) {
        Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
namespace LuaCEF.Installer {
    public static class EnvironmentNotification {
        [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr SendMessageTimeout(
            IntPtr window, uint message, UIntPtr wParam, string lParam,
            uint flags, uint timeout, out UIntPtr result);
    }
}
'@
    }
    $messageResult = [UIntPtr]::Zero
    [LuaCEF.Installer.EnvironmentNotification]::SendMessageTimeout(
        [IntPtr]0xffff, 0x001a, [UIntPtr]::Zero, 'Environment', 2, 5000,
        [ref]$messageResult) | Out-Null
}

Write-Host "LuaCEF installed in: $GamePath"
Write-Host "CEF runtime: $RuntimePath"
Write-Host 'Restart the game and its launcher to pick up the environment variable.'
Write-Host 'An ASI loader and MoonLoader must already be installed in the game.'
