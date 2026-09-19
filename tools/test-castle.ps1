param(
    [Parameter(Mandatory=$true)][string]$Device,
    [ValidateSet('arm64-v8a','x86_64')][string]$Abi='arm64-v8a',
    [string]$Sdk="$env:LOCALAPPDATA\Android\Sdk"
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$compiler=Join-Path $Sdk 'ndk\29.0.14206865\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe'
$adb=Join-Path $Sdk 'platform-tools\adb.exe'
$target=if($Abi -eq 'arm64-v8a') {'aarch64-linux-android28'} else {'x86_64-linux-android28'}
Push-Location $root
try {
    New-Item -ItemType Directory -Force build | Out-Null
    & $compiler "--target=$target" -static-libstdc++ -std=c++20 -Wall -Wextra -Wpedantic -O2 -I native native/tests/CastleModelTests.cpp native/app/castle/CastleModel.cpp -o build/castle-model-tests
    if($LASTEXITCODE -ne 0) { throw 'Compilation failed' }
    & $adb -s $Device push build/castle-model-tests /data/local/tmp/castle-model-tests
    if($LASTEXITCODE -ne 0) { throw 'Test upload failed' }
    & $adb -s $Device shell chmod 755 /data/local/tmp/castle-model-tests
    & $adb -s $Device shell /data/local/tmp/castle-model-tests /data/local/tmp
    if($LASTEXITCODE -ne 0) { throw 'Castle model tests failed' }
    $engineSources = @(Get-ChildItem native/engine -Recurse -Filter '*.cpp' | ForEach-Object FullName)
    & $compiler "--target=$target" -static-libstdc++ -std=c++20 -Wall -Wextra -Wpedantic -Wno-unused-function -O2 -I native native/tests/CastleSceneTests.cpp native/app/castle/CastleScene.cpp native/app/castle/CastleModel.cpp native/app/Sfx.cpp @engineSources -lGLESv3 -llog -o build/castle-scene-tests
    if($LASTEXITCODE -ne 0) { throw 'Scene test compilation failed' }
    & $adb -s $Device push build/castle-scene-tests /data/local/tmp/castle-scene-tests
    if($LASTEXITCODE -ne 0) { throw 'Scene test upload failed' }
    & $adb -s $Device shell chmod 755 /data/local/tmp/castle-scene-tests
    & $adb -s $Device shell /data/local/tmp/castle-scene-tests /data/local/tmp
    if($LASTEXITCODE -ne 0) { throw 'Castle scene tests failed' }
} finally { Pop-Location }
