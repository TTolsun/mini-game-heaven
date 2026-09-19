---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 터치·확정과 모달

Engine::onTouch는 카메라 구멍·시스템 바·여백을 반영한 viewport를 사용해 화면 좌표를 1280×720 좌표로 바꿉니다. 여백의 터치는 Cancel로 전달해 경계 밖 Down/Up이 버튼을 누르지 못하게 합니다. BattleScene은 현재 hit 목록의 Down/Up 일치를 검사하고 act로 전달합니다. act는 먼저 hit와 포인터를 폐기합니다. 화면 변경 직후 들어온 중복 이벤트가 새 명령을 실행하지 못합니다. BACK은 모달·미확정 이동을 취소하고 배치에서는 도장으로 돌아갑니다.

근거: app/srpg/BattleModel.cpp, app/srpg/BattleScene.cpp, engine/Engine.cpp, platform/android/AndroidMain.cpp.

[시나리오 목록](index.md)
