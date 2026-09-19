#!/usr/bin/env bash
# Install only the pinned engine; never reset an existing checkout.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
readarray -t lock < <(node -e 'const f=require(process.argv[1]); console.log(f.repository); console.log(f.revision)' "$ROOT/tools/sdd-tool.json")
repo="${lock[0]}"
revision="${lock[1]}"
[[ "$revision" =~ ^[0-9a-f]{40}$ ]] || { echo 'Invalid SDD revision' >&2; exit 1; }
dest="$ROOT/.tool/camera-hal-sdd"
if [[ ! -d "$dest" ]]; then
  git clone --no-checkout "$repo" "$dest"
  git -C "$dest" checkout --detach "$revision"
fi
[[ "$(git -C "$dest" rev-parse HEAD)" == "$revision" ]] || {
  echo "SDD revision mismatch. Move $dest aside and run tools/setup-sdd.sh again." >&2; exit 1;
}
[[ -z "$(git -C "$dest" status --porcelain --untracked-files=normal)" ]] || {
  echo 'SDD engine checkout is dirty.' >&2; exit 1;
}
uv sync --project "$dest" --frozen
