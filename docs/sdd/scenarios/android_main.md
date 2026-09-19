---
generated_at: 2026-09-17T14:52:21+00:00
source_commit: aeac21306341033f510bb6317043cf9fe2d6723b
agent: ollama/qwen3.5:4b
status: needs-review
section: scenarios
entry: android_main(android_app *)
---

# 앱 시작과 메인 루프 (android_main)

**아래 코드 대조 설명을 먼저 읽으세요. 다이어그램과 번호 목록은 정적 호출 후보이며, 실행 순서·분기·콜백 시점을 정확히 재현하지 않습니다.**



## 이 흐름에서 확인할 것

`android_main`은 `AppState`를 만들고 `handleAppCmd`를 앱 명령 콜백으로 등록합니다 (`platform/android/AndroidMain.cpp:156`). 등록 자체가 윈도우 초기화를 즉시 실행하지는 않습니다. 루프에서 이벤트를 처리하고 종료 요청을 검사한 뒤 `processInput`을 호출합니다. surface와 focus가 모두 있을 때 `Engine::frame` 다음에 `GlContext::swapBuffers`를 호출합니다.

윈도우 생성 명령에서는 GL 초기화와 루트 `MiniGameApp` 설정을 수행합니다. 포커스를 얻거나 잃으면 오디오를 시작하거나 중지합니다 (`platform/android/AndroidMain.cpp:54`). 함수 포인터와 가상 함수 후보를 모두 펼친 아래 정적 목록은 한 번의 실행 기록이 아닙니다.

정적 분석 한계: 가상 호출 대상, 콜백 실행 시점, 조건 분기는 코드와 함께 확인해야 합니다. 이번 검토는 기기 실행 검증을 포함하지 않습니다.

```mermaid
sequenceDiagram
    participant AndroidMain
    participant AppState
    participant AndroidAssetLoader
    participant Engine
    participant AndroidHaptics
    participant AndroidAudio
    participant GlContext
    participant android_poll_source
    participant TouchEvent
    participant Vec2
    participant Scene
    participant MiniGameApp
    participant ResultScene
    participant Timer
    participant DodgeGame
    participant Particles
    participant Popups
    participant Animation
    participant JumpGame
    participant TapGame
    participant Mixer
    participant GameAssets
    participant MenuScene
    participant Button
    participant Haptics
    participant Burst
    participant Color
    participant Rect
    participant SpriteBatch
    AndroidMain->>AppState: AppState()
    AppState->>AndroidAssetLoader: AndroidAssetLoader()
    AppState->>Engine: Engine()
    AppState->>AndroidHaptics: AndroidHaptics()
    AppState->>AndroidAudio: AndroidAudio()
    AppState->>Engine: mixer()
    AppState->>Engine: setHaptics()
    AppState->>Engine: setDataPath()
    AndroidMain->>AppState: animating()
    AppState->>GlContext: hasSurface()
    AndroidMain-->>android_poll_source: process() [함수 포인터, 정적 추적 불가]
    AndroidMain->>AndroidMain: processInput()
    AndroidMain->>AndroidMain: phaseFor()
    AndroidMain->>Engine: onTouch()
    Engine->>TouchEvent: TouchEvent()
    Engine->>Vec2: operator=()
    Engine->>Vec2: Vec2()
    Engine-->>Scene: onTouch() [virtual, 현재 구현 하나]
    AndroidMain->>Engine: onTouch()
    Engine->>TouchEvent: TouchEvent()
    Engine->>Vec2: operator=()
    Engine->>Vec2: Vec2()
    Engine-->>Scene: onTouch() [virtual, 현재 구현 하나]
    AndroidMain->>Engine: onBack()
    Engine-->>Scene: onBack() [virtual 후보]
    Engine-->>MiniGameApp: onBack() [virtual 후보]
    Engine-->>ResultScene: onBack() [virtual 후보]
    AndroidMain->>AppState: animating()
    AppState->>GlContext: hasSurface()
    AndroidMain->>Engine: frame()
    Engine->>Timer: tick()
    Engine-->>DodgeGame: update() [virtual 후보]
    DodgeGame->>Particles: update()
    DodgeGame->>Popups: update()
    DodgeGame->>Animation: update()
    DodgeGame->>DodgeGame: updateFalling()
    DodgeGame->>DodgeGame: spawn()
    DodgeGame->>DodgeGame: spawnIntervalAt()
    DodgeGame->>DodgeGame: updateDino()
    DodgeGame->>DodgeGame: updateFalling()
    Engine-->>JumpGame: update() [virtual 후보]
    JumpGame->>Particles: update()
    JumpGame->>Animation: update()
    JumpGame->>JumpGame: updateWorld()
    JumpGame->>JumpGame: updatePlayer()
    Engine-->>TapGame: update() [virtual 후보]
    TapGame->>Particles: update()
    TapGame->>Popups: update()
    TapGame->>Mixer: play()
    TapGame->>Engine: mixer()
    TapGame->>GameAssets: sfx()
    TapGame->>Engine: addTrauma()
    TapGame->>Mixer: play()
    TapGame->>Engine: mixer()
    TapGame->>GameAssets: sfx()
    TapGame->>TapGame: endRound()
    Engine-->>MiniGameApp: update() [virtual 후보]
    MiniGameApp-->>Scene: onExit() [virtual, 현재 구현 하나]
    MiniGameApp-->>Scene: onEnter() [virtual, 현재 구현 하나]
    MiniGameApp-->>DodgeGame: update() [virtual 후보]
    MiniGameApp-->>JumpGame: update() [virtual 후보]
    MiniGameApp-->>TapGame: update() [virtual 후보]
    MiniGameApp-->>MiniGameApp: update() [virtual 후보]
    MiniGameApp-->>MenuScene: update() [virtual 후보]
    MiniGameApp-->>ResultScene: update() [virtual 후보]
    MiniGameApp->>Button: update()
    MiniGameApp-->>DodgeGame: isFinished() [virtual 후보]
    MiniGameApp-->>JumpGame: isFinished() [virtual 후보]
    MiniGameApp-->>TapGame: isFinished() [virtual 후보]
    MiniGameApp->>MiniGameApp: showResult()
    MiniGameApp-->>DodgeGame: score() [virtual 후보]
    MiniGameApp-->>JumpGame: score() [virtual 후보]
    MiniGameApp-->>TapGame: score() [virtual 후보]
    Engine-->>MenuScene: update() [virtual 후보]
    MenuScene->>Animation: update()
    MenuScene->>Button: update()
    Engine-->>ResultScene: update() [virtual 후보]
    ResultScene->>Button: update()
    ResultScene->>Button: update()
    ResultScene->>Particles: update()
    ResultScene->>Haptics: medium()
    ResultScene->>Engine: haptics()
    ResultScene->>Engine: addTrauma()
    ResultScene->>Burst: Burst()
    ResultScene->>Color: operator=()
    ResultScene->>Color: rgb8()
    ResultScene->>Color: operator=()
    ResultScene->>Color: rgb8()
    ResultScene->>Particles: emit()
    ResultScene->>Vec2: Vec2()
    ResultScene->>Rect: center()
    ResultScene->>Rect: bottom()
    Engine->>Vec2: Vec2()
    Engine->>Vec2: Vec2()
    Engine->>Vec2: operator=()
    Engine->>Vec2: Vec2()
    Engine->>SpriteBatch: begin()
    SpriteBatch->>Vec2: Vec2()
    SpriteBatch->>SpriteBatch: makeOrtho()
    SpriteBatch->>Vec2: Vec2()
    Engine->>Vec2: Vec2()
    Engine-->>Scene: render() [virtual, override 없음]
    Engine->>SpriteBatch: end()
    SpriteBatch->>SpriteBatch: flush()
    AndroidMain->>GlContext: swapBuffers()
```

## 정적 호출 후보 (실행 추적 아님)

1. `AndroidMain` 가 `AppState::AppState()` 를 호출합니다. `platform/android/AndroidMain.cpp:159`
2. `AppState` 가 `AndroidAssetLoader::AndroidAssetLoader()` 를 호출합니다. `platform/android/AndroidMain.cpp:30`
3. `AppState` 가 `Engine::Engine()` 를 호출합니다. `platform/android/AndroidMain.cpp:30`
4. `AppState` 가 `AndroidHaptics::AndroidHaptics()` 를 호출합니다. `platform/android/AndroidMain.cpp:30`
5. `AppState` 가 `AndroidAudio::AndroidAudio()` 를 호출합니다. `platform/android/AndroidMain.cpp:30`
6. `AppState` 가 `Engine::mixer()` 를 호출합니다. `platform/android/AndroidMain.cpp:30`
7. `AppState` 가 `Engine::setHaptics()` 를 호출합니다. `platform/android/AndroidMain.cpp:31`
8. `AppState` 가 `Engine::setDataPath()` 를 호출합니다. `platform/android/AndroidMain.cpp:32`
9. `AndroidMain` 가 `AppState::animating()` 를 호출합니다. `platform/android/AndroidMain.cpp:173`
10. `AppState` 가 `GlContext::hasSurface()` 를 호출합니다. `platform/android/AndroidMain.cpp:42`
11. `AndroidMain` 가 `android_poll_source::process()` 를 호출합니다. (함수 포인터, 정적 추적 불가) `platform/android/AndroidMain.cpp:176`
12. `AndroidMain` 가 `AndroidMain::processInput()` 를 호출합니다. `platform/android/AndroidMain.cpp:184`
13. `AndroidMain` 가 `AndroidMain::phaseFor()` 를 호출합니다. `platform/android/AndroidMain.cpp:122`
14. `AndroidMain` 가 `Engine::onTouch()` 를 호출합니다. `platform/android/AndroidMain.cpp:128`
15. `Engine` 가 `TouchEvent::TouchEvent()` 를 호출합니다. `engine/Engine.cpp:120`
16. `Engine` 가 `Vec2::operator=()` 를 호출합니다. `engine/Engine.cpp:123`
17. `Engine` 가 `Vec2::Vec2()` 를 호출합니다. `engine/Engine.cpp:123`
18. `Engine` 가 `Scene::onTouch()` 를 호출합니다. (virtual, 현재 구현 하나) `engine/Engine.cpp:124`
19. `AndroidMain` 가 `Engine::onTouch()` 를 호출합니다. `platform/android/AndroidMain.cpp:136`
20. `Engine` 가 `TouchEvent::TouchEvent()` 를 호출합니다. `engine/Engine.cpp:120`
21. `Engine` 가 `Vec2::operator=()` 를 호출합니다. `engine/Engine.cpp:123`
22. `Engine` 가 `Vec2::Vec2()` 를 호출합니다. `engine/Engine.cpp:123`
23. `Engine` 가 `Scene::onTouch()` 를 호출합니다. (virtual, 현재 구현 하나) `engine/Engine.cpp:124`
24. `AndroidMain` 가 `Engine::onBack()` 를 호출합니다. `platform/android/AndroidMain.cpp:146`
25. `Engine` 가 `Scene::onBack()` 를 호출합니다. (virtual 후보) `engine/Engine.cpp:128`
26. `Engine` 가 `MiniGameApp::onBack()` 를 호출합니다. (virtual 후보) `engine/Engine.cpp:128`
27. `Engine` 가 `ResultScene::onBack()` 를 호출합니다. (virtual 후보) `engine/Engine.cpp:128`
28. `AndroidMain` 가 `AppState::animating()` 를 호출합니다. `platform/android/AndroidMain.cpp:186`
29. `AppState` 가 `GlContext::hasSurface()` 를 호출합니다. `platform/android/AndroidMain.cpp:42`
30. `AndroidMain` 가 `Engine::frame()` 를 호출합니다. `platform/android/AndroidMain.cpp:187`

(이후 단계는 생략했습니다. 전체 흐름은 위 다이어그램을 보세요.)

??? note "근거와 검토 정보"
    - 근거 파일: `engine/Engine.cpp`, `platform/android/AndroidMain.cpp`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `aeac213063`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 생성 당시 기록 (후속 코드 대조: docs/sdd-review.json)

다음 단계: [Dodge 게임 한 틱 (DodgeGame::update)](dodge_update.md)
