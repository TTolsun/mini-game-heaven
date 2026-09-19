---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 아군 명령과 진영 전환

execute는 단계·행동 여부·이동·사거리·시야·해금·기·대상 조건을 검증하고 실패 시 변경하지 않습니다. 이동과 사거리 이탈, 행동, 피해·밀기·충돌·추격을 처리합니다. 각 동료 추격은 한 번이며 재귀하지 않습니다. 마지막 살아 있는 아군이 행동하면 enemyTurn을 같은 명령 안에서 실행합니다. 적은 지형 BFS로 접근하고 사거리 안의 아군을 공격합니다. 로시 전투 불능, 적 전멸, 라운드 제한 순으로 상태를 정합니다.

근거: app/srpg/BattleModel.cpp, app/srpg/BattleScene.cpp, engine/Engine.cpp, platform/android/AndroidMain.cpp.

[시나리오 목록](index.md)
