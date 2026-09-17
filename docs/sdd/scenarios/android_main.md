---
generated_at: 2026-09-17T14:52:21+00:00
source_commit: aeac21306341033f510bb6317043cf9fe2d6723b
agent: ollama/qwen3.5:4b
status: ok
section: scenarios
entry: android_main(android_app *)
---

# 앱 시작과 메인 루프 (android_main)

**`android_main(android_app *)` 에서 시작하는 호출 순서를 아래 번호대로 따라가세요.**



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

## 호출 순서

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

## 이 흐름에서 확인할 것

앱 시작 시 호출이 `android_main` 를 거쳐 `AppState`, `Engine`, `AndroidMain` 의 사이클을 반복하며 진행됩니다. 진입점인 `AndroidMain::main()` 은 `AppState::AppState()` 를 생성하여 초기화를 시작합니다 `platform/android/AndroidMain.cpp:159`. 이어지는 초기화 과정에서 `AndroidAssetLoader`, `Engine`, `AndroidHaptics`, `AndroidAudio` 가 순차적으로 생성되며 각자의 설정 함수가 호출됩니다. `AppState` 는 `GlContext::hasSurface()` 를 확인한 뒤 `android_poll_source::process()` 를 통해 이벤트 루프 진입을 시도합니다. 정적 추적 불가인 `process()` 함수는 `processInput()` 과 같은 구체적인 입력 처리 로직으로 분기합니다.

입력 처리 단계에서 `AndroidMain` 는 `Engine::onTouch()` 와 `Engine::frame()` 을 호출하여 게임 상태 업데이트를 주도합니다. `Engine` 내부에서는 `Scene::onTouch()` 와 같은 가상 함수가 호출되어 현재 활성화된 시나리오에 따라 동작이 결정됩니다. `Scene` 의 구현은 단일하게 존재하지만, `onBack()` 과 같은 특정 이벤트는 `MiniGameApp`, `ResultScene` 등 여러 후보 클래스를 거칩니다. 정적 추적의 한계로 인해 스레드 소유권이나 타이밍 정보는 현재 사실 목록에서 확인되지 않았습니다.

확인 필요: 가상 함수나 함수 포인터 때문에 정적으로 끊긴 호출이 2 개 있습니다. 끊긴 지점 이후는 코드를 직접 따라가야 합니다.

??? note "근거와 검토 정보"
    - 근거 파일: `engine/Engine.cpp`, `platform/android/AndroidMain.cpp`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `aeac213063`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [Dodge 게임 한 틱 (DodgeGame::update)](dodge_update.md)
