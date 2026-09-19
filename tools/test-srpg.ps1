param(
    [Parameter(Mandatory=$true)][string]$Device,
    [ValidateSet('arm64-v8a','x86_64')][string]$Abi='x86_64',
    [string]$Sdk="$env:LOCALAPPDATA\Android\Sdk"
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
$compiler=Join-Path $Sdk 'ndk\29.0.14206865\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe'
$adb=Join-Path $Sdk 'platform-tools\adb.exe'
$target=if($Abi -eq 'arm64-v8a') {'aarch64-linux-android28'} else {'x86_64-linux-android28'}
Push-Location $projectRoot
try {
    New-Item -ItemType Directory -Force build | Out-Null
    & $compiler "--target=$target" -static-libstdc++ -std=c++20 -Wall -Wextra -Wpedantic -O2 -I native native/tests/BattleModelTests.cpp native/app/srpg/BattleModel.cpp -o build/srpg-model-tests
    if($LASTEXITCODE -ne 0) { throw 'SRPG compilation failed' }
    & $adb -s $Device push build/srpg-model-tests /data/local/tmp/srpg-model-tests
    if($LASTEXITCODE -ne 0) { throw 'Test upload failed' }
    & $adb -s $Device shell chmod 755 /data/local/tmp/srpg-model-tests
    if($LASTEXITCODE -ne 0) { throw 'Test permissions failed' }
    & $adb -s $Device shell /data/local/tmp/srpg-model-tests /data/local/tmp
    if($LASTEXITCODE -ne 0) { throw 'SRPG model tests failed' }
    & $compiler "--target=$target" -static-libstdc++ -std=c++20 -Wall -Wextra -Wpedantic -O2 -I native native/tests/PresentationTests.cpp -o build/srpg-presentation-tests
    if($LASTEXITCODE -ne 0) { throw 'Presentation compilation failed' }
    & $adb -s $Device push build/srpg-presentation-tests /data/local/tmp/srpg-presentation-tests
    if($LASTEXITCODE -ne 0) { throw 'Presentation upload failed' }
    & $adb -s $Device shell chmod 755 /data/local/tmp/srpg-presentation-tests
    & $adb -s $Device shell /data/local/tmp/srpg-presentation-tests
    if($LASTEXITCODE -ne 0) { throw 'Presentation tests failed' }
    $engineSources = @(Get-ChildItem native/engine -Recurse -Filter '*.cpp' | ForEach-Object FullName)
    & $compiler "--target=$target" -static-libstdc++ -std=c++20 -Wall -Wextra -Wpedantic -Wno-unused-function -O2 -I native native/tests/BattleSceneTests.cpp native/app/srpg/BattleScene.cpp native/app/srpg/BattleArt.cpp native/app/srpg/BattleModel.cpp native/app/Sfx.cpp @engineSources -lGLESv3 -llog -o build/srpg-scene-tests
    if($LASTEXITCODE -ne 0) { throw 'SRPG scene compilation failed' }
    & $adb -s $Device push build/srpg-scene-tests /data/local/tmp/srpg-scene-tests
    if($LASTEXITCODE -ne 0) { throw 'Scene test upload failed' }
    & $adb -s $Device shell chmod 755 /data/local/tmp/srpg-scene-tests
    if($LASTEXITCODE -ne 0) { throw 'Scene test permissions failed' }
    & $adb -s $Device shell /data/local/tmp/srpg-scene-tests /data/local/tmp
    if($LASTEXITCODE -ne 0) { throw 'SRPG scene tests failed' }
} finally { Pop-Location }
