<#
.SYNOPSIS
    SampleOrderSystem 솔루션을 빌드하고, 빌드된 실행 파일(gtest/gmock 러너)을 실행해 테스트 결과를 반환한다.

.PARAMETER Configuration
    빌드 구성 (기본값: Test — gtest/gmock 러너가 빌드되는 구성)

.PARAMETER Platform
    빌드 플랫폼 (기본값: x64)
#>
param(
    [string]$Configuration = "Test",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$repoRoot = $PSScriptRoot
$solutionPath = Join-Path $repoRoot "SampleOrderSystem.slnx"

function Find-MSBuild {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe를 찾을 수 없습니다: $vswhere (Visual Studio 설치 여부를 확인하세요)"
    }

    $msbuildPath = & $vswhere -latest -prerelease -products * `
        -requires Microsoft.Component.MSBuild `
        -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1

    if (-not $msbuildPath -or -not (Test-Path $msbuildPath)) {
        throw "MSBuild.exe를 찾을 수 없습니다. Visual Studio Build Tools가 설치되어 있는지 확인하세요."
    }

    return $msbuildPath
}

function Restore-NuGetPackages {
    $nugetExe = Get-Command "nuget.exe" -ErrorAction SilentlyContinue
    $packagesConfig = Join-Path $repoRoot "SampleOrderSystem\packages.config"

    if ($nugetExe -and (Test-Path $packagesConfig)) {
        Write-Host "NuGet 패키지 복원 중..." -ForegroundColor Cyan
        & $nugetExe.Source restore $packagesConfig -PackagesDirectory (Join-Path $repoRoot "packages")
        if ($LASTEXITCODE -ne 0) {
            throw "NuGet 패키지 복원에 실패했습니다 (exit code $LASTEXITCODE)."
        }
    }
    else {
        Write-Host "nuget.exe를 찾을 수 없어 패키지 복원을 건너뜁니다 (packages 폴더가 이미 존재한다고 가정)." -ForegroundColor Yellow
    }
}

function Build-Solution {
    param([string]$MSBuildPath)

    Write-Host "빌드 시작: Configuration=$Configuration, Platform=$Platform" -ForegroundColor Cyan
    & $MSBuildPath $solutionPath `
        "/p:Configuration=$Configuration" `
        "/p:Platform=$Platform" `
        "/m" `
        "/nologo" `
        "/verbosity:minimal"

    if ($LASTEXITCODE -ne 0) {
        throw "빌드에 실패했습니다 (exit code $LASTEXITCODE)."
    }
}

function Find-TestExecutable {
    $exe = Get-ChildItem -Path $repoRoot -Recurse -Filter "SampleOrderSystem.exe" -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match [regex]::Escape("$Platform\$Configuration") -or $_.FullName -match [regex]::Escape("$Configuration") } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    if (-not $exe) {
        throw "빌드된 실행 파일(SampleOrderSystem.exe)을 찾을 수 없습니다."
    }

    return $exe.FullName
}

function Run-Tests {
    param([string]$ExePath)

    Write-Host "테스트 실행: $ExePath" -ForegroundColor Cyan
    & $ExePath | Out-Host
    return $LASTEXITCODE
}

if (-not (Test-Path $solutionPath)) {
    throw "솔루션 파일을 찾을 수 없습니다: $solutionPath"
}

Restore-NuGetPackages
$msbuild = Find-MSBuild
Build-Solution -MSBuildPath $msbuild
$testExe = Find-TestExecutable
$testExitCode = Run-Tests -ExePath $testExe

if ($testExitCode -eq 0) {
    Write-Host "모든 테스트를 통과했습니다." -ForegroundColor Green
}
else {
    Write-Host "테스트 실패 (exit code $testExitCode)." -ForegroundColor Red
}

exit $testExitCode
