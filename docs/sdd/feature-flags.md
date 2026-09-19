---
status: ok
section: feature-flags
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 컴파일 플래그

arm64-v8a Debug CMake compile database에서 확인한 정의입니다.

| 플래그 | 값 | 의미 |
|---|---|---|
| ANDROID | 1 | Android 대상 빌드입니다. |
| _FORTIFY_SOURCE | 2 | 도구 체인의 보강 정의입니다. |
| wuten_EXPORTS | 1 | wuten 공유 라이브러리 대상입니다. |

소스에 이 정의를 조건으로 하는 별도의 게임 규칙 분기는 없습니다. `native/CMakeLists.txt`는 C++20과 경고 옵션을 사용하며 ARM64·x86_64 ABI는 앱 Gradle 설정에 명시합니다.
