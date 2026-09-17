# mini-game-heaven

C++로 만드는 안드로이드 미니게임 모음. 게임 로직과 렌더링은 거의 전부 C++이고, Android 쪽은 GameActivity와 Gradle 포장 계층만 남긴다.

## 목표 (v0.1)

- APK 설치 → 메인 메뉴 → 게임 선택 → 플레이 → 점수 저장까지 한 사이클 완성
- 미니게임 3개: Dodge / Jump / Tap
- 세로 화면, 터치 입력, 효과음, 진동, 로컬 최고 점수
- 60 / 120Hz 대응, FPS 표시 디버그 옵션
- Release APK 생성

## 기술 스택

| 영역 | 선택 |
|------|------|
| Language | C++20 |
| Android | NDK + GameActivity |
| Build | CMake + Gradle |
| Rendering | OpenGL ES 3.x |
| Audio | Oboe 또는 AAudio |
| Input | GameActivity input |
| Assets | AAssetManager |
| UI | 자체 C++ UI |
| Storage | native file |
| Frame pacing | Choreographer |

## 구조

```
mini-game-heaven/
├── app/                      # Android shell (Gradle, Manifest, GameActivity)
│   └── src/main/
│       ├── AndroidManifest.xml
│       ├── assets/
│       └── cpp/
└── native/                   # 거의 전부 C++
    ├── CMakeLists.txt
    ├── engine/               # GameLoop, Renderer, Input, Audio, Asset, Scene
    ├── games/                # dodge/, jump/, tap/
    └── app/                  # MiniGameApp, GameRegistry
```

공통 엔진과 개별 게임은 `IMiniGame` 인터페이스로 분리한다. 게임 추가는 `GameRegistry`에 한 줄 등록하면 끝나도록 설계한다.

```cpp
class IMiniGame {
public:
    virtual ~IMiniGame() = default;

    virtual void onEnter() = 0;
    virtual void update(float dt) = 0;
    virtual void render(Renderer& renderer) = 0;
    virtual void onTouch(const TouchEvent& event) = 0;
    virtual bool isFinished() const = 0;
    virtual int getScore() const = 0;
};
```

## 로드맵

- v0.1: Dodge / Jump / Tap
- v0.2: + Shooter, Racing
- v0.3: + Rhythm, Fishing, Memory
- v1.0: 10~15 mini games + 성능 디버그 화면 (FPS, frame time, draw calls, memory)
