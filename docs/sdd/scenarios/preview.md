---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 명령 결과 예측

BattleModel::preview는 현재 모델을 값으로 복사하고 복사본의 execute를 호출합니다. 화면은 반환된 피해·기 변화·새 단서를 표시합니다. 실제 모델과 저장 파일은 변경하지 않습니다.

확정할 때 동일한 명령을 실제 모델에 실행하므로 별도 예측 공식을 유지하지 않습니다. 규칙 변경 시 예측과 실행 일치 테스트를 확인합니다. 근거: `app/srpg/BattleModel.cpp:70`, `app/srpg/BattleScene.cpp:195`.

[시나리오 목록](index.md)
