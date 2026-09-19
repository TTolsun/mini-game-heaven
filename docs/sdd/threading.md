---
status: needs-review
section: threading
reviewer: Codex
reviewed: 2026-09-19
---

# 스레드와 콜백 모델

| 실행 흐름 | 담당 작업 | 근거 |
|---|---|---|
| android_main 게임 루프 | 이벤트, 터치, 모델 갱신, 렌더링, 파일 저장 | `platform/android/AndroidMain.cpp:156` |
| AAudio 데이터 콜백 | Mixer::render로 PCM을 혼합하고 출력 채널로 복제 | `platform/android/AndroidAudio.cpp:72` |
| JNI 호출 | 게임 스레드에서 AndroidHaptics가 JVM 연결을 관리하고 진동을 요청 | `platform/android/AndroidHaptics.cpp:102` |

Mixer::play는 빈 보이스의 필드를 채운 뒤 release 저장으로 Sound 포인터를 공개합니다. 콜백은 acquire 읽기로 이를 확인하며 재생 종료 후 nullptr를 release 저장합니다. 게임 스레드는 재생 중인 보이스를 덮어쓰지 않습니다. `engine/audio/Mixer.cpp:8`, `engine/audio/Mixer.cpp:27`

CastleScene이 소유한 Sfx는 Scene이 살아 있는 동안 유지됩니다. AppState 멤버 역순 파괴에 따라 AndroidAudio가 Engine보다 먼저 파괴되고 stop()에서 스트림을 닫습니다. 다른 Scene으로 전환하는 기능을 추가하면 재생 중인 Sound의 수명을 다시 검토해야 합니다. `platform/android/AndroidMain.cpp:28`, `platform/android/AndroidAudio.cpp:63`

EGL display/context의 소유자는 AndroidAudio가 아니라 GlContext입니다. 윈도우 종료에서는 surface를 해제하며 context는 shutdown까지 보존합니다. `platform/android/GlContext.cpp:73`

미검증 범위: 여러 기기에서 콜백 종료 순서, 오디오 라우트 변경, 장시간 프레임·배터리 동작을 계측하지 않았습니다. onData의 thread_local 버퍼는 크기가 증가할 때 메모리를 할당할 수 있으므로 콜백이 항상 무할당이라고 보장하지 않습니다. onError는 오류를 기록하며 자동 스트림 복구를 구현하지 않습니다. `platform/android/AndroidAudio.cpp:78`, `platform/android/AndroidAudio.cpp:89`

이 한계를 명시적으로 남기므로 needs-review 상태를 유지합니다. 모델 테스트와 빌드 통과가 동시성·기기 성능 검증을 대신하지 않습니다.
