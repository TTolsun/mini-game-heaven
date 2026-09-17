# Shared helpers for tools/docs-check.sh and tools/docs-sync.sh (sourced, not run).
#
# The SDD pipeline lives in a sibling checkout of camera-hal-sdd. Override with
# SDD_TOOL_DIR. Everything runs through `uv run --project <tool>` so the tool's
# own virtualenv is used and nothing needs to be installed into this repo.

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDD_TOOL_DIR="${SDD_TOOL_DIR:-$ROOT/../camera-hal-sdd}"
SDD_CONFIG="$ROOT/sdd.yaml"
FACTS_BUILD="$ROOT/build/sdd/facts/facts.json"
FACTS_DOCS="$ROOT/docs/sdd/facts.json"
SITE_BUILD="$ROOT/build/sdd/sdd.html"
SITE_DOCS="$ROOT/docs/index.html"

sdd() {
    uv run --project "$SDD_TOOL_DIR" sdd --config "$SDD_CONFIG" "$@"
}

require_tool() {
    if [[ ! -f "$SDD_TOOL_DIR/pyproject.toml" ]]; then
        echo "camera-hal-sdd 를 찾지 못했습니다: $SDD_TOOL_DIR (SDD_TOOL_DIR 로 지정)" >&2
        exit 2
    fi
    command -v uv >/dev/null || { echo "uv 가 필요합니다 (https://docs.astral.sh/uv/)" >&2; exit 2; }
    command -v node >/dev/null || { echo "node 가 필요합니다" >&2; exit 2; }
}

ensure_compdb() {
    if [[ ! -f "$ROOT/compile_commands.json" ]]; then
        bash "$ROOT/tools/sync-compile-commands.sh"
    fi
}

# facts.json minus the fields that change on every run or machine.
normalize_facts() {
    node -e '
const fs = require("fs");
const f = JSON.parse(fs.readFileSync(process.argv[1], "utf8"));
for (const k of ["generated_at", "source_root", "source_commit", "compile_commands"]) delete (f.meta || {})[k];
process.stdout.write(JSON.stringify(f, null, 1));
' "$1"
}

# Value of a frontmatter key in a markdown file, or empty.
frontmatter() {
    awk -v key="$2" 'NR==1 && $0!="---"{exit} NR>1 && $0=="---"{exit} NR>1 && index($0, key":")==1 {sub(key":[ ]*", ""); print; exit}' "$1"
}
