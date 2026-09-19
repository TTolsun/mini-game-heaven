---
status: ok
section: scenarios
reviewer: Codex
reviewed: 2026-09-19
---

# 방어전 시작

startRaid는 준비 상태이고 캠페인이 완료되지 않았을 때만 성공합니다. 방어 상태로 바꾸고 성 체력, 처치·등장 수, 전투 시간과 생성 타이머를 초기화합니다. `app/castle/CastleModel.cpp:116`

용사 수는 min(12, 2+day)이며 용사 배열을 비웁니다. 모든 마물의 체력을 시설·레벨을 반영한 최대치로 회복하고 공격 대기 시간을 초기화합니다. Scene은 일시정지를 풀고 준비 상태로 복원할 수 있는 저장 파일을 기록합니다. `app/castle/CastleModel.cpp:45`, `app/castle/CastleScene.cpp:329`

10일 방어를 완료하면 새 방어는 시작할 수 없습니다. 준비 화면에서 남은 재화로 성장·합성·시설 관리는 가능하지만 추가 보상 획득은 현재 구현 범위에 없습니다.
