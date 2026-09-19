---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 한 프레임과 표현

Engine::frame은 프레임 시간으로 Scene::update를 호출하고 배치 렌더링을 수행합니다. update는 표시 시간을 진행하고 현재 타격의 피드백을 한 번 발생시킵니다. 50ms hit-stop은 실제 시간으로 소진하며 전투 모델을 다시 실행하지 않습니다. BattleScene::render는 캠페인 단계에 따라 이야기·도장·배치·전투·결과를 그리고 마지막에 모달을 표시합니다. 연출 중에는 모델이 승패에 도달해도 전투 화면을 유지하고 끝난 뒤 결과 화면을 표시합니다. 명령 판정은 render/update에서 실행하지 않습니다. 프레임은 고정 FBO에 그린 후 GL_NEAREST로 안전한 viewport에 blit합니다.

근거: app/srpg/BattleModel.cpp, app/srpg/BattleScene.cpp, engine/Engine.cpp, platform/android/AndroidMain.cpp.

[시나리오 목록](index.md)
