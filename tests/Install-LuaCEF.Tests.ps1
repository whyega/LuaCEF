#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$installer = Join-Path $PSScriptRoot '..\installer\Install-LuaCEF.ps1'
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('LuaCEF-installer-tests-' + [Guid]::NewGuid())
$originalUserEnvironment = [Environment]::GetEnvironmentVariable('LUACEF_HOST_PATH', 'User')
$originalProcessEnvironment = [Environment]::GetEnvironmentVariable('LUACEF_HOST_PATH', 'Process')

function Assert-True([bool] $Condition, [string] $Message) {
    if (-not $Condition) { throw $Message }
}

function Write-TestFile([string] $Path, [string] $Content = 'payload') {
    New-Item -ItemType Directory -Path (Split-Path -Parent $Path) -Force | Out-Null
    [IO.File]::WriteAllText($Path, $Content)
}

try {
    $source = Join-Path $testRoot 'release package'
    $game = Join-Path $testRoot ('game [' + [char]0x0422 + [char]0x0435 + [char]0x0441 + [char]0x0442 + ']')
    Write-TestFile (Join-Path $game 'gta_sa.exe')
    foreach ($file in @('luacef.asi', 'moonloader\lib\luacef.lua', 'luacef\cef-host.exe',
        'luacef\cef-subprocess.exe', 'luacef\libcef.dll', 'luacef\chrome_elf.dll',
        'luacef\icudtl.dat', 'luacef\resources.pak', 'luacef\locales\ru.pak')) {
        Write-TestFile (Join-Path $source $file) $file
    }
    Write-TestFile (Join-Path $source 'luacef\cache\old.dat')
    Write-TestFile (Join-Path $source 'luacef\host.log')

    $shared = Join-Path $testRoot 'shared runtime'
    & $installer -GamePath $game -SourcePath $source -RuntimePath $shared -WhatIf
    Assert-True (-not (Test-Path -LiteralPath $shared)) 'WhatIf created a runtime directory'
    Assert-True (-not (Test-Path -LiteralPath (Join-Path $game 'luacef.asi'))) 'WhatIf copied the plugin'

    & $installer -GamePath $game -SourcePath $source -Portable
    foreach ($file in @('luacef.asi', 'moonloader\lib\luacef.lua', 'luacef\cef-host.exe',
        'luacef\cef-subprocess.exe', 'luacef\libcef.dll', 'luacef\locales\ru.pak')) {
        Assert-True ([IO.File]::ReadAllText((Join-Path $game $file)) -eq $file) "Wrong installed file: $file"
    }
    Assert-True (-not (Test-Path -LiteralPath (Join-Path $game 'luacef\cache'))) 'Copied stale cache'
    Assert-True (-not (Test-Path -LiteralPath (Join-Path $game 'luacef\host.log'))) 'Copied stale log'

    Write-TestFile (Join-Path $game 'luacef\cache\keep.dat') 'keep'
    Write-TestFile (Join-Path $source 'luacef\locales\ru.pak') 'updated locale'
    & $installer -GamePath $game -SourcePath $source -Portable
    Assert-True ([IO.File]::ReadAllText((Join-Path $game 'luacef\locales\ru.pak')) -eq 'updated locale') 'Locale update failed'
    Assert-True ([IO.File]::ReadAllText((Join-Path $game 'luacef\cache\keep.dat')) -eq 'keep') 'Update damaged the cache'
    & $installer -GamePath $game -SourcePath $game -Portable

    $badGame = Join-Path $testRoot 'invalid game'
    New-Item -ItemType Directory -Path $badGame | Out-Null
    $rejected = $false
    try { & $installer -GamePath $badGame -SourcePath $source -Portable } catch { $rejected = $true }
    Assert-True $rejected 'A folder without gta_sa.exe was accepted'

    $hostFile = Join-Path $source 'luacef\cef-host.exe'
    Remove-Item -LiteralPath $hostFile
    $rejected = $false
    try { & $installer -GamePath $game -SourcePath $source -Portable } catch { $rejected = $true }
    Assert-True $rejected 'Incomplete package was accepted'
    Write-TestFile $hostFile
    $rejected = $false
    try { & $installer -GamePath $game -SourcePath $source -RuntimePath (Join-Path $source 'luacef\nested') -WhatIf }
    catch { $rejected = $true }
    Assert-True $rejected 'Nested runtime destination was accepted'

    Assert-True ([Environment]::GetEnvironmentVariable('LUACEF_HOST_PATH', 'User') -eq $originalUserEnvironment) 'Tests changed the user environment'
    Assert-True ([Environment]::GetEnvironmentVariable('LUACEF_HOST_PATH', 'Process') -eq $originalProcessEnvironment) 'Tests changed the process environment'
    Write-Host 'All installer tests passed.'
} finally {
    $resolvedRoot = [IO.Path]::GetFullPath($testRoot)
    $temporaryRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\') + '\'
    if ($resolvedRoot.StartsWith($temporaryRoot, [StringComparison]::OrdinalIgnoreCase) -and
        (Split-Path -Leaf $resolvedRoot).StartsWith('LuaCEF-installer-tests-')) {
        Remove-Item -LiteralPath $resolvedRoot -Recurse -Force -ErrorAction SilentlyContinue
    }
}
