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
| Android 에서 네이티브로 들어오는 진입점을 찾습니다. | [HAL 진입점](#hal-진입점) |
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

## HAL 진입점

| 함수 | 위치 | 설명 |
|---|---|---|
| `android_main()` | `platform/android/AndroidMain.cpp:156` | 확인 필요 |
| `app::allGames()` | `app/GameRegistry.cpp:9` | Adding a game: implement IMiniGame, then register it in GameRegistry.cpp. |
| `app::createGame()` | `app/GameRegistry.cpp:27` | 확인 필요 |
| `handleAppCmd()` | `platform/android/AndroidMain.cpp:54` | 확인 필요 |
| `processInput()` | `platform/android/AndroidMain.cpp:113` | 확인 필요 |

## 코드를 처음 읽는 순서

1. `engine/Engine.cpp` 에서 `tick()` 부분을 읽습니다.
2. `app/games/DodgeGame.cpp` 에서 `update()` 부분을 읽습니다.
3. `engine/graphics/Particles.cpp` 에서 `operator*=()` 부분을 읽습니다.
4. `engine/math/Vec2.h` 에서 `Vec2()` 부분을 읽습니다.
5. `engine/graphics/Animation.cpp` 에서 `duration()` 부분을 읽습니다.
6. `engine/graphics/Sprite.h` 에서 `Vec2()` 부분을 읽습니다.

이 순서는 `한 프레임 (Engine::frame)` 시나리오의 호출 경로에서 만들었습니다. 자세한 흐름은 시나리오 문서 [한 프레임 (Engine::frame)](scenarios/frame.md) 에서 확인하세요.

## 구조 설명

이 게임 엔진의 코드는 `app`, `engine`, `platform` 세 패키지로 나뉘며 각자는 명확한 책임을 맡고 있습니다. `app` 패키지는 20 개의 클래스로 게임 로직과 UI 를 담당하고, `engine` 패키지는 29 개의 클래스로 그래픽, 오디오, 입력 등 플랫폼 독립적인 핵심 기능을 제공합니다. `platform` 패키지는 5 개의 클래스로 Android 네이티브 코드를 직접 호출하는 진입점을 구성합니다.

패키지 간의 의존 관계는 일방향이며, `app` 는 `engine` 을 참조하고 `platform` 은 `engine` 과 직접 상호작용합니다. `app` 에서 `engine` 으로 가는 association 와 inheritance 관계가 총 42 개로 기록되어 있으며, 이는 게임이 엔진의 그래픽 및 오디오 시스템을 활용함을 의미합니다. 반대로 `platform` 이 `engine` 을 참조하는 관계는 association 와 inheritance 가 각각 2 개로 매우 제한적입니다.

Android 가 네이티브 코드를 부르는 진입점은 `platform::android` 패키지의 클래스들에서 시작됩니다. `platform::android::GlContext` 는 EGL 디스플레이와 컨텍스트를 소유하며, `platform::android::AndroidAudio` 는 믹서로부터 샘플을 가져오는 저지연 출력 스트림을 제공합니다. `platform::android::AndroidHaptics` 는 JNI 를 통해 `android.os.Vibrator` 를 구동하는 유일한 위치입니다.

엔진 코드는 `engine::Engine` 이 렌더러, 스프라이트 아틀라스, 오디오 믹서 및 활성 시나인을 소유하며 화면 픽셀을 고정 너비의 월드로 매핑합니다. `engine::Engine` 의 `frame()` 메서드는 한 번의 업데이트와 렌더링 패스를 수행하며, 호출자는 이후 버퍼를 교환합니다. `engine::Engine` 의 `setHaptics()` 는 플랫폼 서비스를 설정하는 선택 사항이며 기본값은 노-옵입니다.

`app::Sfx` 클래스는 게임 내 모든 사운드 효과를 합성하며, 이는 시작 시 생성되고 오디오 파일 없이 수행됩니다. `build()` 메서드는 이 과정을 실행합니다. `app::ui::Button` 은 스프릿 스킨된 버튼으로, 클릭 시 약간 작아지고 어둡게 변하며, 내부에서 클릭하면 탄성적으로 원래 상태로 돌아갑니다. `app::ui::Popups` 은 점수 텍스트 등을 오버슈트 효과로 띄우고 사라지게 합니다.

`engine::Scene` 은 앱의 한 화면을 담당하며 엔진은 한 번에 정확히 하나의 시나인을 구동합니다. `onBack()` 메서드는 시스템 백 버튼을 처리하며, 소비되지 않으면 앱이 종료됩니다. `engine::SpriteBatch` 는 텍스처화된 쿼드를 누적하고 최소한의 드로우 호출로 제출하며, 바인딩된 텍스처가 변경되거나 버퍼가 가득 차면 플러시합니다.

`platform::android::AndroidAssetLoader` 는 APK 의 assets 디렉토리에 패킹된 파일을 읽습니다. `engine::AssetLoader` 는 플랫폼 중립적인 파일 접근을 제공합니다. `engine::Font` 는 TTF 를 stb_truetype 로 변환하여 공유 아틀라스에 베이킹하며, HUD 크기에 적합합니다.

`engine::Mixer` 는 다음성 샘플 믹서이며, `play()` 는 게임 스레드에서 호출되고 `render()` 는 오디오 콜백에서 실행됩니다. `engine::Sound` 은 믹서의 샘플 레이트인 모노 PCM 클립입니다. `engine::Particles` 은 가벼운 CPU 파티클 풀로, 중력, 마찰력, 회전 및 페이드 효과를 가진 사각형이나 스프릿의 폭발을 생성합니다.

`engine::TextureAtlas` 는 여러 작은 이미지를 로드 시간 동안 하나의 GPU 텍스처에 패킹하며, 스프라이트 배치에서 모든 것을 단일 텍스처 바인딩으로 드로우할 수 있게 합니다. `engine::Sprite` 는 텍스처의 사각형 영역이며 정규화된 UV 좌표와 픽셀 크기를 포함합니다.

`engine::Shader` 는 컴파일 및 링크된 GLSL ES 프로그램입니다. `engine::Texture` 는 RGBA8 GPU 텍스처를 소유하며 GL 객체를 소유합니다. `engine::Rect` 은 축 정렬 사각형이며 원점은 왼쪽 상단 모서리로 y 는 아래로 자라 화면 좌표와 일치합니다.

`engine::Vec2` 는 2 차원 벡터 연산자를 제공합니다. `engine::TouchEvent` 은 월드 좌표에서 플랫폼 중립적인 터치 이벤트를 제공합니다. `engine::Timer`는 프레임 타이머이며, 긴 휴지 (앱 전환, 디버거) 를 물리학적 폭발로 방지하기 위해 클램핑합니다.

각 클래스의 메서드 호출은 해당 소스 파일의 줄 번호를 통해 확인할 수 있습니다. 예를 들어 `app::ResultScene` 의 `onEnter()` 는 `app/scenes/ResultScene.cpp:29` 에서 실행됩니다. `engine::Engine` 의 `initGraphics()` 는 현재 GL 컨텍스트가 있는 상태에서 호출되어야 합니다. `platform::android::AndroidHaptics` 의 `vibrate()` 는 JNI 를 통해 구현되어 있습니다.

확인 필요: `platform::android::GlContext` 의 `swapBuffers()` 가 실제로 어떤 타이밍에 호출되는지 확인해야 합니다. 또한 `engine::Engine` 의 `frame()` 이 호출될 때 스레드 소유가 어떻게 관리되는지 확인이 필요합니다.

??? note "근거와 검토 정보"
    - 근거 파일: `app/GameAssets.cpp`, `app/GameAssets.h`, `app/GameRegistry.cpp`, `app/GameRegistry.h`, `app/HighScores.cpp`, `app/HighScores.h`, `app/IMiniGame.h`, `app/MiniGameApp.cpp`, `app/MiniGameApp.h`, `app/Sfx.cpp`, `app/Sfx.h`, `app/games/DodgeGame.cpp`, `app/games/DodgeGame.h`, `app/games/JumpGame.cpp`, `app/games/JumpGame.h`, `app/games/TapGame.cpp`, `app/games/TapGame.h`, `app/scenes/MenuScene.cpp`, `app/scenes/MenuScene.h`, `app/scenes/ResultScene.cpp`, `app/scenes/ResultScene.h`, `app/ui/Button.h`, `app/ui/Popups.h`, `engine/Engine.cpp`, `engine/Engine.h`, `engine/Scene.h`, `engine/asset/AssetLoader.h`, `engine/asset/Image.cpp`, `engine/asset/Image.h`, `engine/audio/Mixer.cpp`, `engine/audio/Mixer.h`, `engine/audio/Sound.h`, `engine/core/Timer.h`, `engine/graphics/Animation.cpp`, `engine/graphics/Animation.h`, `engine/graphics/Font.cpp`, `engine/graphics/Font.h`, `engine/graphics/Particles.cpp`, `engine/graphics/Particles.h`, `engine/graphics/Shader.cpp`, `engine/graphics/Shader.h`, `engine/graphics/Sprite.h`, `engine/graphics/SpriteBatch.cpp`, `engine/graphics/SpriteBatch.h`, `engine/graphics/Texture.cpp`, `engine/graphics/Texture.h`, `engine/graphics/TextureAtlas.cpp`, `engine/graphics/TextureAtlas.h`, `engine/input/TouchEvent.h`, `engine/math/Color.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`, `engine/platform/Haptics.h`, `platform/android/AndroidAssetLoader.cpp`, `platform/android/AndroidAssetLoader.h`, `platform/android/AndroidAudio.cpp`, `platform/android/AndroidAudio.h`, `platform/android/AndroidHaptics.cpp`, `platform/android/AndroidHaptics.h`, `platform/android/AndroidMain.cpp`, `platform/android/GlContext.cpp`, `platform/android/GlContext.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `b4e0160483`)
    - 인용 검증: 통과
    - 입력 예산 때문에 제외된 사실: functions, class:AppState, class:app::DodgeGame, class:app::DodgeGame::Falling, class:app::DodgeGame::Kind, class:app::GameAssets, class:app::GameId, class:app::GameInfo, class:app::HighScores, class:app::IMiniGame, class:app::JumpGame, class:app::JumpGame::Column, class:app::JumpGame::DeathCause, class:app::MenuScene, class:app::MiniGameApp
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [컴포넌트 구조와 책임](components.md)
