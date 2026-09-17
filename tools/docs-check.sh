#!/usr/bin/env bash
# 설계 문서(docs/sdd) 검사. CI 의 docs-check 워크플로와 같은 단계를 같은 순서로 실행한다.
#
#   사실 최신성: 코드에서 다시 뽑은 facts 가 커밋된 docs/sdd/facts.json 과 같은가
#   문서 최신성: base..HEAD 변경이 영향을 주는 절의 문서가 함께 바뀌었는가 (sdd impact)
#   검토 상태:   status: needs-review 인 문서 목록 (경고)
#   사이트 일치: docs/sdd 에서 만든 단일 HTML 이 커밋된 docs/index.html 과 같은가
#
# 사용법: tools/docs-check.sh [--base <ref>] [--ci]
#   --base  영향 계산의 기준 커밋. 기본은 origin/main 과의 merge-base.
#   --ci    GitHub Actions 주석 형식으로 출력한다.
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/docs-lib.sh"

BASE=""
CI=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --base) BASE="$2"; shift 2 ;;
        --ci) CI=1; shift ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done

fail=0
problem() {
    fail=1
    if [[ $CI == 1 ]]; then echo "::error::$1"; else echo "[!!] $1"; fi
}
warn() {
    if [[ $CI == 1 ]]; then echo "::warning::$1"; else echo "[..] $1"; fi
}
ok() { echo "[OK] $1"; }

require_tool
ensure_compdb
cd "$ROOT"

if [[ -z "$BASE" ]]; then
    BASE="$(git merge-base HEAD origin/main 2>/dev/null || git rev-parse HEAD)"
fi

# 1. 사실 최신성 ---------------------------------------------------------------
sdd extract >/dev/null
if [[ ! -f "$FACTS_DOCS" ]]; then
    problem "docs/sdd/facts.json 이 없습니다. tools/docs-sync.sh 를 먼저 실행하세요."
elif ! diff -q <(normalize_facts "$FACTS_BUILD") <(normalize_facts "$FACTS_DOCS") >/dev/null; then
    problem "코드 구조가 바뀌었는데 docs/sdd/facts.json 이 갱신되지 않았습니다. tools/docs-sync.sh 를 실행하세요."
else
    ok "사실 최신성: facts.json 이 코드와 일치"
fi

# 2. 문서 최신성 ---------------------------------------------------------------
if [[ "$BASE" != "$(git rev-parse HEAD)" ]]; then
    sdd impact --base "$BASE" >/dev/null
    stale="$(node - "$ROOT/build/impact.json" "$ROOT/docs/sdd-config/sections.yaml" "$BASE" <<'EOF'
const fs = require("fs");
const { execFileSync } = require("child_process");
const [impactPath, sectionsPath, base] = process.argv.slice(2);
const impact = JSON.parse(fs.readFileSync(impactPath, "utf8"));
// sections.yaml 은 단순한 형태라 id / output / kind 만 정규식으로 읽는다.
const text = fs.readFileSync(sectionsPath, "utf8");
const sections = {};
for (const block of text.split(/\n  - id: /).slice(1)) {
  const id = block.split("\n")[0].trim();
  const output = (block.match(/\n    output: (\S+)/) || [])[1];
  const kind = (block.match(/\n    kind: (\S+)/) || [])[1];
  sections[id] = { output, kind };
}
const changed = new Set(execFileSync("git", ["diff", "--name-only", `${base}..HEAD`, "--", "docs/sdd"], { encoding: "utf8" })
  .split("\n").map(s => s.trim()).filter(Boolean));
const stale = [];
for (const [id, reasons] of Object.entries(impact.sections || {})) {
  const sec = sections[id];
  if (!sec || !sec.output) continue;
  if (sec.kind === "per-scenario") continue; // 시나리오는 아래에서 개별 파일로 본다
  if (!changed.has(`docs/sdd/${sec.output}`)) stale.push(`${sec.output} (${id}: ${reasons.join(", ")})`);
}
for (const [id, reasons] of Object.entries(impact.scenarios || {})) {
  const file = `scenarios/${id}.md`;
  if (!changed.has(`docs/sdd/${file}`)) stale.push(`${file} (${reasons.join(", ")})`);
}
process.stdout.write(stale.join("\n"));
EOF
)"
    if [[ -n "$stale" ]]; then
        problem "다음 문서가 영향을 받았지만 갱신되지 않았습니다 (base $BASE):"
        echo "$stale" | sed 's/^/      /'
        echo "      tools/docs-sync.sh --base $BASE 로 다시 생성한 뒤 검토해서 커밋하세요."
    else
        ok "문서 최신성: 영향받은 절이 모두 갱신됨 (base ${BASE:0:10})"
    fi
else
    ok "문서 최신성: base 가 HEAD 와 같아 건너뜀"
fi

# 3. 검토 상태 -----------------------------------------------------------------
review=()
while IFS= read -r f; do
    if [[ "$(frontmatter "$f" status)" == "needs-review" ]]; then
        review+=("${f#$ROOT/}")
    fi
done < <(find "$ROOT/docs/sdd" -name "*.md" | sort)
if [[ ${#review[@]} -gt 0 ]]; then
    warn "검토가 필요한 문서 ${#review[@]} 개 (status: needs-review): ${review[*]}"
else
    ok "검토 상태: needs-review 문서 없음"
fi

# 4. 사이트 일치 ---------------------------------------------------------------
sdd export-html --out "$SITE_BUILD" >/dev/null
if [[ ! -f "$SITE_DOCS" ]]; then
    problem "docs/index.html 이 없습니다. tools/docs-sync.sh 를 실행하세요."
elif ! cmp -s "$SITE_BUILD" "$SITE_DOCS"; then
    problem "docs/index.html 이 docs/sdd 와 다릅니다. tools/docs-sync.sh --site 로 다시 만드세요."
else
    ok "사이트 일치: docs/index.html 이 docs/sdd 와 일치"
fi

exit $fail
