---
generated_at: 2026-09-17T14:52:44+00:00
source_commit: aeac21306341033f510bb6317043cf9fe2d6723b
agent: ollama/qwen3.5:4b
status: needs-review
section: scenarios
entry: engine::Engine::frame()
---

# 한 프레임 (Engine::frame)

**아래 코드 대조 설명을 먼저 읽으세요. 다이어그램과 번호 목록은 정적 호출 후보이며, 실행 순서·분기·콜백 시점을 정확히 재현하지 않습니다.**



## 이 흐름에서 확인할 것

`Engine::frame`은 그래픽 초기화 여부를 확인한 뒤 `Timer::tick`으로 프레임 시간을 구합니다 (`engine/Engine.cpp:77`). hit-stop 중에는 장면에 전달할 dt를 0으로 만들고, 활성 루트 장면의 `update`를 호출합니다. 이 앱의 루트는 `MiniGameApp`이며 내부의 현재 메뉴·게임·결과 장면에 업데이트를 전달합니다 (`app/MiniGameApp.cpp:65`). 매 프레임 모든 가상 함수 후보를 실행하지 않습니다.

이후 실제 경과 시간으로 화면 흔들림을 계산하고 화면을 지운 뒤 `SpriteBatch::begin`, 장면의 `render`, `SpriteBatch::end`를 순서대로 실행합니다. 버퍼 교환은 호출자인 Android 메인 루프가 수행합니다. 게임 업데이트와 파티클 업데이트를 병렬로 실행하는 코드는 없습니다.

정적 분석 한계: 가상 호출 대상, 콜백 실행 시점, 조건 분기는 코드와 함께 확인해야 합니다. 이번 검토는 기기 실행 검증을 포함하지 않습니다.

```mermaid
sequenceDiagram
    participant Engine
    participant Timer
    participant DodgeGame
    participant Particles
    participant Vec2
    participant Popups
    participant Animation
    participant Sprite
    participant Rect
    participant Haptics
    participant NullHaptics
    participant AndroidHaptics
    participant Mixer
    participant Sound
    participant GameAssets
    participant Burst
    participant Color
    participant Particle
    participant JumpGame
    participant TapGame
    participant MiniGameApp
    participant MenuScene
    participant ResultScene
    participant SpriteBatch
    participant Scene
    Engine->>Timer: tick()
    Engine-->>DodgeGame: update() [virtual 후보]
    DodgeGame->>Particles: update()
    Particles->>Vec2: operator*=()
    Particles->>Vec2: operator+=()
    Particles->>Vec2: operator*()
    Vec2->>Vec2: Vec2()
    DodgeGame->>Popups: update()
    DodgeGame->>Animation: update()
    Animation->>Animation: duration()
    DodgeGame->>DodgeGame: updateFalling()
    DodgeGame->>Animation: frame()
    Animation->>Sprite: Sprite()
    DodgeGame->>Animation: frame()
    Animation->>Sprite: Sprite()
    DodgeGame->>Vec2: operator*()
    Vec2->>Vec2: Vec2()
    DodgeGame->>Sprite: size()
    Sprite->>Vec2: Vec2()
    DodgeGame->>Rect: fromCenter()
    Rect->>Rect: Rect()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>Vec2: operator*()
    Vec2->>Vec2: Vec2()
    DodgeGame->>Sprite: size()
    Sprite->>Vec2: Vec2()
    DodgeGame->>Rect: fromCenter()
    Rect->>Rect: Rect()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>Vec2: operator*()
    Vec2->>Vec2: Vec2()
    DodgeGame->>Rect: overlaps()
    DodgeGame->>DodgeGame: hit()
    DodgeGame->>Animation: restart()
    DodgeGame->>Engine: hitStop()
    DodgeGame->>Engine: addTrauma()
    DodgeGame->>Haptics: heavy()
    Haptics-->>NullHaptics: vibrate() [virtual 후보]
    Haptics-->>AndroidHaptics: vibrate() [virtual 후보]
    DodgeGame->>Engine: haptics()
    DodgeGame->>Mixer: play()
    Mixer->>Sound: empty()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Burst: Burst()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Particles: emit()
    Particles-->>Particles: angle() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: speed() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: unit() [콜백 객체, 정적 추적 불가]
    Particles->>Particle: Particle()
    Particles->>Vec2: operator=()
    Particles->>Vec2: operator=()
    Particles->>Vec2: Vec2()
    Particles-->>Particles: life() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: size() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: angle() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: spin() [콜백 객체, 정적 추적 불가]
    Particles->>Color: operator=()
    Particles->>Color: Color()
    Particles->>Sprite: operator=()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>DodgeGame: land()
    DodgeGame->>Vec2: operator*()
    Vec2->>Vec2: Vec2()
    DodgeGame->>Sprite: size()
    Sprite->>Vec2: Vec2()
    DodgeGame->>Burst: Burst()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Mixer: play()
    Mixer->>Sound: empty()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Engine: addTrauma()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Mixer: play()
    Mixer->>Sound: empty()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Particles: emit()
    Particles-->>Particles: angle() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: speed() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: unit() [콜백 객체, 정적 추적 불가]
    Particles->>Particle: Particle()
    Particles->>Vec2: operator=()
    Particles->>Vec2: operator=()
    Particles->>Vec2: Vec2()
    Particles-->>Particles: life() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: size() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: angle() [콜백 객체, 정적 추적 불가]
    Particles-->>Particles: spin() [콜백 객체, 정적 추적 불가]
    Particles->>Color: operator=()
    Particles->>Color: Color()
    Particles->>Sprite: operator=()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>Popups: add()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Mixer: play()
    Mixer->>Sound: empty()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Haptics: light()
    DodgeGame->>Engine: haptics()
    DodgeGame->>DodgeGame: spawn()
    DodgeGame->>DodgeGame: spawnIntervalAt()
    DodgeGame->>DodgeGame: updateDino()
    DodgeGame->>DodgeGame: updateFalling()
    Engine-->>JumpGame: update() [virtual 후보]
    Engine-->>TapGame: update() [virtual 후보]
    Engine-->>MiniGameApp: update() [virtual 후보]
    Engine-->>MenuScene: update() [virtual 후보]
    Engine-->>ResultScene: update() [virtual 후보]
    Engine->>Vec2: Vec2()
    Engine->>Vec2: Vec2()
    Engine->>Vec2: operator=()
    Engine->>Vec2: Vec2()
    Engine->>SpriteBatch: begin()
    Engine->>Vec2: Vec2()
    Engine-->>Scene: render() [virtual, override 없음]
    Engine->>SpriteBatch: end()
```

## 정적 호출 후보 (실행 추적 아님)

1. `Engine` 가 `Timer::tick()` 를 호출합니다. `engine/Engine.cpp:81`
2. `Engine` 가 `DodgeGame::update()` 를 호출합니다. (virtual 후보) `engine/Engine.cpp:91`
3. `DodgeGame` 가 `Particles::update()` 를 호출합니다. `app/games/DodgeGame.cpp:243`
4. `Particles` 가 `Vec2::operator*=()` 를 호출합니다. `engine/graphics/Particles.cpp:45`
5. `Particles` 가 `Vec2::operator+=()` 를 호출합니다. `engine/graphics/Particles.cpp:46`
6. `Particles` 가 `Vec2::operator*()` 를 호출합니다. `engine/graphics/Particles.cpp:46`
7. `Vec2` 가 `Vec2::Vec2()` 를 호출합니다. `engine/math/Vec2.h:16`
8. `DodgeGame` 가 `Popups::update()` 를 호출합니다. `app/games/DodgeGame.cpp:244`
9. `DodgeGame` 가 `Animation::update()` 를 호출합니다. `app/games/DodgeGame.cpp:247`
10. `Animation` 가 `Animation::duration()` 를 호출합니다. `engine/graphics/Animation.cpp:32`
11. `DodgeGame` 가 `DodgeGame::updateFalling()` 를 호출합니다. `app/games/DodgeGame.cpp:249`
12. `DodgeGame` 가 `Animation::frame()` 를 호출합니다. `app/games/DodgeGame.cpp:204`
13. `Animation` 가 `Sprite::Sprite()` 를 호출합니다. `engine/graphics/Animation.cpp:39`
14. `DodgeGame` 가 `Animation::frame()` 를 호출합니다. `app/games/DodgeGame.cpp:204`
15. `Animation` 가 `Sprite::Sprite()` 를 호출합니다. `engine/graphics/Animation.cpp:39`
16. `DodgeGame` 가 `Vec2::operator*()` 를 호출합니다. `app/games/DodgeGame.cpp:205`
17. `Vec2` 가 `Vec2::Vec2()` 를 호출합니다. `engine/math/Vec2.h:16`
18. `DodgeGame` 가 `Sprite::size()` 를 호출합니다. `app/games/DodgeGame.cpp:205`
19. `Sprite` 가 `Vec2::Vec2()` 를 호출합니다. `engine/graphics/Sprite.h:21`
20. `DodgeGame` 가 `Rect::fromCenter()` 를 호출합니다. `app/games/DodgeGame.cpp:207`
21. `Rect` 가 `Rect::Rect()` 를 호출합니다. `engine/math/Rect.h:19`
22. `DodgeGame` 가 `Vec2::Vec2()` 를 호출합니다. `app/games/DodgeGame.cpp:208`
23. `DodgeGame` 가 `Vec2::operator*()` 를 호출합니다. `app/games/DodgeGame.cpp:218`
24. `Vec2` 가 `Vec2::Vec2()` 를 호출합니다. `engine/math/Vec2.h:16`
25. `DodgeGame` 가 `Sprite::size()` 를 호출합니다. `app/games/DodgeGame.cpp:218`
26. `Sprite` 가 `Vec2::Vec2()` 를 호출합니다. `engine/graphics/Sprite.h:21`
27. `DodgeGame` 가 `Rect::fromCenter()` 를 호출합니다. `app/games/DodgeGame.cpp:219`
28. `Rect` 가 `Rect::Rect()` 를 호출합니다. `engine/math/Rect.h:19`
29. `DodgeGame` 가 `Vec2::Vec2()` 를 호출합니다. `app/games/DodgeGame.cpp:219`
30. `DodgeGame` 가 `Vec2::operator*()` 를 호출합니다. `app/games/DodgeGame.cpp:219`

(이후 단계는 생략했습니다. 전체 흐름은 위 다이어그램을 보세요.)

??? note "근거와 검토 정보"
    - 근거 파일: `app/games/DodgeGame.cpp`, `engine/Engine.cpp`, `engine/graphics/Animation.cpp`, `engine/graphics/Particles.cpp`, `engine/graphics/Sprite.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `aeac213063`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 생성 당시 기록 (후속 코드 대조: docs/sdd-review.json)

다음 단계: [공유 에셋 로딩 (GameAssets::load)](load_assets.md)
