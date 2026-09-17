---
generated_at: 2026-09-17T14:52:32+00:00
source_commit: aeac21306341033f510bb6317043cf9fe2d6723b
agent: ollama/qwen3.5:4b
status: ok
section: scenarios
entry: app::DodgeGame::update(float)
---

# Dodge 게임 한 틱 (DodgeGame::update)

**`app::DodgeGame::update(float)` 에서 시작하는 호출 순서를 아래 번호대로 따라가세요.**



```mermaid
sequenceDiagram
    participant DodgeGame
    participant Particles
    participant Vec2
    participant Popups
    participant Animation
    participant Sprite
    participant Rect
    participant Engine
    participant Haptics
    participant Mixer
    participant GameAssets
    participant Burst
    participant Color
    participant Falling
    participant Particle
    participant Ease
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
    DodgeGame->>Engine: haptics()
    DodgeGame->>Mixer: play()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Burst: Burst()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    DodgeGame->>Particles: emit()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>DodgeGame: land()
    DodgeGame->>Vec2: operator*()
    DodgeGame->>Sprite: size()
    DodgeGame->>Burst: Burst()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    DodgeGame->>Mixer: play()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Engine: addTrauma()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: rgb8()
    DodgeGame->>Mixer: play()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Particles: emit()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>Popups: add()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>Color: rgb8()
    DodgeGame->>Mixer: play()
    DodgeGame->>Engine: mixer()
    DodgeGame->>GameAssets: sfx()
    DodgeGame->>Haptics: light()
    DodgeGame->>Engine: haptics()
    DodgeGame->>DodgeGame: spawn()
    DodgeGame->>Engine: worldWidth()
    DodgeGame->>Falling: Falling()
    DodgeGame-->>DodgeGame: chance() [콜백 객체, 정적 추적 불가]
    DodgeGame->>Sprite: operator=()
    DodgeGame->>GameAssets: crate()
    GameAssets->>Sprite: Sprite()
    DodgeGame->>GameAssets: stone()
    GameAssets->>Sprite: Sprite()
    DodgeGame->>Vec2: operator=()
    DodgeGame->>Vec2: Vec2()
    DodgeGame-->>DodgeGame: x() [콜백 객체, 정적 추적 불가]
    DodgeGame->>DodgeGame: fallSpeedAt()
    DodgeGame-->>DodgeGame: spin() [콜백 객체, 정적 추적 불가]
    DodgeGame->>DodgeGame: spawnIntervalAt()
    DodgeGame->>DodgeGame: updateDino()
    DodgeGame->>Animation: update()
    Animation->>Animation: duration()
    DodgeGame->>Burst: Burst()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: withAlpha()
    Color->>Color: Color()
    DodgeGame->>Color: rgb8()
    Color->>Color: Color()
    DodgeGame->>Color: operator=()
    DodgeGame->>Color: withAlpha()
    Color->>Color: Color()
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
    DodgeGame->>Animation: update()
    DodgeGame->>Animation: frame()
    DodgeGame->>Engine: worldWidth()
    DodgeGame->>Ease: outBack()
    DodgeGame->>Vec2: operator=()
    DodgeGame->>Vec2: Vec2()
    DodgeGame->>DodgeGame: updateFalling()
```

## 호출 순서

1. `DodgeGame` 가 `Particles::update()` 를 호출합니다. `app/games/DodgeGame.cpp:243`
2. `Particles` 가 `Vec2::operator*=()` 를 호출합니다. `engine/graphics/Particles.cpp:45`
3. `Particles` 가 `Vec2::operator+=()` 를 호출합니다. `engine/graphics/Particles.cpp:46`
4. `Particles` 가 `Vec2::operator*()` 를 호출합니다. `engine/graphics/Particles.cpp:46`
5. `Vec2` 가 `Vec2::Vec2()` 를 호출합니다. `engine/math/Vec2.h:16`
6. `DodgeGame` 가 `Popups::update()` 를 호출합니다. `app/games/DodgeGame.cpp:244`
7. `DodgeGame` 가 `Animation::update()` 를 호출합니다. `app/games/DodgeGame.cpp:247`
8. `Animation` 가 `Animation::duration()` 를 호출합니다. `engine/graphics/Animation.cpp:32`
9. `DodgeGame` 가 `DodgeGame::updateFalling()` 를 호출합니다. `app/games/DodgeGame.cpp:249`
10. `DodgeGame` 가 `Animation::frame()` 를 호출합니다. `app/games/DodgeGame.cpp:204`
11. `Animation` 가 `Sprite::Sprite()` 를 호출합니다. `engine/graphics/Animation.cpp:39`
12. `DodgeGame` 가 `Animation::frame()` 를 호출합니다. `app/games/DodgeGame.cpp:204`
13. `Animation` 가 `Sprite::Sprite()` 를 호출합니다. `engine/graphics/Animation.cpp:39`
14. `DodgeGame` 가 `Vec2::operator*()` 를 호출합니다. `app/games/DodgeGame.cpp:205`
15. `Vec2` 가 `Vec2::Vec2()` 를 호출합니다. `engine/math/Vec2.h:16`
16. `DodgeGame` 가 `Sprite::size()` 를 호출합니다. `app/games/DodgeGame.cpp:205`
17. `Sprite` 가 `Vec2::Vec2()` 를 호출합니다. `engine/graphics/Sprite.h:21`
18. `DodgeGame` 가 `Rect::fromCenter()` 를 호출합니다. `app/games/DodgeGame.cpp:207`
19. `Rect` 가 `Rect::Rect()` 를 호출합니다. `engine/math/Rect.h:19`
20. `DodgeGame` 가 `Vec2::Vec2()` 를 호출합니다. `app/games/DodgeGame.cpp:208`
21. `DodgeGame` 가 `Vec2::operator*()` 를 호출합니다. `app/games/DodgeGame.cpp:218`
22. `Vec2` 가 `Vec2::Vec2()` 를 호출합니다. `engine/math/Vec2.h:16`
23. `DodgeGame` 가 `Sprite::size()` 를 호출합니다. `app/games/DodgeGame.cpp:218`
24. `Sprite` 가 `Vec2::Vec2()` 를 호출합니다. `engine/graphics/Sprite.h:21`
25. `DodgeGame` 가 `Rect::fromCenter()` 를 호출합니다. `app/games/DodgeGame.cpp:219`
26. `Rect` 가 `Rect::Rect()` 를 호출합니다. `engine/math/Rect.h:19`
27. `DodgeGame` 가 `Vec2::Vec2()` 를 호출합니다. `app/games/DodgeGame.cpp:219`
28. `DodgeGame` 가 `Vec2::operator*()` 를 호출합니다. `app/games/DodgeGame.cpp:219`
29. `Vec2` 가 `Vec2::Vec2()` 를 호출합니다. `engine/math/Vec2.h:16`
30. `DodgeGame` 가 `Rect::overlaps()` 를 호출합니다. `app/games/DodgeGame.cpp:220`

(이후 단계는 생략했습니다. 전체 흐름은 위 다이어그램을 보세요.)

## 이 흐름에서 확인할 것

Dodge 게임 한 틱의 호출은 `app::DodgeGame::update(float)` 에서 시작해 `Particles`, `Popups`, `Animation` 등 여러 컴포넌트를 순차적으로 거쳐갑니다. 진입점인 `app::DodgeGame::update(float)` 는 먼저 `Particles::update()` 를 호출합니다 `app/games/DodgeGame.cpp:243`. 이어진 `Particles` 내부에서는 `Vec2::operator*=()` 와 `Vec2::operator+=()` 같은 연산자가 실행되며, 이는 다시 `Vec2::Vec2()` 를 생성하는 과정으로 이어집니다 `engine/graphics/Particles.cpp:45`, `engine/math/Vec2.h:16`. 이후 `DodgeGame` 는 `Popups::update()` 와 `Animation::update()` 를 차례로 호출합니다 `app/games/DodgeGame.cpp:244`, `app/games/DodgeGame.cpp:247`. `Animation::duration()` 을 통해 애니메이션 지속 시간을 확인한 뒤, `Animation::frame()` 와 `Sprite::Sprite()` 를 재귀적으로 호출하며 스프라이트 초기화 과정을 거칩니다 `engine/graphics/Animation.cpp:32`, `engine/graphics/Animation.cpp:39`. 이 과정에서 `Vec2::operator*()` 가 반복되어 벡터 곱셈이 수행되고, `Rect::fromCenter()` 를 통해 사각형 영역이 계산됩니다 `app/games/DodgeGame.cpp:205`, `app/games/DodgeGame.cpp:218`, `engine/math/Rect.h:19`. 최종적으로 `DodgeGame` 는 `Rect::overlaps()` 를 호출하여 충돌 감지를 시도합니다 `app/games/DodgeGame.cpp:220`.

확인 필요: 가상 함수나 함수 포인터 때문에 정적으로 끊긴 호출이 10 개 있습니다. 끊긴 지점 이후는 코드를 직접 따라가야 합니다.

??? note "근거와 검토 정보"
    - 근거 파일: `app/games/DodgeGame.cpp`, `engine/graphics/Animation.cpp`, `engine/graphics/Particles.cpp`, `engine/graphics/Sprite.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `aeac213063`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [한 프레임 (Engine::frame)](frame.md)
