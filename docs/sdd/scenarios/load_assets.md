---
status: ok
section: scenarios
reviewer: Codex (AI)
reviewed: 2026-09-20
---

# 한국어 글꼴과 캠페인 불러오기

BattleScene::onEnter는 srpg-glyphs.txt와 NotoSansKR.ttf를 아틀라스에 굽고 Sfx를 합성합니다. 내부 데이터 경로의 wuten-v2.txt를 불러와 단계·선택 가능 인물·이동 범위·미리보기를 갱신합니다. 저장이 없거나 잘못되면 첫 Story로 시작합니다.

근거: app/srpg/BattleModel.cpp, app/srpg/BattleScene.cpp, engine/Engine.cpp, platform/android/AndroidMain.cpp.

[시나리오 목록](index.md)
