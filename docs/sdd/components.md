---
status: ok
section: components
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 컴포넌트 구조와 책임

## app

| 구성 | 책임과 수명 |
|---|---|
| BattleData | 격자·행동·단서·기술 수치와 세 수련의 읽기 전용 정의입니다. |
| BattleState | 현재 수련·턴·두 인물의 위치·체력·기·배움·승패를 저장합니다. |
| Command / Outcome | 목적지와 행동을 입력받고 피해·새 단서·체득·밀쳐내기 결과를 반환합니다. |
| BattleModel | BattleState를 값으로 소유합니다. BFS 이동, 시야·공격선, 명령 처리, 재도전과 파일 저장을 수행합니다. |
| BattleScene | BattleModel·Font·Sfx·입력 대상 목록을 소유합니다. Engine 포인터는 소유하지 않는 참조입니다. |
| Sfx | 시작 시 합성한 클릭·타격·깨달음 효과음의 PCM 데이터를 소유합니다. |

근거: `app/srpg/BattleData.h`, `app/srpg/BattleModel.h`, `app/srpg/BattleScene.h`, `app/Sfx.h`.

## engine

Engine은 현재 Scene, SpriteBatch와 TextureAtlas를 unique_ptr로 소유하며, Mixer·Timer를 값으로 보유합니다. Font는 공용 아틀라스에 구운 글리프를 참조합니다. SpriteBatch는 사각형을 모아 그립니다. Mixer는 재생 중인 Sound를 참조하므로 Sound가 더 오래 살아 있어야 합니다.

근거: `engine/Engine.h`, `engine/graphics/Font.h`, `engine/graphics/SpriteBatch.h`, `engine/audio/Mixer.h`.

## platform

AppState는 AndroidAssetLoader·Engine·AndroidHaptics·AndroidAudio·GlContext를 소유합니다. 선언 순서의 역순으로 해제되므로 AndroidAudio는 Engine보다 먼저 파괴됩니다. EGL display/context/surface는 GlContext가 소유하고, 윈도우 해제에서는 surface만 제거합니다.

근거: `platform/android/AndroidMain.cpp:28`, `platform/android/GlContext.cpp`.

## 명령 경계

Scene은 화면과 입력을 담당하고 Model은 규칙을 담당합니다. 모델은 Android·GLES 객체를 소유하지 않습니다. 미리보기 복사본과 실제 모델에 같은 execute를 사용하므로 별도의 피해 예측 공식을 두지 않습니다. 이 시제품은 다중 부대용 AI나 캠페인 클래스를 아직 갖고 있지 않습니다.
