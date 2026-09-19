---
status: ok
section: scenarios
reviewer: Codex
reviewed: 2026-09-19
---

# 마물 합성과 도감 등록

CastleScene은 합성 상대와 결과 종, 평균 레벨, 마력 비용을 미리 보여 줍니다. 확인 버튼을 누르면 CastleModel::fuse를 호출합니다. `app/castle/CastleScene.cpp:281`, `app/castle/CastleScene.cpp:364`

1. 준비 상태인지 확인하고 서로 다른 유효한 인덱스의 조합을 검사합니다.
2. 슬라임+슬라임은 왕관 슬라임, 슬라임+임프는 달그림자, 임프+골렘은 새벽 드래곤이 됩니다.
3. 두 개체가 모두 Lv.2 이상이고 마력이 40 이상일 때만 진행합니다.
4. 첫 번째로 선택한 마물의 방과 두 레벨의 정수 평균을 결과에 유지합니다.
5. 낮은 인덱스에 결과를 기록하고 높은 인덱스의 재료를 제거해 배열을 당깁니다.
6. 개체 수를 하나 줄이고 마력 40을 차감하며 discovered 비트에 결과 종을 추가합니다. 이전 발견 기록은 지우지 않습니다.

근거: `app/castle/CastleModel.cpp:162`, `app/castle/CastleModel.cpp:171`. Monster는 헤더에 정의된 값 구조체이며 별도 Monster.cpp는 없습니다.

성공한 행동은 Scene에서 저장합니다. 합성 후 이전 버튼 입력 영역을 폐기하여 한 프레임 안의 다음 터치가 소모된 재료 인덱스를 재사용하지 않게 합니다. `app/castle/CastleScene.cpp:364`
