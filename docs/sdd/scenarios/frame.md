---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 한 프레임과 전투 화면

Engine::frame은 경과 시간을 얻고 Scene::update를 호출한 뒤 배치를 시작해 Scene::render를 호출합니다. BattleScene::update는 짧은 입력 잠금 시간만 줄입니다. 전투 판정은 프레임마다 진행하지 않습니다.

BattleScene::render는 안전 영역·화면 비율에 맞춰 좌표를 정하고 지도·범위·단서·행동·예측을 그립니다. 결과나 안내창이 열리면 기존 입력 대상을 버리고 해당 창의 버튼만 등록합니다. 근거: `engine/Engine.cpp:77`, `app/srpg/BattleScene.cpp:45`, `app/srpg/BattleScene.cpp:107`.

[시나리오 목록](index.md)
