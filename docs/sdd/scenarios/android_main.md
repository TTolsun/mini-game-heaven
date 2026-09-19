---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# Android 시작과 루프

android_main은 AppState와 플랫폼 서비스를 구성하고 native_app_glue 명령·입력을 처리합니다. 윈도우 생성 때 EGL surface와 Engine 그래픽을 준비하고 BattleScene을 설치합니다. 입력 처리 뒤 Engine::frame과 swap을 수행합니다. surface 재생성 때 기존 모델 상태를 유지합니다.

근거: app/srpg/BattleModel.cpp, app/srpg/BattleScene.cpp, engine/Engine.cpp, platform/android/AndroidMain.cpp.

[시나리오 목록](index.md)
