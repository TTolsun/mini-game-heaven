---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-19
---

# 글꼴과 수련 상태 불러오기

BattleScene::onEnter는 srpg-glyphs.txt와 NotoSansKR.ttf로 글리프를 아틀라스에 굽고 Sfx를 합성합니다. 내부 파일 경로의 wuten-v1.txt를 읽은 뒤 이동 범위와 선택 행동 예측을 갱신합니다.

저장을 읽지 못하면 초기 모델을 유지합니다. load는 후보를 검증한 뒤 반영하므로 부분적으로 읽힌 상태가 게임에 들어오지 않습니다. 근거: `app/srpg/BattleScene.cpp:27`, `app/srpg/BattleModel.cpp:246`.

[시나리오 목록](index.md)
