---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 배움을 유지하며 다음 수련 시작

nextLesson은 승리 상태이고 다음 수련이 있을 때만 진행합니다. 현재 단서와 모방 성공 횟수를 다음 수련 입장 상태로 넘기고 위치·체력·기·턴을 초기화합니다.

retry는 이번 수련의 입장 단서·성공 횟수로 되돌립니다. 실패한 수련을 반복해서 지식을 복제하지 않습니다. restart는 첫 수련과 빈 배움으로 시작합니다. 마지막 수련 뒤에는 잘못된 다음 인덱스로 넘어가지 않습니다. 근거: `app/srpg/BattleModel.cpp:193`, `app/srpg/BattleModel.cpp:203`.

[시나리오 목록](index.md)
