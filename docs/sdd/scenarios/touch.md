---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 터치 입력 전달

AndroidMain::processInput은 포인터 이벤트를 Engine::onTouch로 전달합니다. Engine은 화면 좌표를 월드 좌표로 바꾸고 BattleScene::onTouch를 호출합니다.

누를 때 대상과 포인터 ID를 기억하고, 뗄 때 동일한 유효 대상 안인지 확인한 후 act를 호출합니다. act와 BACK은 이전 hit 목록과 진행 중인 제스처를 폐기합니다. 이동 미리보기 상태의 BACK은 원래 위치를 복원하고 턴을 소비하지 않습니다.

근거: `platform/android/AndroidMain.cpp:113`, `engine/Engine.cpp:116`, `app/srpg/BattleScene.cpp:195`, `app/srpg/BattleScene.cpp:223`, `app/srpg/BattleScene.cpp:239`.

[시나리오 목록](index.md)
