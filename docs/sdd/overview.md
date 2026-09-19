---
status: ok
section: overview
reviewer: Codex
reviewed: 2026-09-19
---

# 시스템 개요

마물 정원은 카이로소프트 「마왕성 스토리」의 성 운영 경험을 기준으로 개발하는 C++20 Android 던전 경영 게임입니다. 현재는 방 확장·통로 편집·왕좌 이동, 소환·먹이·합성과 자동 방어를 구현했습니다. 용사는 편집한 성의 최단 경로를 이동하며, 배치된 마물과 함정이 그 경로에서 교전합니다. 탐험·성 등급·진화는 후속 구현 범위입니다.

| 영역 | 책임 | 근거 |
|---|---|---|
| app/castle | 게임 상태, 경제, 합성, 전투, 저장과 게임 화면 | `app/castle/CastleModel.cpp:9`, `app/castle/CastleScene.cpp:29` |
| engine | Scene 구동, 좌표 변환, 그래픽, 글꼴, 오디오 믹서 | `engine/Engine.cpp:77` |
| platform/android | GameActivity 이벤트, EGL, AAudio, 자산 접근, JNI 햅틱 | `platform/android/AndroidMain.cpp:54` |

`android_main()`이 AppState와 이벤트 루프를 구성합니다. 최초 윈도우 생성 시 GL과 Engine을 초기화하고 CastleScene을 설치합니다. 입력은 Engine을 거쳐 CastleScene으로 전달되며, 포커스와 surface가 있을 때 프레임을 그립니다. `platform/android/AndroidMain.cpp:156`

CastleModel은 표준 C++만 사용합니다. CastleScene은 Engine 서비스를 사용하고 Engine은 Scene 인터페이스를 통해 앱을 호출합니다. 이전 미니게임 소스는 보존하지만 현재 CMake 대상에서 제외했습니다. `CMakeLists.txt:11`

읽는 순서는 AndroidMain → Engine::frame → CastleScene → CastleModel입니다. [시나리오 목록](scenarios/index.md)에서 각 흐름을 확인할 수 있습니다.
