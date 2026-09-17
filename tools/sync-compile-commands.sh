#!/usr/bin/env bash
# Copy the newest compile_commands.json produced by the Gradle/CMake build to the
# repo root so clangd, IDEs and the SDD pipeline can find it. Run after
# `./gradlew assembleDebug`.
#
# One adjustment on the way: libclang (pip package, used by camera-hal-sdd) does
# not ship clang's builtin headers, so every entry gets `-resource-dir` pointing
# at the NDK toolchain's own resource directory, derived from the compiler path.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
src="$(ls -t "$root"/app/.cxx/Debug/*/arm64-v8a/compile_commands.json 2>/dev/null | head -1)"
if [[ -z "$src" ]]; then
    echo "compile_commands.json not found. Run ./gradlew assembleDebug first." >&2
    exit 1
fi
node - "$src" "$root/compile_commands.json" <<'EOF'
const fs = require("fs");
const path = require("path");
const [src, dst] = process.argv.slice(2);
const entries = JSON.parse(fs.readFileSync(src, "utf8"));
const cache = new Map();
function resourceDir(compiler) {
  if (cache.has(compiler)) return cache.get(compiler);
  // <prefix>/bin/clang++[.exe] -> <prefix>/lib/clang/<version>
  const prefix = path.dirname(path.dirname(compiler));
  const libClang = path.join(prefix, "lib", "clang");
  let dir = "";
  if (fs.existsSync(libClang)) {
    const versions = fs.readdirSync(libClang).sort();
    if (versions.length) dir = path.join(libClang, versions[versions.length - 1]).replace(/\\/g, "/");
  }
  cache.set(compiler, dir);
  return dir;
}
for (const e of entries) {
  if (e.command && !e.command.includes("-resource-dir")) {
    // The compiler is the first token; it may be quoted because of spaces.
    const m = e.command.match(/^("([^"]+)"|(\S+))\s/);
    const compiler = (m[2] || m[3] || "").replace(/\\\\/g, "\\");
    const dir = resourceDir(compiler);
    if (dir) e.command = e.command.replace(/^("[^"]+"|\S+)\s/, `$1 -resource-dir ${dir} `);
  } else if (e.arguments && !e.arguments.includes("-resource-dir")) {
    const dir = resourceDir(e.arguments[0]);
    if (dir) e.arguments.splice(1, 0, "-resource-dir", dir);
  }
}
fs.writeFileSync(dst, JSON.stringify(entries, null, 1) + "\n");
console.log(`copied ${src} -> ${dst} (${entries.length} entries)`);
EOF
