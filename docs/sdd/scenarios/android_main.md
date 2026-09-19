---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 앱 시작과 메인 루프

윈도우 생성 이벤트에서 GlContext가 surface를 만들고 Engine의 그래픽을 초기화합니다. 안전 영역을 적용한 뒤 BattleScene을 설치하여 onEnter를 실행합니다. 이후 android_main 루프에서 이벤트와 입력을 처리하고, surface와 포커스가 있으면 프레임을 그립니다.

윈도우 해제는 surface를 제거하고, 포커스 상실은 오디오를 멈춥니다. 앱 종료 시 AppState 소유 객체가 정리됩니다. 근거: `platform/android/AndroidMain.cpp:54`, `platform/android/AndroidMain.cpp:156`.

[시나리오 목록](index.md)
