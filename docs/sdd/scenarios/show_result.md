---
status: ok
section: scenarios
reviewer: Codex
reviewed: 2026-09-19
---

# 방어 결과와 보상

finishRaid는 성 체력이 남았는지로 승패를 정하고 결과 상태로 바꿉니다. 승패·날짜·처치 수에 따른 기본 보상과 창고·서고 보상을 지급하며 재화는 99999로 제한합니다. 10일 차 승리면 캠페인 완료를 기록합니다. `app/castle/CastleModel.cpp:179`

CastleScene이 상태 전환을 감지해 저장하고 결과 화면을 표시합니다. 이미 지급된 재화와 다음 준비 날짜를 함께 저장하므로 결과 화면에서 앱이 종료되어도 보상을 다시 지급하지 않습니다. `app/castle/CastleScene.cpp:46`, `app/castle/CastleModel.cpp:197`

continueBuilding은 결과 상태에서만 작동합니다. 승리했고 캠페인 완료가 아니면 날짜를 증가시키고, 패배면 같은 날에 재도전합니다. 성 체력을 초기화하여 준비 상태로 돌아갑니다. 마물 체력은 다음 startRaid에서 회복합니다. `app/castle/CastleModel.cpp:191`
