---
status: ok
section: feature-flags
reviewer: Codex
reviewed: 2026-09-19
---

# 컴파일 플래그 매트릭스

현재 기준은 Gradle/CMake가 만든 ARM64 Debug compile_commands.json입니다. NDK-build 프로젝트가 아닙니다. CMake는 C++20과 경고 옵션을 지정하며 현재 게임 기능을 켜고 끄는 별도 컴파일 플래그는 없습니다. 근거: `CMakeLists.txt:4`, `CMakeLists.txt:42`.

| 추출된 define | 값 | 의미 |
|---|---|---|
| ANDROID | 1 | Android 타깃 빌드입니다. |
| _FORTIFY_SOURCE | 2 | 툴체인이 지정한 강화 옵션입니다. |
| minigame_EXPORTS | 1 | minigame 공유 라이브러리 타깃의 export define입니다. |

소환·합성·전투 수치는 CastleModel.h의 constexpr 자료와 CastleModel.cpp의 규칙으로 관리합니다. ABI는 app/build.gradle.kts의 arm64-v8a와 x86_64입니다. 이 표의 define은 추출 대상 코드에서 별도 기능 분기를 만들지 않습니다.
