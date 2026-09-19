---
status: ok
section: overview
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 시스템 개요

무천은 C++20 Android 무술 SRPG입니다. 현재는 세 개의 수련전에서 간파·모방·체득, 기 축적과 위험, 방향 예고와 밀쳐내기를 검증합니다.

| 영역 | 역할 |
|---|---|
| app/srpg | 전투 상태·명령·규칙·저장과 전투 화면을 담당합니다. |
| engine | 렌더링·글꼴·입력·시간·음향의 공용 기반입니다. |
| platform/android | GameActivity·EGL·AAudio·JNI 햅틱을 연결합니다. |

윈도우 생성 때 AndroidMain이 그래픽을 초기화하고 BattleScene을 설치합니다. 게임 루프는 입력을 전달하고 Engine::frame을 호출합니다. 근거: `platform/android/AndroidMain.cpp:54`, `platform/android/AndroidMain.cpp:156`.

전투 상태는 매 프레임 자동 진행하지 않습니다. BattleScene::act가 유효한 확정 명령을 BattleModel::execute에 전달할 때 아군 행동과 상대 응답이 함께 처리됩니다. preview는 동일 명령을 복사본에서 실행합니다. 근거: `app/srpg/BattleScene.cpp:195`, `app/srpg/BattleModel.cpp:70`, `app/srpg/BattleModel.cpp:74`.

읽는 순서는 AndroidMain → Engine::onTouch → BattleScene::onTouch/act → BattleModel::preview/execute → BattleScene::render입니다. engine은 app과 platform을 참조하지 않지만 현재 그래픽 구현은 GLES에, 로그는 Android API에 의존합니다.

검토자는 실제 코드와 위 흐름을 대조했습니다. 실행 검증은 [검증 기록](../validation.md), 책임 분리는 [컴포넌트](components.md)를 참조합니다.
