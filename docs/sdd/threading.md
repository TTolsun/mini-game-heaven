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

게임 루프에서 장면 업데이트·입력·렌더링을 수행하고, AAudio의 데이터 콜백에서 음원을 혼합합니다. `AndroidAudio::onData`는 `Mixer::render` 결과를 출력 채널에 복사합니다. 오디오는 AAudio를 사용하며 JNI 진동 처리는 `AndroidHaptics`의 책임입니다 (`platform/android/AndroidAudio.cpp:72`, `platform/android/AndroidHaptics.cpp:102`).

`Mixer::play`는 빈 보이스의 필드를 먼저 설정한 뒤 음원 포인터를 release-store로 공개합니다. `render`는 acquire-load로 읽고, 재생 완료 시 nullptr를 저장한 다음 해당 보이스를 더 수정하지 않습니다 (`engine/audio/Mixer.cpp:8`, `engine/audio/Mixer.cpp:27`). 포인터는 음원 소유권을 이전하지 않으므로 음원이 재생보다 오래 살아야 합니다. 이 규칙은 게임 스레드 하나가 play를 호출한다는 전제입니다.

`AppState`에서는 `AndroidAudio`가 `Engine`보다 먼저 파괴되도록 멤버가 배치되어 있습니다. 포커스를 잃으면 오디오를 중지하며, surface가 없거나 포커스가 없으면 프레임 렌더링을 하지 않습니다 (`platform/android/AndroidMain.cpp:27`).

확인 필요: 콜백 타이밍, 기기별 스케줄링, 오디오 중지·재개 및 GL 리소스 종료 동작은 이번 정적 코드 검토로 검증하지 않았습니다. 실제 기기에서 별도 검증해야 합니다.

??? note "근거와 검토 정보"
    - 근거 파일: `engine/Engine.cpp`, `engine/Engine.h`, `engine/Scene.h`, `engine/asset/AssetLoader.h`, `engine/asset/Image.cpp`, `engine/asset/Image.h`, `engine/audio/Mixer.cpp`, `engine/audio/Mixer.h`, `engine/audio/Sound.h`, `engine/core/Timer.h`, `engine/graphics/Animation.cpp`, `engine/graphics/Animation.h`, `engine/graphics/Font.cpp`, `engine/graphics/Font.h`, `engine/graphics/Particles.cpp`, `engine/graphics/Particles.h`, `engine/graphics/Shader.cpp`, `engine/graphics/Shader.h`, `engine/graphics/Sprite.h`, `engine/graphics/SpriteBatch.cpp`, `engine/graphics/SpriteBatch.h`, `engine/graphics/Texture.cpp`, `engine/graphics/Texture.h`, `engine/graphics/TextureAtlas.cpp`, `engine/graphics/TextureAtlas.h`, `engine/input/TouchEvent.h`, `engine/math/Color.h`, `engine/math/Rect.h`, `engine/math/Vec2.h`, `engine/platform/Haptics.h`, `platform/android/AndroidAudio.cpp`, `platform/android/AndroidAudio.h`, `platform/android/AndroidHaptics.cpp`, `platform/android/AndroidHaptics.h`, `platform/android/GlContext.cpp`, `platform/android/GlContext.h`
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `b4e0160483`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [변경 시 지켜야 할 제약](constraints.md)
