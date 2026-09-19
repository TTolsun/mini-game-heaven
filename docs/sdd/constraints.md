---
status: ok
kind: manual
evidence_files:
  - app/srpg/BattleModel.cpp
  - app/srpg/BattleScene.cpp
  - engine/Engine.h
  - platform/android/AndroidMain.cpp
---

# 변경 시 지켜야 할 제약

## 전투 모델과 표현을 분리한다

BattleModel은 Android·GLES·Scene을 참조하지 않습니다. 명령 하나가 아군 행동과 상대 응답을 원자적으로 처리하고, preview는 모델 복사본에 동일한 execute를 실행합니다. 미리보기에서 실제 상태를 바꾸거나 애니메이션 프레임에 따라 피해를 계산하지 않습니다. 근거: `native/app/srpg/BattleModel.cpp`.

## 입력 대상은 상태 변경 즉시 폐기한다

BattleScene은 메뉴·선택·명령·BACK 처리 때 기존 hit 목록과 포인터 상태를 폐기합니다. 한 Android 입력 버퍼에 여러 탭이 와도 이전 프레임의 확정 버튼으로 명령을 반복 실행하지 못하게 합니다. 근거: `native/app/srpg/BattleScene.cpp`.

## 기력과 배움은 행동 결과다

같은 종류의 단서는 중복 성장하지 않습니다. 체득 이후 성공 횟수는 상한을 유지합니다. 기 축적은 이동과 피격으로 연속 효과가 끊기며, 충돌·밀쳐내기로 중단된 적의 기술은 관찰할 수 없습니다. 근거: `native/app/srpg/BattleModel.cpp`, `native/app/srpg/BattleData.h`.

## 확정된 상태만 저장한다

`wuten-v1.txt`에 위치·턴·체력·기·단서·성공 횟수·입장 상태·승패를 저장합니다. 임시 파일을 닫은 뒤 Android의 rename으로 교체합니다. 파싱과 범위 검증을 통과한 후보만 반영하며 실패 시 기존 상태를 유지합니다. 재도전은 수련 입장 상태로 돌아갑니다. 근거: `native/app/srpg/BattleModel.cpp`.

## 엔진의 의존 방향과 서비스 수명

engine은 app과 platform을 include하지 않습니다. Scene이 소유한 효과음은 재생 중인 보이스보다 오래 살아야 합니다. AndroidAudio는 Engine보다 먼저 파괴됩니다. 장면 전환을 추가할 때 재생 중인 Sound 수명을 다시 검토합니다. 근거: `native/engine/audio/Mixer.cpp`, `native/platform/android/AndroidMain.cpp`.

## 표시와 검증

화면은 safeTop·safeBottom을 피하고, 한국어 글리프는 소스에서 재생성합니다. 색상에 더해 공격선의 느낌표·방향 점·문구를 표시합니다. 신규 명령은 모델 예측 일치·잘못된 입력 거부·저장 복원·실제 Scene 입력 테스트를 함께 확인합니다.

코드 변경 뒤 Debug·Release 빌드, lint, 모델·Scene 테스트, 문서 동기화·검토·검사를 수행합니다. 모델 성능 수치는 전체 프레임·배터리 성능이 아닙니다. 실제 검증 범위는 [검증 기록](../validation.md)을 따릅니다.
