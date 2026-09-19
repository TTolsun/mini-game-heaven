---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 전리품·성장과 다음 이야기

nextLesson은 Victory이고 보상을 아직 받지 않은 맵에서만 실행됩니다. 완료 비트, 보급·약초·행동 경험·중량 보너스·관계를 반영하고 전투 Unit 배열을 비웁니다. 다음 Story 또는 마지막 Complete로 바뀝니다. retry와 returnToBase는 패배 때 출전 체크포인트의 배움·발견·약초를 복원합니다.

근거: app/srpg/BattleModel.cpp, app/srpg/BattleScene.cpp, engine/Engine.cpp, platform/android/AndroidMain.cpp.

[시나리오 목록](index.md)
