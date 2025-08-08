param(
    [string]$Config = "Release",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Arch = "x64",
    [string]$SourceDir = (Resolve-Path "$PSScriptRoot\..").Path,
    [string]$BuildDir = (Join-Path (Resolve-Path "$PSScriptRoot\..").Path "build")
)

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
Set-Location $BuildDir

$cmakeConfigure = @(
    "-G", "$Generator",
    "-A", "$Arch",
    "-DCMAKE_BUILD_TYPE=$Config",
    "$SourceDir"
)

& cmake @cmakeConfigure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$cmakeBuild = @(
    "--build", ".",
    "--config", "$Config",
    "-j"
)

& cmake @cmakeBuild
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Build completed. Binaries:"
Write-Host "Editor: $BuildDir\bin\GameEngineEditor.exe"
Write-Host "Demos:  $BuildDir\demo\GameEngineDemo.exe"
Write-Host "        $BuildDir\demo\GameObjectDemo.exe"
