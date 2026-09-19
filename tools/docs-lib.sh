# Shared helpers for tools/docs-check.sh and tools/docs-sync.sh (sourced, not run).
#
# The SDD pipeline is an immutable checkout selected by tools/sdd-tool.json.

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDD_TOOL_DIR="$ROOT/.tool/camera-hal-sdd"
SDD_CONFIG="$ROOT/sdd.yaml"
FACTS_BUILD="$ROOT/build/sdd/facts/facts.json"
FACTS_DOCS="$ROOT/docs/sdd/facts.json"
SITE_BUILD="$ROOT/build/sdd/sdd.html"
SITE_DOCS="$ROOT/docs/index.html"
SITE_TITLE="무천 SRPG SDD"

sdd() {
    uv run --frozen --project "$SDD_TOOL_DIR" sdd --config "$SDD_CONFIG" "$@"
}

export_site() {
    sdd export-html --title "$SITE_TITLE" --out "$1"
}

require_tool() {
    if [[ ! -f "$SDD_TOOL_DIR/pyproject.toml" ]]; then
        echo "Run bash tools/setup-sdd.sh first: $SDD_TOOL_DIR" >&2
        exit 2
    fi
    command -v uv >/dev/null || { echo "uv 가 필요합니다 (https://docs.astral.sh/uv/)" >&2; exit 2; }
    command -v node >/dev/null || { echo "node 가 필요합니다" >&2; exit 2; }
    local pinned
    pinned="$(node -p 'require(process.argv[1]).revision' "$ROOT/tools/sdd-tool.json")"
    [[ "$(git -C "$SDD_TOOL_DIR" rev-parse HEAD)" == "$pinned" ]] || { echo 'SDD revision mismatch' >&2; exit 2; }
    [[ -z "$(git -C "$SDD_TOOL_DIR" status --porcelain)" ]] || { echo 'SDD checkout is dirty' >&2; exit 2; }
}

ensure_compdb() {
    bash "$ROOT/tools/sync-compile-commands.sh"
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
