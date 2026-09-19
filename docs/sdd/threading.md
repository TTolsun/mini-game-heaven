---
status: needs-review
section: threading
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 스레드와 콜백 모델

| 실행 흐름 | 작업 | 근거 |
|---|---|---|
| android_main 루프 | 입력, 전투 명령, 화면 갱신, 파일 저장을 처리합니다. | `platform/android/AndroidMain.cpp:156` |
| AAudio 데이터 콜백 | Mixer::render로 PCM을 혼합합니다. | `platform/android/AndroidAudio.cpp:72` |
| JNI 햅틱 호출 | 게임 스레드에서 JVM 연결과 진동 요청을 관리합니다. | `platform/android/AndroidHaptics.cpp:102` |

BattleModel에는 비동기 작업이나 별도 스레드가 없습니다. 명령·미리보기·저장은 호출한 게임 스레드에서 동기적으로 완료됩니다.

Mixer::play는 빈 보이스의 필드를 채운 뒤 release 저장으로 Sound 포인터를 공개합니다. 콜백은 acquire 읽기로 이를 확인하고 재생 종료 때 nullptr를 release 저장합니다. 게임 스레드는 재생 중인 보이스를 덮어쓰지 않습니다. 근거: `engine/audio/Mixer.cpp`.

BattleScene이 소유한 Sfx는 장면이 살아 있는 동안 유지됩니다. AppState 멤버는 역순으로 파괴되어 AndroidAudio가 Engine보다 먼저 닫힙니다. 다른 장면으로 전환하는 기능을 추가할 때 재생 중인 Sound 수명을 다시 검토해야 합니다. 근거: `app/srpg/BattleScene.h`, `platform/android/AndroidMain.cpp:28`.

정적 분석만으로 여러 기기의 콜백 종료 순서, 오디오 라우트 변경, 장시간 프레임·배터리 동작을 보장할 수 없습니다. onData 버퍼는 용량 증가 때 할당할 수 있고 오류 콜백은 자동 스트림 복구를 구현하지 않습니다. 이 한계를 남기므로 needs-review를 유지합니다. 근거: `platform/android/AndroidAudio.cpp:78`, `platform/android/AndroidAudio.cpp:89`.
