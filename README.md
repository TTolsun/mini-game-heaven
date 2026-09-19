# DRAGON BALL: 武天 (가제)

C++20 기반 Android 무술 SRPG입니다. 무천도사의 어린 시절부터 손오공과의 만남까지를 그리며, 핵심은 **전투에서 상대를 간파하고 기술을 모방한 뒤 자기 무술로 체득하는 과정**입니다.

현재 0.4.0 개발 버전은 이 재미를 확인하는 **세 개의 단독 수련전**입니다. 전체 이야기, 다섯 맵의 1장, 동료 협공과 행동별 성장 계통은 후속 범위입니다.

## 실행되는 플레이

1. 파란 칸을 눌러 이동할 위치를 고르고 관찰·방어·타격·기 모으기·기술 중 하나를 선택합니다.
2. 예상 피해와 기력 변화, 새 단서 여부를 확인한 뒤 `이동 + 행동 확정`을 누릅니다.
3. 상대 3칸 안에서 공격을 관찰하고, 예고된 빨간 공격선을 벗어나고, 공격선 안에서 방어하면 서로 다른 단서를 얻습니다.
4. 세 단서를 모두 얻으면 철산격 모방이 열립니다. 기 20을 쓰고, 상대 공격에 노출된 채 사용하면 추가 피해를 받습니다.
5. 살아 있는 상대에게 모방을 두 번 성공시키면 로시류 붕권을 체득합니다. 기 비용이 15로 줄고 피해·밀쳐내기가 강화됩니다.
6. 상대를 쓰러뜨리면 다음 수련으로 진행합니다. 단서와 체득은 이어지고 체력·기는 초기화됩니다.

기 모으기는 같은 자리에서 연속 사용하면 20 → 30 → 50을 얻습니다. 상한은 100입니다. 모으는 중 공격받으면 추가 피해와 기 손실이 있고 연속 축적이 끊깁니다. 같은 단서는 다시 얻지 못하며 재도전하면 해당 수련에 입장했던 배움으로 돌아갑니다.

첫 번째·두 번째·세 번째 수련은 각각 열린 도장, 기둥이 있는 도장, 벽 충돌을 활용하는 도장입니다. 상대는 다음 공격 방향을 보여 줍니다. 체득한 붕권으로 밀거나 벽에 부딪치게 하면 예고된 공격을 끊을 수 있습니다.

현재 화면은 세로형이며 임시 도형 캐릭터를 사용합니다. 한국어 안내와 색·기호를 함께 표시합니다. `수련 안내`에서 단서 조건과 조작을 다시 볼 수 있습니다.

## 문서

- [게임 방향](docs/game-direction.md): 주제, 전체 이야기와 재미의 네 축입니다.
- [SRPG 설계](docs/srpg-design.md): 장 구성, 전투 중 성장, 첫 시제품과 후속 범위입니다.
- [설정 근거](docs/srpg-lore.md): 원작·애니메이션·2차 자료·게임 창작을 구분합니다.
- [시제품 밸런스 검토](design/balance/balance-check-srpg-prototype-2026-09-19.md): 수치, 검증 전략과 남은 문제입니다.
- [검증 기록](docs/validation.md), [코드 구조](docs/sdd/index.md), [개발 정책](docs/development-policy.md)을 함께 관리합니다.

## C++ 구조와 저장

- `native/app/srpg/BattleData.h`: 세 수련의 맵·수치·행동 정의입니다.
- `BattleModel.*`: 격자 이동, 상대 응답, 단서·모방·체득, 승패와 저장을 처리합니다. Android·그래픽 의존성이 없습니다.
- `BattleScene.*`: 지도, 예측, 행동 확인, 결과, 한국어 안내와 입력을 담당합니다.
- `native/engine/`: OpenGL ES 3 렌더링·글꼴·입력·음향 기반입니다.
- `native/platform/android/`: GameActivity·AAudio·햅틱·EGL 연결입니다.

`wuten-v1.txt`에 확정된 턴을 저장합니다. 이동 미리보기는 저장하지 않고, 임시 파일 교체로 완료한 상태만 반영합니다. 잘못된 위치·중복 점유·불완전한 저장을 거부합니다.

Gradle 프로젝트와 네이티브 라이브러리는 `wuten`, Android namespace는 `com.ttolsun.wuten`입니다. 기존 설치에 업데이트할 수 있도록 배포 식별자 `com.ttolsun.minigameheaven`는 유지합니다. 저장소의 원격 주소는 이번 코드 개편에서 변경하지 않습니다.

## 빌드와 테스트

JDK 17, Android SDK platform 36, NDK 29.0.14206865, CMake 3.31.6을 사용합니다.

```powershell
.\gradlew.bat assembleDebug assembleRelease testDebugUnitTest lintDebug --no-daemon
.\tools\test-srpg.ps1 -Device <serial> -Abi x86_64
android run --apks=app/build/outputs/apk/debug/app-debug.apk --device=<serial>
```

순수 모델은 Linux에서도 실행할 수 있으며 CI의 필수 검사에 포함합니다.

```bash
mkdir -p build
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 -I native native/tests/BattleModelTests.cpp native/app/srpg/BattleModel.cpp -o build/srpg-model-tests
./build/srpg-model-tests build
```

한국어 문구를 변경하면 Noto Sans KR 원본 가변 TTF로 글꼴을 갱신합니다.

```powershell
uv run --with fonttools python tools/prepare-srpg-font.py <NotoSansKR-variable.ttf>
```

문서 동기화·검토와 릴리스 절차는 [개발 정책](docs/development-policy.md)을 따릅니다. 빌드 성공을 실제 재미·장시간 성능 검증으로 해석하지 않습니다.
