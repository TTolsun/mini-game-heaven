---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 확정된 턴과 배움 저장

save는 상태를 검증하고 WUTEN 1 헤더와 필드를 임시 파일에 기록합니다. 쓰기·닫기 오류를 검사한 뒤 Android/POSIX rename으로 기존 파일을 교체합니다. 이는 동기 호출이며 결과 콜백이나 재귀는 없습니다.

load는 별도 후보에 전체 필드를 읽습니다. 형식·범위·중복 점유·단서와 성공 횟수의 정합성·승패 상태·후행 데이터를 검사하고 성공한 후보만 반영합니다. 화면의 미확정 이동은 모델 밖에 있어 저장되지 않습니다. 근거: `app/srpg/BattleModel.cpp:209`, `app/srpg/BattleModel.cpp:228`, `app/srpg/BattleModel.cpp:246`.

[시나리오 목록](index.md)
