---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 캠페인과 출전 체크포인트 저장

save는 validState를 통과한 WUTEN 2 상태를 임시 파일에 쓰고 flush·close 오류를 검사한 뒤 rename으로 교체합니다. load는 후보 전체를 읽고 숫자·bool·범위·배치 순열·완료 비트·비전투 빈 Unit·전투 점유·승패를 검사합니다. 후행 데이터나 불완전 파일을 거부하며 실패 시 기존 상태를 유지합니다. 이전 WUTEN 1 파일은 별도 파일명으로 유지합니다.

근거: app/srpg/BattleModel.cpp, app/srpg/BattleScene.cpp, engine/Engine.cpp, platform/android/AndroidMain.cpp.

[시나리오 목록](index.md)
