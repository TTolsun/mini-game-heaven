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
| Audio | AAudio + 자체 믹서, 효과음은 코드로 합성 (오디오 파일 없음) |
| Input | GameActivity input |
| Assets | AAssetManager |
| UI | 자체 C++ UI |
| Storage | native file (`highscores.txt`) |
| Haptics | android.os.Vibrator (JNI 한 곳) |
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

- v0.1: Dodge / Jump / Tap (완료: 게임 3개, GUI 팩 UI, 햅틱, 효과음, 점수 저장)
- v0.2: + Shooter, Racing
- v0.3: + Rhythm, Fishing, Memory
- v1.0: 10~15 mini games + 성능 디버그 화면 (FPS, frame time, draw calls, memory)

## 빌드

요구 사항: JDK 17, Android SDK (platform 36, NDK 29.0.14206865, CMake 3.31.6). NDK와 CMake는 `android sdk install ndk/29.0.14206865 cmake/3.31.6`으로 설치한다.

```bash
export JAVA_HOME="C:/Program Files/Eclipse Adoptium/jdk-17.0.20.101-hotspot"
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

`compile_commands.json`은 빌드마다 `app/.cxx/Debug/<hash>/arm64-v8a/`에 생성된다. clangd에서 쓰려면 `tools/sync-compile-commands.sh`로 저장소 루트에 복사한다.

## 문서 (SDD)

문서 검토 기록과 APK 릴리스 절차는 [개발 정책](docs/development-policy.md)을 따릅니다. 최초에는 `bash tools/setup-sdd.sh`로 커밋이 고정된 엔진을 설치합니다.

`docs/sdd/`는 [camera-hal-sdd](https://github.com/TTolsun/camera-hal-sdd) 파이프라인이 `native/` C++ 소스에서 생성한 설계 문서입니다. 표와 시퀀스 다이어그램은 libclang이 읽은 사실에서 도구가 만들고, LLM(로컬 Ollama의 `qwen3.5:4b`)은 서술 문단만 씁니다. 문장마다 `파일:줄` 인용이 붙고, 인용이 사실에 없으면 `status: needs-review`로 남습니다. 단일 HTML은 `docs/index.html`입니다.

정책은 hal-camera와 같습니다. 코드와 문서가 어긋난 채로 `main`에 들어가지 않습니다.

| 언제 | 무엇을 | 어떻게 |
|------|--------|--------|
| PR마다 (CI `docs-check`) | 사실 최신성, 문서 최신성, 사이트 일치를 검사 | `tools/docs-check.sh --base <ref>` 와 같은 단계. LLM 없음 |
| `main` push 후 (CI `docs-sync`, 로컬 러너) | 영향받은 절을 Qwen으로 다시 써서 PR 생성 | `tools/docs-sync.sh`. 러너 변수 `DOCGEN_LOCAL_RUNNER_ENABLED=true` 필요. 검토 기록은 자동 승인하지 않음 |
| 로컬에서 코드를 바꾼 뒤 | 오래된 문서 확인 → 다시 생성 → 검토 → 커밋 | 아래 순서 |

```bash
bash tools/docs-check.sh                 # 어느 문서가 오래됐는지 (base: origin/main 과의 merge-base)
bash tools/docs-sync.sh                  # 마지막 동기화 이후 영향 절만 Qwen 으로 재생성 (--force: 전체)
bash tools/docs-sync.sh --site           # LLM 없이 facts.json 과 docs/index.html 만 재생성
```

요구 사항: `tools/setup-sdd.sh`로 설치한 엔진, `uv`, Node.js 24, 그리고 `./gradlew assembleDebug`가 만든 `compile_commands.json`입니다. `tools/sync-compile-commands.sh`가 compile DB를 갱신하고 libclang용 `-resource-dir`를 붙입니다. 섹션과 시나리오 정의는 `docs/sdd-config/`, 파이프라인 설정은 `sdd.yaml`입니다. 수동 문서 `constraints.md`와 `decisions.md`는 덮어쓰지 않습니다. 문서를 코드와 대조한 뒤 `node tools/docs-review.mjs accept --reviewer NAME --reason TEXT`로 검토를 기록하고 `tools/docs-check.sh`를 실행합니다.
