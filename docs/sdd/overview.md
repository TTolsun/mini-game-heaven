---
status: ok
section: overview
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 시스템 개요

무천은 C++20 Android SRPG입니다. 현재는 스토리 → 도장 → 출전 → 진영 턴 전투 → 보상 루프를 다섯 맵에 연결합니다. 캐릭터 3명, 적 4종, 기술 10개를 고정 데이터로 정의합니다.

| 영역 | 역할 |
|---|---|
| app/srpg | 캠페인 상태·전투·성장·연구·저장·화면을 담당합니다. |
| engine | 렌더링·글꼴·입력·시간·음향의 공용 기반입니다. |
| platform/android | GameActivity·EGL·AAudio·JNI 햅틱을 연결합니다. |

AndroidMain은 윈도우 생성 때 그래픽을 초기화하고 BattleScene을 설치합니다. 루프가 입력과 Engine::frame을 전달합니다. BattleScene::act는 현재 단계에 맞는 모델 명령을 실행하고 성공한 상태를 저장합니다. BattleModel::execute는 한 아군의 이동·행동을 처리하고 모든 아군이 행동했을 때 enemyTurn을 이어 실행합니다. 수동 턴 종료도 같은 enemyTurn을 사용합니다.

preview는 모델 복사본에서 execute를 호출합니다. Outcome의 고정 크기 이벤트 목록으로 이동·충돌·추격 등을 표현합니다. 판정은 프레임 시간과 분리되며 표시는 BattleTimeline에서 이벤트별 시간에 따라 순차 재생합니다. Chase·Collision 예고와 실제 Hit 피해를 구분합니다. BattleArt가 픽셀 유닛·타일·도장과 초상화를 공급합니다. Engine은 1280×720 타깃을 안전 영역에 최근접 확대하고 같은 영역으로 터치를 역변환합니다. 근거: app/srpg/BattleModel.cpp, BattleScene.cpp.

읽는 순서는 AndroidMain → Engine::onTouch → BattleScene::onTouch/act → BattleModel → BattleScene::render입니다. engine은 app과 platform을 참조하지 않지만 현재 그래픽은 GLES, 로그는 Android API를 사용합니다. 검증 범위는 [실행 기록](../validation.md)을 참조합니다.
