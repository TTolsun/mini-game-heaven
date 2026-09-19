---
generated_at: 2026-09-17T14:39:04+00:00
source_commit: b4e016048393b0285580f4897dc534a99b59117d
agent: ollama/qwen3.5:4b
status: ok
section: overview
facts_omitted:
  - functions
  - class:AppState
  - class:app::DodgeGame
  - class:app::DodgeGame::Falling
  - class:app::DodgeGame::Kind
  - class:app::GameAssets
  - class:app::GameId
  - class:app::GameInfo
  - class:app::HighScores
  - class:app::IMiniGame
  - class:app::JumpGame
  - class:app::JumpGame::Column
  - class:app::JumpGame::DeathCause
  - class:app::MenuScene
  - class:app::MiniGameApp
---

# 시스템 개요

**수정할 기능이 engine / app / platform 중 어느 패키지에 있는지 아래 표에서 먼저 찾으세요.**

| 지금 확인할 내용 | 이동할 절 |
|---|---|
| 수정할 패키지를 찾습니다. | [패키지별 역할](#패키지별-역할) |
| Android 에서 네이티브로 들어오는 진입점을 찾습니다. | [Android 진입점](#android-진입점) |
| 코드를 처음 읽습니다. | [코드를 처음 읽는 순서](#코드를-처음-읽는-순서) |
| 클래스 단위 책임을 확인합니다. | [컴포넌트 구조와 책임](components.md) |

## 패키지별 역할

| 패키지 | 클래스 수 | 대표 클래스 |
|---|---|---|
| `app` | 20 | `app::Sfx`, `app::GameAssets`, `app::IMiniGame`, `app::GameId`, `app::GameInfo` |
| `engine` | 29 | `engine::Vec2`, `engine::TouchEvent`, `engine::TouchEvent::Phase`, `engine::Scene`, `engine::Sound` |
| `platform` | 5 | `platform::android::AndroidAssetLoader`, `platform::android::AndroidAudio`, `platform::android::AndroidHaptics`, `platform::android::GlContext`, `AppState` |

## 패키지 사이의 의존

| 방향 | 관계 종류 | 관계 수 |
|---|---|---|
| `app` → `engine` | association | 38 |
| `app` → `engine` | inheritance | 4 |
| `platform` → `engine` | association | 2 |
| `platform` → `engine` | inheritance | 2 |

## Android 진입점

| 함수 | 위치 | 설명 |
|---|---|---|
| `android_main()` | `platform/android/AndroidMain.cpp:156` | 확인 필요 |
| `app::allGames()` | `app/GameRegistry.cpp:9` | Adding a game: implement IMiniGame, then register it in GameRegistry.cpp. |
| `app::createGame()` | `app/GameRegistry.cpp:27` | 확인 필요 |
| `handleAppCmd()` | `platform/android/AndroidMain.cpp:54` | 확인 필요 |
| `processInput()` | `platform/android/AndroidMain.cpp:113` | 확인 필요 |

## 코드를 처음 읽는 순서

1. `platform/android/AndroidMain.cpp`의 `android_main`과 `handleAppCmd`에서 앱 수명과 루프를 읽습니다.
2. `engine/Engine.cpp`의 `frame`과 `onTouch`에서 루트 장면으로 전달하는 지점을 읽습니다.
3. `app/MiniGameApp.cpp`의 `update`와 `switchTo`에서 지연된 장면 교체를 읽습니다.
4. `app/GameRegistry.cpp`에서 게임 ID와 구현의 연결을 확인하고 해당 게임 코드를 읽습니다.

## 구조 설명

`engine`은 장면·그래픽·입력·오디오의 공통 기능을 제공하며 app과 platform 헤더에 의존하지 않습니다. `app`은 engine을 사용해 메뉴, 게임과 결과 장면을 구현합니다. `platform/android`는 Android 수명 이벤트와 입출력을 연결하며 `AndroidMain.cpp`에서 app의 루트 장면을 생성합니다. 위 관계 표는 추출된 클래스 관계만 나타내므로 include 의존성을 모두 표현하지 않습니다.

네이티브 진입점은 `android_main`입니다 (`platform/android/AndroidMain.cpp:156`). `Engine`은 루트 장면 하나를 구동하고 `MiniGameApp`은 그 안에서 메뉴·게임·결과 장면을 관리합니다. 프레임 업데이트와 렌더링 후 Android 메인 루프가 버퍼를 교환합니다. 음원 혼합은 별도의 AAudio 데이터 콜백에서 처리합니다.

시나리오 다이어그램은 정적으로 찾은 호출 후보를 보여 줍니다. 가상 함수의 후보를 실제 실행 경로로 확정하거나, 람다 본문을 등록 시점에 실행하는 것으로 해석하면 안 됩니다. 각 시나리오의 코드 대조 설명과 한계를 함께 읽으세요.

??? note "근거와 검토 정보"
    - 근거 파일: `app/GameAssets.cpp`, `app/GameAssets.h`, `app/GameRegistry.cpp`, `app/GameRegistry.h`, `app/HighScores.cpp`, `app/HighScores.h`, `app/IMiniGame.h`, `app/MiniGameApp.cpp`, `app/MiniGameApp.h`, `app/Sfx.cpp`, `app/Sfx.h`, `app/games/DodgeGame.cpp`, `app/games/DodgeGame.h`, `app/games/JumpGame.cpp`, `app/games/JumpGame.h`, `app/games/TapGame.cpp`, `app/games/TapGame.h`, `app/scenes/MenuScene.cpp`, `app/scenes/MenuScene.h`, `app/scenes/ResultScene.cpp`, `app/scenes/ResultScene.h`, `app/ui/Button.h`, `app/ui/Popups.h`, `engine/Engine.cpp`, `engine/Engine.h`, `engine/Scene.h`, `engine/asset/AssetLoader.h`, `engine/asset/Image.cpp`, `engine/asset/Image.h`, `engine/audio/Mixer.cpp`, `engine/audio/Mixer.h`, `engine/audio/Sound.h`, `engine/core/Timer.h`, `engine/graphics/Animation.cpp`, `engine/graphics/Animation.h`, `engine/graphics/Font.cpp`, `engine/graphics/Font.h`, `engine/graphics/Particles.cpp`, `engine/graphics/Particles.h`, `engine/graphics/Shader.cpp`, `engine/graphics/Shader.h`, `engine/graphics/Sprite.h`, `engine/graphics/SpriteBatch.cpp`, `engine/graphics/SpriteBatch.h`, `engine/graphics/Texture.cpp`, `engine/graphics/Texture.h`, `engine/graphics/TextureAtlas.cpp`, `engine/graphics/TextureAtlas.h`, `engine/input/TouchEvent.h`, `engine/math/Color.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`, `engine/platform/Haptics.h`, `platform/android/AndroidAssetLoader.cpp`, `platform/android/AndroidAssetLoader.h`, `platform/android/AndroidAudio.cpp`, `platform/android/AndroidAudio.h`, `platform/android/AndroidHaptics.cpp`, `platform/android/AndroidHaptics.h`, `platform/android/AndroidMain.cpp`, `platform/android/GlContext.cpp`, `platform/android/GlContext.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `b4e0160483`)
    - 인용 검증: 통과
    - 입력 예산 때문에 제외된 사실: functions, class:AppState, class:app::DodgeGame, class:app::DodgeGame::Falling, class:app::DodgeGame::Kind, class:app::GameAssets, class:app::GameId, class:app::GameInfo, class:app::HighScores, class:app::IMiniGame, class:app::JumpGame, class:app::JumpGame::Column, class:app::JumpGame::DeathCause, class:app::MenuScene, class:app::MiniGameApp
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [컴포넌트 구조와 책임](components.md)
