---
generated_at: 2026-09-17T14:41:41+00:00
source_commit: b4e016048393b0285580f4897dc534a99b59117d
agent: ollama/qwen3.5:4b
status: ok
section: scenarios
---

# 핵심 시나리오 시퀀스

**추적하려는 동작의 시나리오를 표에서 고르세요. 각 시나리오는 진입 함수부터 호출 순서를 번호로 보여 줍니다.**

| 지금 확인할 내용 | 이동할 절 |
|---|---|
| 앱 시작과 메인 루프 (android_main) 흐름을 추적합니다. | [앱 시작과 메인 루프 (android_main)](android_main.md) |
| 오디오 콜백 믹싱 (Mixer::render) 흐름을 추적합니다. | [오디오 콜백 믹싱 (Mixer::render)](audio_render.md) |
| 한 프레임 (Engine::frame) 흐름을 추적합니다. | [한 프레임 (Engine::frame)](frame.md) |
| 공유 에셋 로딩 (GameAssets::load) 흐름을 추적합니다. | [공유 에셋 로딩 (GameAssets::load)](load_assets.md) |
| 게임 종료와 결과 화면 (MiniGameApp::showResult) 흐름을 추적합니다. | [게임 종료와 결과 화면 (MiniGameApp::showResult)](show_result.md) |
| 게임 시작 (MiniGameApp::startGame) 흐름을 추적합니다. | [게임 시작 (MiniGameApp::startGame)](start_game.md) |
| 터치 입력 전달 (Engine::onTouch) 흐름을 추적합니다. | [터치 입력 전달 (Engine::onTouch)](touch.md) |

## 시나리오 목록

| 시나리오 | 진입점 | 단계 수 | 미해결 호출 |
|---|---|---|---|
| [앱 시작과 메인 루프 (android_main)](android_main.md) | `android_main(android_app *)` | 105 | 2 |
| [오디오 콜백 믹싱 (Mixer::render)](audio_render.md) | `engine::Mixer::render(float *, int)` | 0 | 0 |
| [한 프레임 (Engine::frame)](frame.md) | `engine::Engine::frame()` | 138 | 15 |
| [공유 에셋 로딩 (GameAssets::load)](load_assets.md) | `app::GameAssets::load(engine::Engine &)` | 151 | 2 |
| [게임 종료와 결과 화면 (MiniGameApp::showResult)](show_result.md) | `app::MiniGameApp::showResult(app::GameId, int)` | 60 | 0 |
| [게임 시작 (MiniGameApp::startGame)](start_game.md) | `app::MiniGameApp::startGame(app::GameId)` | 32 | 0 |
| [터치 입력 전달 (Engine::onTouch)](touch.md) | `engine::Engine::onTouch(int32_t, engine::TouchEvent::Phase, float, float)` | 4 | 0 |

??? note "근거와 검토 정보"
    - 근거 파일: (없음)
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `b4e0160483`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [컴파일 플래그 매트릭스](../feature-flags.md)
