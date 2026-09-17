---
generated_at: 2026-09-17T14:52:44+00:00
source_commit: aeac21306341033f510bb6317043cf9fe2d6723b
agent: ollama/qwen3.5:4b
status: ok
section: scenarios
entry: engine::Engine::frame()
---

# 한 프레임 (Engine::frame)

**`engine::Engine::frame()` 에서 시작하는 호출 순서를 아래 번호대로 따라가세요.**



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

## 호출 순서

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

## 이 흐름에서 확인할 것

한 프레임은 `Engine::frame()` 에서 시작하여 `Timer::tick()` 을 거쳐 게임 로직 업데이트로 이어집니다. `Engine` 는 타이머를 호출한 뒤 바로 `DodgeGame::update()` 를 실행합니다 `engine/Engine.cpp:81`, `engine/Engine.cpp:91`. `DodgeGame` 내부에서는 파티클 시스템과 애니메이션이 병렬로 업데이트되며, 각 컴포넌트는 구체적인 계산 함수를 직접 호출합니다.

파티클 업데이트 단계에서 `Particles` 는 벡터 연산자 (`operator*`, `operator+=`) 를 순차적으로 호출하여 물리 계산을 수행합니다. 이 과정에서 `Vec2::Vec2()` 생성자가 반복되어 호출되며, 이는 수학 라이브러리의 기본 동작입니다 `engine/graphics/Particles.cpp:45`, `engine/graphics/Particles.cpp:46`, `engine/math/Vec2.h:16`. 애니메이션 시스템은 `Animation::duration()` 을 통해 시간 정보를 가져오고, `Sprite::Sprite()` 를 생성하여 그래픽 리소스를 초기화합니다.

`DodgeGame` 는 `Popups::update()` 와 `Animation::frame()` 을 호출하며, 이 과정에서 스프라이트 크기와 화면 좌표를 계산합니다. `Sprite::size()` 와 `Rect::fromCenter()` 가 빈번하게 호출되어 UI 요소의 위치를 결정하고, `Vec2::operator*()` 를 통해 좌표 변환을 수행합니다 `app/games/DodgeGame.cpp:244`, `app/games/DodgeGame.cpp:247`, `engine/graphics/Sprite.h:21`, `engine/math/Rect.h:19`.

이 흐름은 가상 함수 호출과 정적 타입 추적을 통해 이루어지며, 특정 단계에서 분기나 정적 분석의 단절이 발생할 수 있습니다. 현재 사실 목록에는 스레드 소유권이나 타이밍에 대한 명시적인 설계 의도가 포함되어 있지 않습니다. 확인 필요: `DodgeGame::update()` 의 가상 함수 구현 여부 및 정적 분석 시 끊기는 지점을 어떻게 처리하는지 확인해야 합니다.

다음 절은 프레임 렌더링 단계와 입력 처리 흐름을 다룹니다.

확인 필요: 가상 함수나 함수 포인터 때문에 정적으로 끊긴 호출이 15 개 있습니다. 끊긴 지점 이후는 코드를 직접 따라가야 합니다.

??? note "근거와 검토 정보"
    - 근거 파일: `app/games/DodgeGame.cpp`, `engine/Engine.cpp`, `engine/graphics/Animation.cpp`, `engine/graphics/Particles.cpp`, `engine/graphics/Sprite.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `aeac213063`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [공유 에셋 로딩 (GameAssets::load)](load_assets.md)
