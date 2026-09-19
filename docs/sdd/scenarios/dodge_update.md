---
generated_at: 2026-09-17T14:52:32+00:00
source_commit: aeac21306341033f510bb6317043cf9fe2d6723b
agent: ollama/qwen3.5:4b
status: needs-review
section: scenarios
entry: app::DodgeGame::update(float)
---

# Dodge 게임 한 틱 (DodgeGame::update)

**아래 코드 대조 설명을 먼저 읽으세요. 다이어그램과 번호 목록은 정적 호출 후보이며, 실행 순서·분기·콜백 시점을 정확히 재현하지 않습니다.**



## 이 흐름에서 확인할 것

`DodgeGame::update`는 이미 종료되었으면 반환하고 파티클과 팝업을 순서대로 갱신합니다 (`app/games/DodgeGame.cpp:239`). 사망 상태에서는 사망 애니메이션·타이머·낙하물을 갱신하고 대기 시간이 지나면 종료 상태로 바꿉니다.

생존 상태에서는 경과 시간과 생성 타이머를 갱신하고, 타이머가 끝나면 장애물을 생성합니다. 이어서 공룡과 낙하물을 갱신하고 생존 시간·회피·보너스로 점수를 계산합니다. `Animation::frame`은 현재 프레임의 스프라이트 참조를 반환하며 재귀적으로 텍스처를 초기화하지 않습니다 (`engine/graphics/Animation.cpp:38`). 충돌 등 하위 함수의 조건 분기는 정적 목록에 모두 펼쳐질 수 있습니다.

정적 분석 한계: 가상 호출 대상, 콜백 실행 시점, 조건 분기는 코드와 함께 확인해야 합니다. 이번 검토는 기기 실행 검증을 포함하지 않습니다.

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

## 정적 호출 후보 (실행 추적 아님)

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

??? note "근거와 검토 정보"
    - 근거 파일: `app/games/DodgeGame.cpp`, `engine/graphics/Animation.cpp`, `engine/graphics/Particles.cpp`, `engine/graphics/Sprite.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `aeac213063`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 생성 당시 기록 (후속 코드 대조: docs/sdd-review.json)

다음 단계: [한 프레임 (Engine::frame)](frame.md)
