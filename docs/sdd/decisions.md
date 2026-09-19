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
| 상태 | D-007로 대체됨 (2026-09-19) |

??? note "근거와 검토 정보"
    - 근거 파일: `evidence_files` frontmatter 참조
    - 근거 수준: 설계 결정 (코드 확인 아님)
    - 검토: 2026-09-17 작성자 검토

다음 단계: [시스템 개요](overview.md)

## D-007 몬스터 수집·성장·합성 중심으로 전환한다

2026-09-19 사용자가 미니게임 모음의 방향 전환을 요청했고, 참고작의 요소 중 몬스터 수집·성장·합성을 선택했습니다. 기존 엔진은 유지하고 기본 진입점을 CastleScene으로 바꿉니다. CastleModel은 그래픽 의존성을 두지 않아 실제 기기에서 시뮬레이션 테스트를 돌릴 수 있게 합니다. 이전 미니게임 코드는 빌드 대상에서 제외하여 보존합니다.

새 캐릭터는 코드로 그린 독자적인 픽셀 패턴을 사용합니다. 한국어 지원을 위해 OFL 1.1의 Noto Sans KR을 도입하며, 필요한 글자만 추출하고 굵기 600으로 고정해 가독성과 APK 크기를 함께 개선합니다. 기존 CC0 미술 자산 계열에 한정하던 D-006을 대체합니다.

## D-008 합성 결과를 미리 보여 주고 발견 이력을 보존한다

합성은 두 개체를 하나로 바꾸는 행동이므로 결과 종·유지되는 레벨·마력 비용을 확인 화면에 표시합니다. 무작위 합성을 쓰지 않으며 레벨 평균을 계승합니다. 사용한 개체가 없어져도 발견한 종은 도감에 남깁니다. 방어전 패배로 개체를 삭제하지 않아 수집을 다시 시작해야 하는 부담을 줄입니다.

## D-009 전투 중 종료는 준비 단계로 복원한다

배치·성장·합성·결과 보상을 원자적 파일 교체로 저장합니다. 중간 전투는 저장하지 않아 재개 시 체력·타이머·피해가 어긋나지 않도록 하고, 결과 보상과 다음 날을 함께 기록하여 중복 보상을 막습니다. 기존 최고 점수 파일과는 저장 경로가 다릅니다.

## D-010 몬스터 외형을 셀 채색 일러스트로 바꾼다

2026-09-19 사용자는 드래곤볼 Z풍의 작화를 가진 비인간형 몬스터를 요청했습니다. 기존 6종의 이름과 게임 규칙은 유지하며 코드로 그리던 픽셀 패턴을 투명 이미지 시트로 대체합니다. 사람이나 원작 인물로 변경하지 않습니다. 굵은 선·각진 표정·괴수 실루엣·셀 채색으로 화면에서 종을 구별할 수 있도록 하고, 공용 아틀라스를 사용해 매 프레임 이미지 로드나 픽셀별 사각형 생성을 피합니다. D-007의 게임 방향은 유지하며 코드 픽셀 미술 부분만 이 결정으로 대체합니다.
