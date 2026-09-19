---
status: ok
section: overview
reviewer: Codex
reviewed: 2026-09-19
---

# 시스템 개요

마물 정원은 C++20으로 구현한 Android 몬스터 수집·성장·합성 게임입니다. 소환과 먹이, 합성으로 만든 마물이 자동 방어를 수행하고 보상으로 다음 성장을 준비합니다.

| 영역 | 책임 | 근거 |
|---|---|---|
| app/castle | 게임 상태, 경제, 합성, 전투, 저장과 게임 화면 | `app/castle/CastleModel.cpp:9`, `app/castle/CastleScene.cpp:35` |
| engine | Scene 구동, 좌표 변환, 그래픽, 글꼴, 오디오 믹서 | `engine/Engine.cpp:77` |
| platform/android | GameActivity 이벤트, EGL, AAudio, 자산 접근, JNI 햅틱 | `platform/android/AndroidMain.cpp:54` |

`android_main()`이 AppState와 이벤트 루프를 구성합니다. 최초 윈도우 생성 시 GL과 Engine을 초기화하고 CastleScene을 설치합니다. 입력은 Engine을 거쳐 CastleScene으로 전달되며, 포커스와 surface가 있을 때 프레임을 그립니다. `platform/android/AndroidMain.cpp:156`

CastleModel은 표준 C++만 사용합니다. CastleScene은 Engine 서비스를 사용하고 Engine은 Scene 인터페이스를 통해 앱을 호출합니다. 이전 미니게임 소스는 보존하지만 현재 CMake 대상에서 제외했습니다. `CMakeLists.txt:11`

읽는 순서는 AndroidMain → Engine::frame → CastleScene → CastleModel입니다. [시나리오 목록](scenarios/index.md)에서 각 흐름을 확인할 수 있습니다.
