#!/usr/bin/env bash
# 설계 문서(docs/sdd) 동기화. 코드에서 사실을 다시 뽑고, 영향받은 절을 로컬 LLM 으로 다시 쓰고,
# facts.json 과 단일 HTML(docs/index.html) 을 갱신한다. 결과는 사람이 검토한 뒤 커밋한다.
#
# 사용법: tools/docs-sync.sh [--base <ref> | --force] [--site]
#   --base   이 커밋 이후 변경이 영향을 주는 절만 다시 쓴다.
#            기본은 docs/sdd/facts.json 에 기록된 마지막 동기화 커밋.
#   --force  전체를 다시 쓴다.
#   --site   LLM 을 부르지 않고 facts.json 과 docs/index.html 만 다시 만든다.
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/docs-lib.sh"

BASE=""
FORCE=0
SITE_ONLY=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --base) BASE="$2"; shift 2 ;;
        --force) FORCE=1; shift ;;
        --site) SITE_ONLY=1; shift ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done

require_tool
cd "$ROOT"
if [[ $SITE_ONLY == 0 && $FORCE == 0 ]] && ! git diff --quiet HEAD -- native app/src/main app/build.gradle.kts docs/sdd-config sdd.yaml; then
    echo 'Uncommitted inputs: use --force to include working-tree changes.' >&2
    exit 1
fi
if [[ $SITE_ONLY == 0 && $FORCE == 0 && -n "$(git ls-files --others --exclude-standard -- native app/src/main)" ]]; then
    echo 'Untracked inputs: use --force to include new files.' >&2
    exit 1
fi
bash tools/sync-compile-commands.sh

if [[ $SITE_ONLY == 1 ]]; then
    sdd extract
else
    if [[ $FORCE == 0 && -z "$BASE" && -f "$FACTS_DOCS" ]]; then
        BASE="$(node -e 'console.log(require(process.argv[1]).meta.source_commit || "")' "$FACTS_DOCS")"
    fi
    if [[ $FORCE == 1 || -z "$BASE" ]]; then
        echo "== 전체 생성"
        sdd run
    elif [[ "$BASE" == "$(git rev-parse HEAD)" ]]; then
        echo "== 마지막 동기화 이후 변경 없음 (${BASE:0:10}); 사실만 다시 뽑습니다"
        sdd extract
    else
        echo "== ${BASE:0:10}..HEAD 영향 절만 생성"
        sdd run --base "$BASE"
    fi
fi

mkdir -p "$(dirname "$FACTS_DOCS")" "$(dirname "$SITE_DOCS")"
cp "$FACTS_BUILD" "$FACTS_DOCS"
sdd export-html --title "$SITE_TITLE" --out "$SITE_DOCS"

echo
echo "동기화 완료. 검토할 것:"
grep -l "^status: needs-review" docs/sdd/*.md docs/sdd/scenarios/*.md 2>/dev/null | sed 's/^/  needs-review: /' || true
if [[ -f "$ROOT/build/change_impact.md" ]]; then
    echo "  변경 요약: build/change_impact.md"
fi
echo "검토 후 node tools/docs-review.mjs accept --reviewer NAME --reason TEXT 로 기록하세요."
echo "동기화는 검토 기록을 자동 승인하지 않습니다."
