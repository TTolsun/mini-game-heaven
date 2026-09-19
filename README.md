# DRAGON BALL: 武天 (가제)

C++20 Android 무술 SRPG입니다. 무천도사의 어린 시절부터 손오공과의 만남까지를 그리며, 현재 0.6.0은 첫 장의 다섯 맵과 도장 테마의 그래픽 시제품을 제공합니다.

**스토리 → 도장 정비 → 출전 준비 → 전투 → 전리품·성장·관계 → 스토리**를 반복합니다. 전투의 중심은 **위치 조정 → 밀쳐내기 → 동료 추격 → 기 활용**입니다. 간파·모방·체득은 전투에서 단서를 발견하고 거점에서 연구하는 기술 성장으로 연결합니다.

## 플레이

1. 이야기에서 동행·질문·개인 수련을 선택해 관계, 보급, 초기 기 보너스를 받습니다.
2. 도장에서 수련 방향, 기술 연구, 장비, 동료 정보와 약초 준비를 확인합니다.
3. 로시를 포함한 2~3명을 출전시키고 시작 칸을 눌러 배치를 바꿉니다.
4. 인물 → 빈 이동 칸 → 행동 → 대상 칸 → 결과 예측 → 확정 순서로 조작합니다. 아군은 각자 한 번 행동하고 마지막 아군 행동 뒤 적 차례가 진행됩니다.
5. 발경으로 적을 동료 옆에 밀면 추격이 발생합니다. 벽이나 다른 적과 충돌하면 추가 피해를 줍니다. 추격은 아군마다 한 라운드에 한 번이며 재귀 연계하지 않습니다.
6. 전투에서 관찰·적 사거리 이탈·방어 피격으로 단서를 얻습니다. 로시가 세 단서와 발경 2회 사용을 채우면 다음 도장에서 붕권을 연구할 수 있습니다.
7. 승리 보상을 한 번 수령하면 경험·관계·연구 조건이 다음 이야기로 이어집니다. 패배 후에는 출전 당시의 배움·약초로 재도전하거나 도장에 돌아갑니다.

일반 타격은 기 +10, 적의 현재 사거리에서 벗어나는 이동은 +15, 피격은 +5입니다. 기 모음은 제자리 연속 20→30→50이며 모으다 맞으면 추가 피해 6과 기 손실 10을 받습니다. 기술과 추격 타격은 공격 성공 기를 추가로 지급하지 않습니다.

## 현재 콘텐츠

| 항목 | 내용 |
|---|---|
| 동료 3명 | 로시: 밀쳐내기·반격, 학선인: 기동·추격, 란란: 기공·회복입니다. 란란은 창작 인물입니다. |
| 적 4종 | 권사·철벽병·기공사·돌격수입니다. |
| 맵 5개 | 산길의 세 사람, 도장의 입문 시험, 계곡의 빛, 엇갈리는 발걸음, 함께 여는 길입니다. |
| 기술 10개 | 발경·붕권·회선각·연타·비연각·점혈·기탄·기공파·응급처치·쌍룡격입니다. |

1280×720 가로 화면, 48px 타일, 48×64 모듈형 유닛, 큰 인물 초상화와 순차 전투 연출을 사용합니다. 여덟 외형이 네 방향·여섯 동작을 공유합니다. 도장·숲·강·다리는 하나의 공통 테마이며 테두리 안 7×7칸만 전장입니다. 연출 절제 설정으로 흔들림·섬광·컷인을 줄일 수 있습니다. 절벽 추락·지형 파괴·장비 상점·분기 엔딩·전체 성장기 캠페인은 구현하지 않았습니다.

## 문서와 구조

- [게임 방향](docs/game-direction.md), [설계](docs/srpg-design.md), [설정 근거](docs/srpg-lore.md)를 관리합니다.
- [밸런스 검토](design/balance/balance-check-campaign-2026-09-20.md), [검증 기록](docs/validation.md), [개발 정책](docs/development-policy.md)을 확인할 수 있습니다.
- [그래픽 기준](docs/art/style-guide.md)과 [이미지 프롬프트·원본 기록](docs/art/visual-slice-prompts.md)을 관리합니다.
- BattleData는 콘텐츠와 수치, BattleModel은 캠페인·전투·저장, BattleScene은 화면·입력, BattleArt는 모듈형 아트, BattleTimeline은 순차 표현을 담당합니다. 모델은 Android/GLES에 의존하지 않습니다.
- `wuten-v2.txt`에 확정 상태를 저장합니다. 이전 수련장 저장 파일은 덮어쓰거나 자동 변환하지 않습니다.

프로젝트·라이브러리는 wuten, namespace는 com.ttolsun.wuten입니다. 기존 설치의 업데이트를 위해 applicationId com.ttolsun.minigameheaven을 유지합니다.

## 빌드와 테스트

JDK 17, Android SDK 36, NDK 29.0.14206865, CMake 3.31.6을 사용합니다.

```powershell
.\gradlew.bat assembleDebug assembleRelease testDebugUnitTest lintDebug --no-daemon
.\tools\test-srpg.ps1 -Device <serial> -Abi arm64-v8a
android run --apks=app/build/outputs/apk/debug/app-debug.apk --device=<serial>
```

```bash
mkdir -p build
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 -I native native/tests/BattleModelTests.cpp native/app/srpg/BattleModel.cpp -o build/srpg-model-tests
./build/srpg-model-tests build
```

한국어 문구를 바꾸면 `uv run --with fonttools python tools/prepare-srpg-font.py <NotoSansKR-variable.ttf>`로 글꼴을 갱신합니다. 서명된 기존 설치에는 동일한 키의 Release APK로 업데이트합니다.
