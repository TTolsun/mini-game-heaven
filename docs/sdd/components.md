---
status: ok
section: components
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 컴포넌트 구조와 책임

| 구성 | 책임 |
|---|---|
| BattleData | 세 동료·네 적·열 기술·다섯 맵·격자와 열거형 정의입니다. |
| HeroProgress / Unit | 장기 성장·수련·장비·발견과 전투 위치·체력·기·행동·추격 상태를 나눕니다. |
| BattleState | 캠페인 단계·관계·자원·보상 이력·편성·배치·성장과 출전 체크포인트를 소유합니다. |
| Command / Outcome / BattleEvent | 인물·이동·행동·대상을 입력받아 판정 요약과 최대 40개 이벤트를 반환합니다. |
| BattleModel | 단계별 명령, BFS·시야·기술·추격·적 행동·성장·검증된 저장을 처리하는 값 타입입니다. |
| BattleScene | 모델·글꼴·효과음·BattleArt·입력·선택·모달·연출 상태를 소유합니다. Engine 포인터는 비소유 참조입니다. |
| BattleArt | 팔레트·부품으로 8벌의 픽셀 유닛, 32개 타일과 도장을 한 번 생성하고 별도 Texture로 소유합니다. 초상화 PNG를 읽고 셀 UV로 표시합니다. |
| BattleTimeline | 명령 전 유닛과 Outcome으로 시각 위치·체력·방향을 계산하는 순수 함수입니다. |
| Viewport | 안전 영역 안에 16:9 영역을 계산하고 터치를 역변환합니다. |

근거: app/srpg/BattleData.h, BattleModel.h, BattleScene.h.

Engine은 Scene·SpriteBatch·TextureAtlas를 unique_ptr로, Mixer·Timer·렌더 Texture를 값으로 소유합니다. 1280×720 framebuffer는 Engine 소멸 때 해제합니다. BattleArt의 Texture는 RAII로 해제합니다. Font는 공용 아틀라스 글리프를 참조합니다. Mixer의 Sound 참조보다 Scene의 Sfx 수명이 길어야 합니다. AppState의 선언 역순 파괴로 AndroidAudio가 Engine보다 먼저 닫힙니다. 근거: engine/Engine.h, engine/audio/Mixer.h, platform/android/AndroidMain.cpp.

캠페인과 전투는 현재 단일 작은 BattleModel에 있고 별도 Campaign 클래스나 비동기 AI는 없습니다. Android/GLES 의존성은 모델 밖에 유지합니다.
