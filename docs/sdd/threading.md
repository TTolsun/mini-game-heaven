---
generated_at: 2026-09-17T14:43:32+00:00
source_commit: b4e016048393b0285580f4897dc534a99b59117d
agent: ollama/qwen3.5:4b
status: needs-review
section: threading
needs_human: true
---

# 스레드와 콜백 모델

**게임 스레드(android_main), 오디오 콜백 스레드(AAudio), Java 메인 스레드가 만나는 지점은 정적 분석으로 다 나오지 않습니다. 아래 표의 클래스를 수정하기 전에 실행 스레드를 직접 확인하세요.**

| 지금 확인할 내용 | 이동할 절 |
|---|---|
| 관련 클래스 절을 확인합니다. | [관련 클래스](#관련-클래스) |
| 구조 설명 절을 확인합니다. | [구조 설명](#구조-설명) |

## 관련 클래스

| 클래스 | 선언 위치 | 상속 | 책임 (주석) |
|---|---|---|---|
| `engine::Animation` | `engine/graphics/Animation.h:13` | – | A sequence of atlas sprites played at a fixed frame rate. |
| `engine::AssetLoader` | `engine/asset/AssetLoader.h:11` | – | Platform-neutral file access. The Android implementation reads from the APK's AAssetManager; a desktop build could read from disk. |
| `engine::Color` | `engine/math/Color.h:5` | – | 확인 필요 |
| `engine::Engine` | `engine/Engine.h:24` | – | Platform-independent core: owns the renderer, the sprite atlas, the audio mixer and the active scene, and maps screen pixels to a fixed-width world. |
| `engine::Font` | `engine/graphics/Font.h:22` | – | Bitmap font baked from a TTF with stb_truetype into the shared atlas. Glyphs are baked once at kBakePixelHeight and scaled when drawn, which is fine for the sizes a phone HUD uses. |
| `engine::Font::Glyph` | `engine/graphics/Font.h:40` | – | 확인 필요 |
| `engine::Haptics` | `engine/platform/Haptics.h:7` | – | Vibration feedback. The Android implementation goes through JNI; the default no-op keeps games free of null checks. |
| `engine::Haptics::Strength` | `engine/platform/Haptics.h:9` | – | 확인 필요 |
| `engine::Image` | `engine/asset/Image.h:11` | – | Decoded RGBA8 pixels in CPU memory. |
| `engine::Mixer` | `engine/audio/Mixer.h:14` | – | Polyphonic sample mixer. play() is called from the game thread, render() from the audio callback; voices are claimed with atomics so neither side blocks. Sounds must outlive any voice playing them. |
| `engine::Mixer::Voice` | `engine/audio/Mixer.h:27` | – | 확인 필요 |
| `engine::NullHaptics` | `engine/platform/Haptics.h:19` | `engine::Haptics` | 확인 필요 |
| `engine::Particles` | `engine/graphics/Particles.h:16` | – | Lightweight CPU particle pool: bursts of squares (or a sprite) with gravity, drag, spin and fade. Enough for dust, confetti and hit sparks. |
| `engine::Particles::Burst` | `engine/graphics/Particles.h:18` | – | 확인 필요 |
| `engine::Particles::Particle` | `engine/graphics/Particles.h:44` | – | 확인 필요 |
| `engine::Rect` | `engine/math/Rect.h:9` | – | Axis-aligned rectangle. Origin is the top-left corner; y grows downward, matching screen coordinates. |
| `engine::Scene` | `engine/Scene.h:12` | – | One screen of the app (menu, a mini game, result). The engine drives exactly one scene at a time. |
| `engine::Shader` | `engine/graphics/Shader.h:10` | – | Compiled + linked GLSL ES program. |
| `engine::Sound` | `engine/audio/Sound.h:8` | – | Mono PCM clip at Mixer::kSampleRate. |
| `engine::Sprite` | `engine/graphics/Sprite.h:11` | – | A rectangular region of a texture, in normalized UV coordinates, plus the region's size in pixels so callers can draw it at native scale. |
| `engine::SpriteBatch` | `engine/graphics/SpriteBatch.h:21` | – | Accumulates textured quads and submits them in as few draw calls as possible. A flush happens when the bound texture changes or the buffer fills up. |
| `engine::SpriteBatch::Vertex` | `engine/graphics/SpriteBatch.h:55` | – | 확인 필요 |
| `engine::TextAlign` | `engine/graphics/Font.h:17` | – | 확인 필요 |
| `engine::Texture` | `engine/graphics/Texture.h:12` | – | GPU texture (RGBA8). Owns the GL object. |
| `engine::TextureAtlas` | `engine/graphics/TextureAtlas.h:18` | – | Packs many small images into one GPU texture at load time so the sprite batch can draw everything with a single texture bind. Uses shelf packing: images fill a row left to right; when a row is full, a |
| `engine::Timer` | `engine/core/Timer.h:9` | – | Frame timer. tick() returns the seconds elapsed since the previous tick, clamped so a long pause (app switch, debugger) does not explode physics. |
| `engine::TouchEvent` | `engine/input/TouchEvent.h:10` | – | Platform-neutral touch event in world coordinates. |
| `engine::TouchEvent::Phase` | `engine/input/TouchEvent.h:11` | – | 확인 필요 |
| `engine::Vec2` | `engine/math/Vec2.h:7` | – | 확인 필요 |
| `platform::android::AndroidAudio` | `platform/android/AndroidAudio.h:12` | – | Low-latency output stream that pulls samples from the engine Mixer. |
| `platform::android::AndroidHaptics` | `platform/android/AndroidHaptics.h:13` | `engine::Haptics` | Drives android.os.Vibrator through JNI. There is no NDK vibration API, so this is the one place the project touches Java at runtime. |
| `platform::android::GlContext` | `platform/android/GlContext.h:12` | – | Owns the EGL display and context for the app's lifetime, and a window surface that comes and goes with the Activity's window. Keeping the context across surface loss means GL textures and buffers surv |

## 구조 설명

`engine::Engine` 은 렌더러, 스프라이트 아틀라스, 오디오 믹서 및 활성 시나리오를 소유하며 `frame()` 메서드를 통해 게임 스레드에서 한 번의 업데이트와 렌더링 패스를 수행합니다 `engine/Engine.cpp:77`. 오디오 믹서는 `play()` 를 게임 스레드에서 호출하고 `render()` 를 오디오 콜백에서 호출하여 음원 소리를 원자적으로 할당합니다 `engine/audio/Mixer.cpp:8`, `engine/audio/Mixer.cpp:27`.

`platform::android::AndroidAudio` 는 엔진의 믹서에서 샘플을 추출하는 저지연 출력 스트림을 소유하며 `onData()` 를 통해 오디오 데이터를 처리합니다 `platform/android/AndroidAudio.cpp:72`. 이 클래스는 JNI 를 통해 Java 측과 통신하므로 스레드 경계에서 호출되는 유일한 플랫폼 특화 메서드입니다 `platform/android/AndroidHaptics.cpp:102`.

`engine::Scene` 은 화면의 한 장을 소유하며 `onEnter()`, `update()`, `render()` 와 같은 메서드를 통해 게임 로직을 관리합니다 `engine/Scene.h:16`, `engine/Scene.h:18`, `engine/Scene.h:19`. 시나리오는 `engine::Engine` 에 의해 구동되며 스레드 소유권은 엔진에 집중되어 있습니다.

`platform::android::AndroidHaptics` 는 `engine::Haptics` 인터페이스를 구현하며 JNI 를 통해 `android.os.Vibrator` 를 구동합니다 `platform/android/AndroidHaptics.cpp:102`. 이 클래스는 NDK vibration API 가 없으므로 프로젝트가 런타임에 Java 를 만나는 유일한 장소입니다.

`engine::SpriteBatch` 는 텍스처화된 쿼드를 누적하고 버퍼가 가득 차거나 바인딩된 텍스처가 변경될 때 플러시합니다 `engine/graphics/SpriteBatch.cpp:204`. `begin()` 과 `end()` 메서드는 프레임당 오르토그래픽 투영을 설정하고 화면 진동과 같은 효과를 위해 모든 게임 측 위치를 건드리지 않고 시프트합니다 `engine/graphics/SpriteBatch.cpp:120`, `engine/graphics/SpriteBatch.cpp:132`.

`engine::Texture` 는 GPU 텍스처 (RGBA8) 를 소유하며 GL 객체를 관리합니다 `engine/graphics/Texture.h:12`. `upload()` 메서드는 RGBA8 픽셀을 텍스처에 복사하고 `bind()` 는 텍스처를 활성화합니다 `engine/graphics/Texture.cpp:51`, `engine/graphics/Texture.cpp:56`.

`platform::android::GlContext`는 앱의 수명 동안 EGL 디스플레이와 컨텍스트, 그리고 Activity 의 윈도우와 함께 오고 가는 윈도우 서페이스를 소유합니다 `platform/android/GlContext.h:12`. 컨텍스트를 서페이스 손실 후에도 유지함으로써 GL 텍스처와 버퍼가 생존하게 합니다 `platform/android/GlContext.cpp:96`.

`engine::Mixer::Voice` 는 음원 소리를 재생할 때 원자적으로 할당되며 음원은 재생 중인 음원을 넘어서는 동안 살아있어야 합니다 `engine/audio/Mixer.h:27`. 믹서는 음원 소유권을 관리하기 위해 원자기를 사용합니다.

`engine::Font` 는 TTF 를 `stb_truetype` 로 변환하여 공유 아틀라스에 베이킹한 비트맵 폰트를 소유합니다 `engine/graphics/Font.cpp:23`. 글꼴은 한 번 베이킹된 후 크기에 따라 스케일링되어 모바일 HUD 의 크기에 적합합니다.

`engine::Particles` 은 중력, 마찰력, 회전 및 페이드 효과를 가진 가벼운 CPU 파티클 풀을 소유하며 `emit()` 와 `update()` 를 통해 파티클 버스트를 생성하고 업데이트합니다 `engine/graphics/Particles.cpp:11`, `engine/graphics/Particles.cpp:41`.

`engine::Shader` 는 컴파일 및 링크된 GLSL ES 프로그램을 소유하며 `build()` 와 `use()` 를 통해 그래픽 파이프라인에 통합합니다 `engine/graphics/Shader.cpp:35`, `engine/graphics/Shader.cpp:64`.

`engine::Sound` 은 믹서의 `kSampleRate` 의 모노 PCM 클립을 소유하며 `empty()` 메서드를 통해 유효성을 확인합니다 `engine/audio/Sound.h:11`.

`engine::Image` 는 CPU 메모리에 디코딩된 RGBA8 픽셀을 소유하며 `load()` 를 통해 PNG/JPG 를 디코딩합니다 `engine/asset/Image.cpp:14`.

`engine::Animation` 은 고정 프레임 레이트로 재생되는 아틀라스 스프라이트 시퀀스를 소유하며 `update()` 와 `frame()` 을 통해 애니메이션을 제어합니다 `engine/graphics/Animation.cpp:27`, `engine/graphics/Animation.cpp:38`.

`engine::AssetLoader` 는 플랫폼 중립적인 파일 접근을 소유하며 `readFile()` 를 통해 전체 파일을 반환합니다 `engine/asset/AssetLoader.h:16`.

`engine::Rect` 은 축 정합 사각형을 소유하며 원점 상단 왼쪽 모서리에서 시작하여 y 가 아래로 증가하는 화면 좌표계를 따릅니다 `engine/math/Rect.h:9`.

`engine::Vec2` 는 2 차원 벡터를 소유하며 연산자重载을 통해 벡터 연산을 수행합니다 `engine/math/Vec2.h:14`, `engine/math/Vec2.h:15`.

`engine::TouchEvent` 은 세계 좌표에서 플랫폼 중립적인 터치 이벤트를 소유하며 `onTouch()` 를 통해 입력을 처리합니다 `engine/Scene.h:20`.

`engine::Timer` 는 프레임 타이머를 소유하며 `tick()` 을 통해 이전 틱부터의 경과 시간을 반환합니다 `engine/core/Timer.h:15`.

`engine::Sprite` 은 텍스처의 정규화된 UV 좌표와 픽셀 크기를 소유하며 `size()` 를 통해 원본 스케일을 알 수 있습니다 `engine/graphics/Sprite.h:21`.

`engine::TextureAtlas` 는 로드 시간 동안 여러 작은 이미지를 하나의 GPU 텍스처에 패킹합니다 `engine/graphics/TextureAtlas.cpp:22`. 스프라이트 배치에서 단일 텍스처 바인딩으로 모든 것을 그릴 수 있도록 합니다.

`platform::android::AndroidAudio` 는 엔진 믹서에서 샘플을 추출하는 저지연 출력 스트림을 소유하며 `start()` 와 `stop()` 를 통해 오디오 플로우를 제어합니다 `platform/android/AndroidAudio.cpp:15`, `platform/android/AndroidAudio.cpp:63`.

`engine::Scene` 의 `onBack()` 메서드는 시스템 백 버튼을 처리하며 true 를 반환하면 소비되고 false 를 반환하면 앱이 종료됩니다 `engine/Scene.h

확인 필요: 이 절의 내용은 정적 분석 결과입니다. 콜백 실행 스레드와 종료 순서는 코드를 직접 실행해서 확인해야 합니다.

??? note "근거와 검토 정보"
    - 근거 파일: `engine/Engine.cpp`, `engine/Engine.h`, `engine/Scene.h`, `engine/asset/AssetLoader.h`, `engine/asset/Image.cpp`, `engine/asset/Image.h`, `engine/audio/Mixer.cpp`, `engine/audio/Mixer.h`, `engine/audio/Sound.h`, `engine/core/Timer.h`, `engine/graphics/Animation.cpp`, `engine/graphics/Animation.h`, `engine/graphics/Font.cpp`, `engine/graphics/Font.h`, `engine/graphics/Particles.cpp`, `engine/graphics/Particles.h`, `engine/graphics/Shader.cpp`, `engine/graphics/Shader.h`, `engine/graphics/Sprite.h`, `engine/graphics/SpriteBatch.cpp`, `engine/graphics/SpriteBatch.h`, `engine/graphics/Texture.cpp`, `engine/graphics/Texture.h`, `engine/graphics/TextureAtlas.cpp`, `engine/graphics/TextureAtlas.h`, `engine/input/TouchEvent.h`, `engine/math/Color.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`, `engine/platform/Haptics.h`, `platform/android/AndroidAudio.cpp`, `platform/android/AndroidAudio.h`, `platform/android/AndroidHaptics.cpp`, `platform/android/AndroidHaptics.h`, `platform/android/GlContext.cpp`, `platform/android/GlContext.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `b4e0160483`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [변경 시 지켜야 할 제약](constraints.md)
