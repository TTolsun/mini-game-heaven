---
status: ok
section: scenarios
reviewer: Codex
reviewed: 2026-09-19
---

# 앱 시작과 메인 루프

android_main은 AppState를 만들고 onAppCmd와 입력 필터를 등록합니다. AppState는 자산 로더, Engine, 햅틱, 오디오, GL을 소유합니다. `platform/android/AndroidMain.cpp:156`, `platform/android/AndroidMain.cpp:28`

이벤트 루프는 surface와 포커스가 없으면 대기합니다. APP_CMD_INIT_WINDOW에서 surface와 초기 그래픽·CastleScene을 준비하고, 복귀 시에는 크기와 타이머를 갱신합니다. 포커스를 잃으면 오디오를 중지하고 프레임 진행을 멈춥니다. 윈도우 종료 시 surface를 해제합니다. `platform/android/AndroidMain.cpp:54`

모든 대기 이벤트를 처리한 뒤 processInput을 호출하고, 활성 상태라면 Engine::frame과 swapBuffers를 수행합니다. BACK은 Scene이 처리하지 않으면 GameActivity_finish로 종료합니다. `platform/android/AndroidMain.cpp:113`

native_app_glue의 콜백 전달과 AAudio 콜백 타이밍은 외부 플랫폼 경계입니다. 이 설명은 코드의 호출 순서를 확인한 것이며 모든 기기에서 생명주기 스트레스 테스트를 했다는 의미는 아닙니다.
