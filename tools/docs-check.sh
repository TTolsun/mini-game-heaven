#!/usr/bin/env bash
# Validate extracted facts, explicit content-bound review, and committed HTML.
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/docs-lib.sh"
while [[ $# -gt 0 ]]; do
  case "$1" in
    --ci) shift ;;
    --base) [[ $# -ge 2 ]] || exit 2; shift 2 ;; # Compatibility; hashes cover working-tree changes too.
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done
require_tool
ensure_compdb
cd "$ROOT"
fail=0
problem() { echo "[FAIL] $1" >&2; fail=1; }
sdd extract
if [[ ! -f "$FACTS_DOCS" ]] || ! diff -q <(normalize_facts "$FACTS_BUILD") <(normalize_facts "$FACTS_DOCS") >/dev/null; then
  problem 'facts.json is stale. Run tools/docs-sync.sh and review the resulting documents.'
fi
if ! node tools/docs-review.mjs check; then
  problem 'Review missing or stale. Record the review after checking source and documentation.'
fi
sdd export-html --title "$SITE_TITLE" --out "$SITE_BUILD"
if [[ ! -f "$SITE_DOCS" ]] || ! cmp -s "$SITE_BUILD" "$SITE_DOCS"; then
  problem 'docs/index.html differs from the Markdown. Run tools/docs-sync.sh --site.'
fi
exit "$fail"
