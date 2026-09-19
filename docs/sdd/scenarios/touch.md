---
status: ok
section: scenarios
reviewer: Codex
reviewed: 2026-09-19
---

# 터치 입력 전달

Android 입력 버퍼의 포인터 좌표가 Engine::onTouch에서 월드 좌표로 변환되어 Scene에 전달됩니다. `platform/android/AndroidMain.cpp:113`, `engine/Engine.cpp:116`

CastleScene은 렌더링할 때 활성 버튼의 사각형·행동·대상을 기록합니다. Down에서 한 포인터만 선택하고 Up이 같은 영역에 있으며 현재도 동일한 대상이 활성일 때 행동을 실행합니다. Cancel은 실행하지 않고 포인터를 해제합니다. `app/castle/CastleScene.cpp:338`

합성·결과 오버레이는 뒤쪽 버튼 목록을 제거합니다. 행동을 실행하면 목록을 비워 같은 Android 입력 묶음의 후속 터치가 이전 화면의 대상을 사용하지 못하게 합니다. 다음 render에서 목록을 다시 만듭니다. BACK도 목록과 진행 중인 제스처를 취소합니다. `app/castle/CastleScene.cpp:305`, `app/castle/CastleScene.cpp:355`

회귀 검증은 CastleSceneTests에서 실제 Scene의 hit 영역과 이벤트 전달을 사용합니다. 합성 확인 직후 연속 탭, BACK 직후 Up, 다음 프레임의 입력 복구를 검사합니다. 렌더러 출력·GPU 검증은 이 무화면 테스트에 포함하지 않습니다.
