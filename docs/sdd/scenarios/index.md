---
status: ok
section: scenarios
reviewer: Codex
reviewed: 2026-09-19
---

# 핵심 시나리오

| 흐름 | 문서 |
|---|---|
| 플랫폼 진입과 생명주기 | [앱 시작](android_main.md) |
| 글꼴과 저장 복원 | [자산 로딩](load_assets.md) |
| 터치·BACK·연속 입력 | [터치 전달](touch.md) |
| 합성 조건·재료 소모·도감 | [합성](fusion.md) |
| 준비 상태에서 전투로 전환 | [방어 시작](start_game.md) |
| 업데이트와 화면 출력 | [한 프레임](frame.md) |
| 자동 전투·승패 판단 | [전투](dodge_update.md) |
| 보상·날짜·재시작 | [결과](show_result.md) |

호출 그래프는 정적 분석 결과이며 분기 조건과 소유권 설명은 해당 C++ 본문과 대조했습니다. 정적 호출 개수는 게임 로직의 검증 범위를 뜻하지 않습니다.
