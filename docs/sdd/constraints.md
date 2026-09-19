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
  - app/castle/CastleScene.cpp
---

# 변경 시 지켜야 할 제약

**이 문서는 사람이 씁니다. 파이프라인은 덮어쓰지 않고, `evidence_files` 에 적힌 파일이 바뀌면 영향 보고에 "재검토" 로 올립니다.**

아래 항목마다 근거 수준을 적습니다. 코드로 확인한 규칙은 `파일:줄` 을, 설계 결정은 [설계 결정 기록](decisions.md)의 항목 번호를, 기기 관찰은 기기와 날짜를 적습니다.

## 의존 방향: engine 은 app 과 platform 을 모른다

`native/engine/` 은 `app/` 이나 `platform/` 헤더를 include 하지 않습니다. Activity·AAudio·JNI 연결은 `platform/android/`에 둡니다. 다만 engine의 그래픽 구현은 GLES에, `engine/core/Log.h`는 Android 로그 API에 의존하므로 다른 플랫폼으로 옮길 때 교체가 필요합니다. 근거: [설계 결정 D-001](decisions.md), `native/CMakeLists.txt` 의 디렉터리 구성.

## Scene 은 콜백 안에서 자기 자신을 바꾸지 않는다

현재 앱은 `CastleScene` 하나에서 준비·방어·결과 상태를 전환합니다. 입력 콜백에서 Scene 객체를 파괴하지 않습니다. 근거: `native/app/castle/CastleScene.cpp`, `native/app/castle/CastleModel.cpp`.

## 오디오 콜백이 만지는 것에는 게임 스레드가 쓰지 않는다

`Mixer::play` 는 `sound == nullptr` 인 보이스만 채우고, 콜백은 보이스를 반납한 뒤(`store(nullptr)`) 그 보이스에 아무것도 쓰지 않습니다. `Sound` 객체는 재생 중인 보이스보다 오래 살아야 합니다 (`CastleScene`이 `Sfx`를 소유하고, `AndroidAudio` 는 `Engine` 보다 먼저 파괴됩니다). 근거: `native/engine/audio/Mixer.cpp`, `native/platform/android/AndroidMain.cpp` 의 `AppState` 멤버 순서, [D-005](decisions.md).

## JNI 를 쓰는 곳은 AndroidHaptics 하나뿐이고, attach 했으면 detach 한다

게임 스레드는 `native_app_glue` 가 JVM 에 attach 하지 않습니다. JNI 가 필요하면 `AndroidHaptics` 처럼 attach 여부를 기억했다가 스레드가 끝나기 전에 `DetachCurrentThread` 를 호출해야 합니다. 그러지 않으면 `android_main` 이 반환될 때 ART 가 프로세스를 abort 합니다. 근거: `native/platform/android/AndroidHaptics.cpp`, 기기 관찰 (Galaxy S25+, 2026-09-17).

## 아틀라스 크기는 기기의 최대 텍스처 크기를 따른다

Engine은 가능한 기기에서 4096, 그 외에는 2048 크기의 아틀라스를 사용합니다. `TextureAtlas` 는 로딩 시점에 shelf packing 을 하며, 가득 차면 `add` 가 false 를 돌려주고 로그만 남깁니다. 큰 배경은 아틀라스가 아니라 별도 `Texture` 로 둡니다. 새 스프라이트를 추가하면 logcat 에서 `atlas full` 이 없는지 확인합니다. 근거: `native/engine/graphics/TextureAtlas.cpp`, `native/app/castle/CastleScene.cpp`.

## HUD 는 safeTop 아래에 그린다

카메라 홀과 상태바 inset 은 `Engine::safeTop()` 으로 제공됩니다. 화면 상단에 붙는 텍스트나 버튼은 `y = safeTop() + 여백` 으로 시작해야 합니다. 근거: `native/engine/Engine.h`, `native/platform/android/AndroidMain.cpp` 의 `applyInsets`.

## 자산 출처와 라이선스는 CREDITS.md에 기록한다

캐릭터와 UI는 코드로 그린 독자적인 픽셀 패턴을 사용합니다. 한국어 글꼴은 OFL 1.1의 Noto Sans KR을 사용하며 라이선스 원문과 가공 방법을 함께 둡니다. 새 자산의 출처는 `app/src/main/assets/CREDITS.md`에 기록합니다. 근거: [D-007](decisions.md).

## 변경 후 검증

1. `./gradlew assembleDebug`로 ARM64와 x86_64를 빌드합니다.
2. `tools/test-castle.ps1`로 합성·경제·저장·10일 캠페인을 검증합니다.
3. 앱에서 먹이 → 합성 미리보기 → 합성 → 도감 → 방어 → 보상 → 재실행을 확인합니다.
4. BACK은 합성창 닫기, 배치 취소, 방어 일시정지, 준비 화면에서 종료의 순서로 처리합니다.
5. 로그에서 GL·폰트·오디오 오류를 확인하고 `tools/docs-check.sh`로 문서를 검사합니다.
??? note "근거와 검토 정보"
    - 근거 파일: `evidence_files` frontmatter 참조
    - 근거 수준: 항목마다 표시
    - 검토: 2026-09-17 작성자 검토

다음 단계: [설계 결정 기록](decisions.md)
