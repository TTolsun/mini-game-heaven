---
status: ok
section: scenarios
reviewer: Codex
reviewed: 2026-09-19
---

# 한국어 글꼴과 저장된 성 불러오기

onEnter는 글리프 목록, Noto Sans KR 폰트, 몬스터 이미지 시트, 합성 효과음, 저장 파일을 순서대로 준비합니다. 몬스터 시트는 아틀라스에 한 번 로드한 뒤 3×2 칸을 종별 Sprite로 나눕니다. 저장 경로는 Engine의 내부 데이터 경로 아래 castle-v1.txt입니다. `app/castle/CastleScene.cpp:29`, `app/castle/MonsterArt.h:12`

Font::load는 ASCII와 지정된 UTF-8 코드포인트를 정렬·중복 제거하고 stb_truetype으로 베이크합니다. 실제 사용한 행만 RGBA 이미지로 변환해 아틀라스에 추가합니다. 파일 읽기나 패킹이 실패하면 false를 반환하며 현재 Scene에는 별도 대체 글꼴이 없습니다. `engine/graphics/Font.cpp:37`

load는 임시 모델에 파일을 읽으며 버전·재화·날짜·종·레벨·방 범위와 중복 배치를 검증합니다. 방은 6개 이상 순서대로 열려 있어야 합니다. 검증 성공 시에만 현재 모델을 교체하고 준비 상태로 시작합니다. 저장이 없거나 잘못되었으면 초기 모델을 유지합니다. `app/castle/CastleModel.cpp:209`

save는 임시 파일을 flush·close한 뒤 Android/POSIX rename으로 기존 파일을 교체합니다. 파일 쓰기·교체 실패는 Scene 알림으로 표시합니다. 전원 장애에 대한 fsync 보장은 없으며 Windows 호스트의 기존 파일 교체 동작은 지원 대상으로 검증하지 않았습니다. `app/castle/CastleModel.cpp:197`, `app/castle/CastleScene.cpp:46`
