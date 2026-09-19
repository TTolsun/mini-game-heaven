# 무천 SRPG SDD

C++ 코드에서 추출한 사실과 코드 대조로 검토한 설명을 담습니다. 게임 목표와 현재 구현은 [게임 방향](../game-direction.md), 실제 실행 결과는 [검증 기록](../validation.md)에서 확인합니다.

| 내용 | 문서 |
|---|---|
| 패키지와 진입점 | [시스템 개요](overview.md) |
| 상태·화면·엔진의 책임 | [컴포넌트 구조](components.md) |
| 명령·예측·저장·입력 흐름 | [핵심 시나리오](scenarios/index.md) |
| 빌드 플래그 | [컴파일 플래그](feature-flags.md) |
| 동시성과 남은 한계 | [스레드 모델](threading.md) |
| 변경 제약 | [제약](constraints.md) |
| 설계 이유 | [결정](decisions.md) |

`status: ok`는 코드 대조 상태이며 기기 성능 보장이 아닙니다. `needs-review`는 명시한 미검증 한계를 유지한다는 뜻입니다. 내용 해시와 검토자는 `docs/sdd-review.json`에 기록합니다. 동기화 도구가 만든 설명도 코드와 대조한 뒤 승인합니다.
