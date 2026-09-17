---
status: ok
kind: manual
evidence_files:
  - app/src/main/AndroidManifest.xml
  - platform/android/AndroidMain.cpp
  - platform/android/GlContext.cpp
  - platform/android/AndroidHaptics.cpp
  - engine/audio/Mixer.cpp
  - engine/Engine.cpp
---

# 설계 결정 기록

**이 문서는 사람이 씁니다. 코드로 확인할 수 있는 동작은 여기 적지 않고, 코드만 봐서는 알 수 없는 "왜" 를 적습니다.**

항목 하나는 결정 한 가지입니다. 번호는 바꾸지 않고, 뒤집힌 결정은 지우지 않고 "대체됨" 으로 표시합니다.

## D-001 Kotlin/Java 소스 없이 GameActivity 를 런처로 쓴다

| 항목 | 내용 |
|---|---|
| 결정 | 매니페스트에서 `com.google.androidgamesdk.GameActivity` 를 직접 런처 액티비티로 지정하고, 네이티브 라이브러리 이름은 `android.app.lib_name` 메타데이터로 넘긴다. |
| 이유 | 프로젝트 목적이 "C++ 비중을 최대화한 NDK 실험장" 이다. Activity 서브클래스 한 줄을 위해 Kotlin 툴체인을 끌어오지 않는다. |
| 대안 | NativeActivity (AppCompat 이 아니라 UI 통합이 어렵다), Kotlin 서브클래스 (관례적이지만 목적에 어긋난다). |
| 영향 받는 코드 | `app/src/main/AndroidManifest.xml`, `native/platform/android/AndroidMain.cpp` |
| 근거 수준 | 설계 결정 · 2026-09-17 |
| 상태 | 유효 |

## D-002 EGL context 는 앱 수명 동안 유지하고 surface 만 만들고 지운다

| 항목 | 내용 |
|---|---|
| 결정 | `GlContext` 는 display/context 를 한 번 만들고, `APP_CMD_TERM_WINDOW` 에서는 surface 만 지운다. |
| 이유 | 텍스처 아틀라스와 VBO 가 백그라운드 전환 후에도 살아 있어야 에셋을 다시 로딩하지 않는다. 아틀라스는 로딩 시점에 패킹하므로 재로딩 비용이 크다. |
| 대안 | 매번 context 재생성 + 에셋 재로딩 (단순하지만 복귀가 느리다). |
| 영향 받는 코드 | `native/platform/android/GlContext.cpp`, `native/platform/android/AndroidMain.cpp` |
| 근거 수준 | 설계 결정 · 2026-09-17 |
| 상태 | 유효 |

## D-003 예측 뒤로가기(predictive back)를 끈다

| 항목 | 내용 |
|---|---|
| 결정 | 매니페스트 `android:enableOnBackInvokedCallback="false"`. |
| 이유 | targetSdk 36 에서 예측 뒤로가기가 켜져 있으면 BACK 이 `onKeyUp` 으로 오지 않아 네이티브 쪽이 "게임 중 BACK = 메뉴로" 를 구현할 수 없다. Java 코드가 없으므로 `OnBackInvokedCallback` 을 등록할 방법도 없다. |
| 대안 | Kotlin 으로 콜백 등록 (D-001 위반). |
| 영향 받는 코드 | `app/src/main/AndroidManifest.xml`, `native/platform/android/AndroidMain.cpp` (`processInput`) |
| 근거 수준 | 기기 관찰 · Galaxy S25+ (API 36), 2026-09-17 |
| 상태 | 유효 |

## D-004 효과음은 파일 없이 코드로 합성한다

| 항목 | 내용 |
|---|---|
| 결정 | `engine/audio/Synth` 로 시작 시점에 모든 효과음을 PCM 으로 만들고 `Mixer` 가 AAudio 콜백에서 섞는다. |
| 이유 | 무료 CC0 자산 정책 아래에서 효과음까지 출처를 관리하는 비용을 없애고, 그림체처럼 소리도 한 톤으로 통일한다. |
| 대안 | CC0 효과음 팩 (출처 관리 필요), Oboe (의존성 추가). |
| 영향 받는 코드 | `native/engine/audio/`, `native/app/Sfx.cpp`, `native/platform/android/AndroidAudio.cpp` |
| 근거 수준 | 설계 결정 · 2026-09-17 |
| 상태 | 유효 |

## D-005 믹서는 보이스가 다 차면 소리를 버린다 (빼앗지 않는다)

| 항목 | 내용 |
|---|---|
| 결정 | `Mixer::play` 는 빈 보이스가 없으면 아무것도 하지 않는다. |
| 이유 | 재생 중인 보이스를 게임 스레드가 다시 쓰면 오디오 콜백과 데이터 레이스가 난다. 보이스 12개면 이 게임의 동시 효과음에 충분하다. |
| 대안 | 가장 오래된 보이스 빼앗기 (atomic 위치 관리가 필요). |
| 영향 받는 코드 | `native/engine/audio/Mixer.cpp` |
| 근거 수준 | 코드 리뷰 결정 · 2026-09-17 |
| 상태 | 유효 |

## D-006 미술 자산은 GameArt2D 무료(CC0) 한 계열로 통일한다

| 항목 | 내용 |
|---|---|
| 결정 | 캐릭터, 타일, 오브젝트, GUI 를 전부 GameArt2D 무료 섹션에서 가져오고, 장애물도 타일셋의 오브젝트(상자·돌·버섯)만 쓴다. |
| 이유 | 여러 작가의 자산을 섞으면 그림체가 흩어진다. 초기에 쓴 젤리 팩은 공룡과 어울리지 않아 제거했다. |
| 대안 | itch.io CC0 팩 혼합, AI 생성 이미지 (프레임 간 그림체 불일치). |
| 영향 받는 코드 | `app/src/main/assets/`, `native/app/GameAssets.cpp` |
| 근거 수준 | 설계 결정 · 2026-09-17 |
| 상태 | 유효 |

??? note "근거와 검토 정보"
    - 근거 파일: `evidence_files` frontmatter 참조
    - 근거 수준: 설계 결정 (코드 확인 아님)
    - 검토: 2026-09-17 작성자 검토

다음 단계: [시스템 개요](overview.md)
