---
generated_at: 2026-09-17T14:41:41+00:00
source_commit: b4e016048393b0285580f4897dc534a99b59117d
agent: ollama/qwen3.5:4b
status: ok
section: feature-flags
---

# 컴파일 플래그 매트릭스

**바꾸려는 동작이 어떤 -D 플래그나**



## 플래그 표

이 표는 compile DB(NDK-build 구성)의 `-D` 목록과 소스의 `#if` 분기 위치를 그대로 옮긴 것입니다. LLM 을 거치지 않았습니다.

| 플래그 | 값 | 분기 위치 (최대 20) |
|---|---|---|
| `ANDROID` | `1` | (소스에서 분기 없음) |
| `_FORTIFY_SOURCE` | `2` | (소스에서 분기 없음) |
| `minigame_EXPORTS` | `1` | (소스에서 분기 없음) |

??? note "근거와 검토 정보"
    - 근거 파일: (없음)
    - 근거 수준: 코드 확인 (정적 분석, simple_compdb 구성, commit `b4e0160483`)
    - 인용 검증: 통과
    - 검토: 2026-09-17 · ollama/qwen3.5:4b · 사람 검토 전

다음 단계: [스레드와 콜백 모델](threading.md)
