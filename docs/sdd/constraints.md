---
status: ok
kind: manual
evidence_files:
  - engine/Engine.h
  - engine/Scene.h
  - engine/audio/Mixer.cpp
  - engine/graphics/TextureAtlas.cpp
  - platform/android/AndroidHaptics.cpp
  - platform/android/AndroidMain.cpp
  - app/MiniGameApp.cpp
---

# 변경 시 지켜야 할 제약

**이 문서는 사람이 씁니다. 파이프라인은 덮어쓰지 않고, `evidence_files` 에 적힌 파일이 바뀌면 영향 보고에 "재검토" 로 올립니다.**

아래 항목마다 근거 수준을 적습니다. 코드로 확인한 규칙은 `파일:줄` 을, 설계 결정은 [설계 결정 기록](decisions.md)의 항목 번호를, 기기 관찰은 기기와 날짜를 적습니다.

## 의존 방향: engine 은 app 과 platform 을 모른다

`native/engine/` 은 `app/` 이나 `platform/` 헤더를 include 하지 않습니다. Activity·AAudio·JNI 연결은 `platform/android/`에 둡니다. 다만 현재 engine의 그래픽 구현은 GLES에, `engine/core/Log.h`는 Android 로그 API에 의존하므로 다른 플랫폼으로 옮길 때 교체가 필요합니다. 근거: [설계 결정 D-001](decisions.md), `native/CMakeLists.txt` 의 디렉터리 구성.

## Scene 은 콜백 안에서 자기 자신을 바꾸지 않는다

메뉴 카드나 결과 버튼의 콜백은 `MiniGameApp::switchTo` 로 다음 Scene 을 `pending_` 에 넣기만 하고, 실제 교체는 다음 `update()` 시작 시점에 합니다. 콜백 안에서 `current_` 를 파괴하면 호출 중인 객체가 사라집니다. 근거: `native/app/MiniGameApp.cpp` (`switchTo`, `update`).

## 오디오 콜백이 만지는 것에는 게임 스레드가 쓰지 않는다

`Mixer::play` 는 `sound == nullptr` 인 보이스만 채우고, 콜백은 보이스를 반납한 뒤(`store(nullptr)`) 그 보이스에 아무것도 쓰지 않습니다. `Sound` 객체는 재생 중인 보이스보다 오래 살아야 합니다 (`GameAssets` 가 `Sfx` 를 소유하고, `AndroidAudio` 는 `Engine` 보다 먼저 파괴됩니다). 근거: `native/engine/audio/Mixer.cpp`, `native/platform/android/AndroidMain.cpp` 의 `AppState` 멤버 순서, [D-005](decisions.md).

## JNI 를 쓰는 곳은 AndroidHaptics 하나뿐이고, attach 했으면 detach 한다

게임 스레드는 `native_app_glue` 가 JVM 에 attach 하지 않습니다. JNI 가 필요하면 `AndroidHaptics` 처럼 attach 여부를 기억했다가 스레드가 끝나기 전에 `DetachCurrentThread` 를 호출해야 합니다. 그러지 않으면 `android_main` 이 반환될 때 ART 가 프로세스를 abort 합니다. 근거: `native/platform/android/AndroidHaptics.cpp`, 기기 관찰 (Galaxy S25+, 2026-09-17).

## 아틀라스 크기는 GPU 한도에 맞춘다

`Engine::initGraphics`는 GPU 한도가 허용하면 4096, 아니면 2048 크기의 아틀라스를 만듭니다. `TextureAtlas` 는 로딩 시점에 shelf packing 을 하며, 가득 차면 `add` 가 false 를 돌려주고 로그만 남깁니다. 큰 배경은 아틀라스가 아니라 별도 `Texture` 로 둡니다. 새 스프라이트를 추가하면 logcat 에서 `atlas full` 이 없는지 확인합니다. 근거: `native/engine/graphics/TextureAtlas.cpp`, `native/app/GameAssets.cpp`.

## HUD 는 safeTop 아래에 그린다

카메라 홀과 상태바 inset 은 `Engine::safeTop()` 으로 제공됩니다. 화면 상단에 붙는 텍스트나 버튼은 `y = safeTop() + 여백` 으로 시작해야 합니다. 근거: `native/engine/Engine.h`, `native/platform/android/AndroidMain.cpp` 의 `applyInsets`.

## 미술 자산과 폰트의 출처는 CREDITS.md 에

미술 자산은 D-006의 CC0 정책을 따르며, 기존 Fredoka 폰트는 CREDITS.md에 SIL OFL 1.1로 기재되어 있습니다. 새 이미지·폰트를 추가할 때는 `app/src/main/assets/CREDITS.md` 에 출처와 라이선스를 적습니다. 유료 자산과 AI 생성 이미지는 쓰지 않습니다. 근거: [D-006](decisions.md).

## 변경 후 검증

1. `./gradlew assembleDebug` 후 기기에 설치해 메뉴 → 게임 3종 → 결과 → 홈 한 바퀴를 돌립니다.
2. BACK 키가 게임 중에는 메뉴로, 메뉴에서는 종료로 동작하는지 확인합니다.
3. `adb logcat -s AndroidMain Engine GameAssets AndroidAudio AndroidHaptics` 에서 오류가 없는지 봅니다.
4. `tools/docs-check.sh` 로 문서가 코드와 맞는지 확인하고, 필요하면 `tools/docs-sync.sh` 를 돌립니다.

??? note "근거와 검토 정보"
    - 근거 파일: `evidence_files` frontmatter 참조
    - 근거 수준: 항목마다 표시
    - 검토: 2026-09-17 작성자 검토

다음 단계: [설계 결정 기록](decisions.md)
