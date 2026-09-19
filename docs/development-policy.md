# 문서 검토와 릴리스 정책

코드 변경 PR 안에서 문서를 동기화하고 검토 기록까지 갱신합니다. `docs-sync`는 누락을 보완하는 자동 PR을 만들 뿐, 검토를 승인하지 않습니다.

## 로컬 검사

Node.js 24, uv, JDK 17, Android SDK와 기존 NDK/CMake가 필요합니다.

```bash
bash tools/setup-sdd.sh
./gradlew assembleDebug testDebugUnitTest lintDebug assembleRelease
bash tools/docs-sync.sh --force
```

`tools/sdd-tool.json`은 엔진 커밋을 고정합니다. `.tool/camera-hal-sdd`의 커밋이 다르거나 수정돼 있으면 검사가 실패합니다. 엔진 버전을 올릴 때에는 잠금 파일과 문서 검토 기록을 같은 PR에서 갱신합니다.

생성된 설명을 코드와 대조한 후 다음과 같이 검토를 기록합니다. 이름에는 실제 검토자를 적고, 이유에는 검토 범위를 적습니다. 명령을 실행하는 것은 해당 내용을 검토했다는 선언이며, 검토자의 신원을 인증하거나 문장 정확성을 증명하는 기능은 아닙니다.

```bash
node tools/docs-review.mjs accept --reviewer "검토자 이름" --reason "변경된 호출 흐름과 문서의 근거를 대조했습니다"
bash tools/docs-check.sh
```

`needs-review` 문서는 기본적으로 승인을 막습니다. 정적 분석으로 확인할 수 없는 한계를 문서에 명시한 상태로 유지하려면, 검토한 페이지마다 이유를 추가합니다. 자동으로 `ok`로 바꾸지 않습니다.

```bash
node tools/docs-review.mjs accept --reviewer "검토자 이름" --reason "코드 및 문서 대조" \
  --acknowledge "docs/sdd/threading.md=콜백 타이밍은 기기 검증 전이며, 문서가 이를 미확인으로 명시하는지 검토했습니다"
```

`docs/sdd-review.json`에는 코드·빌드 설정·앱 자산·문서 설정·엔진 버전과 전체 SDD의 내용 해시를 저장합니다. 작업 트리의 신규 파일과 삭제도 검사합니다. 초기 구현은 누락을 막기 위해 전체 검토 범위를 보수적으로 무효화합니다. 줄바꿈, 추출 시각, 실행 머신 경로와 소스 커밋 라벨만 바뀐 경우는 무시합니다. 기록은 CI에서 만들 수 없습니다.

기존 `status: ok`만으로는 검토자와 대상 소스를 알 수 없습니다. 검토 기록이 없거나 오래되면 `docs-check`가 실패합니다. 생성 문서와 `docs/index.html`, 검토 기록을 함께 커밋합니다. 워크플로, 정책 도구, 이 정책 문서와 릴리스 노트의 변경도 검토를 무효화합니다. 사용자가 위임한 코드·문서 검토를 AI가 수행했다면 검토자를 `Codex (AI)`처럼 명시하고, 기기 검증을 수행한 것으로 기록하지 않습니다.

## 병합 조건

모든 PR에서 `docs-check`의 `check` 작업을 실행합니다. 이 작업은 정책 회귀 테스트, Debug·Release 빌드, 가능한 JVM 단위 테스트, lint, 사실·검토 기록·HTML 일치를 검사합니다. 현재 네이티브 게임의 기기 동작 검증을 JVM 작업이 대신하지는 않습니다.

GitHub main 보호 규칙에 `check`를 필수 상태 검사로 지정하고, 최신 main 반영과 PR 경유를 요구합니다. 경로 필터를 두지 않아 문서와 무관한 PR에서도 필수 검사가 누락되지 않습니다. 이 설정은 워크플로 파일과 별개인 GitHub 저장소 설정입니다.

## APK 릴리스

1. `app/build.gradle.kts`에서 `versionName`과 `versionCode`를 갱신합니다. versionCode는 이전 배포보다 커야 합니다.
2. `docs/releases/<version>.md`에 변경 사항, 지원 범위, 실제 실행한 빌드·테스트, 기기·Android 버전·검증 시나리오, 알려진 한계를 작성합니다. 실행하지 않은 테스트를 통과했다고 기록하지 않습니다.
3. 문서 검토 기록을 갱신하고 필수 검사를 통과한 PR을 main에 병합합니다.
4. 해당 커밋에 `v<version>` 태그를 푸시합니다. 릴리스 워크플로가 문서·빌드 검사를 다시 실행합니다.
5. `release` 환경에서 서명 APK를 만들고 패키지명·버전·debuggable 여부·예상 인증서를 검사합니다. APK, `SHA256SUMS.txt`, `verification.json`을 GitHub Release **초안**에 첨부합니다.
6. APK의 설치·업데이트와 게임 동작을 기기에서 확인한 뒤 초안을 공개합니다. 이번 설정 작업은 태그나 공개 릴리스를 생성하지 않습니다.

`release` 환경에 다음 값을 등록해야 합니다. 기존 키를 유지하며 키 파일이나 비밀번호를 커밋하지 않습니다.

| 종류 | 이름 |
|---|---|
| Secret | `RELEASE_KEYSTORE_BASE64` |
| Secret | `RELEASE_STORE_PASSWORD` |
| Secret | `RELEASE_KEY_ALIAS` |
| Secret | `RELEASE_KEY_PASSWORD` |
| Variable | `RELEASE_CERT_SHA256` (예상 인증서 SHA-256) |

환경에는 릴리스 태그 제한과 필요한 검토자를 설정할 수 있습니다. 필수 값이 없으면 서명 단계는 실패하며 debug 키로 대체하지 않습니다. 로컬 빌드는 루트의 `keystore.properties`에서 `storeFile`, `storePassword`, `keyAlias`, `keyPassword`를 읽거나 같은 역할의 `RELEASE_*` 환경 변수를 사용합니다. 아무 키도 없으면 미서명 Release 빌드는 허용하지만 배포 패키징은 실패합니다.
