#!/usr/bin/env bash
# Copy the newest compile_commands.json produced by the Gradle/CMake build to the
# repo root so clangd and IDEs can find it. Run after `./gradlew assembleDebug`.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
src="$(ls -t "$root"/app/.cxx/Debug/*/arm64-v8a/compile_commands.json 2>/dev/null | head -1)"
if [[ -z "$src" ]]; then
    echo "compile_commands.json not found. Run ./gradlew assembleDebug first." >&2
    exit 1
fi
cp "$src" "$root/compile_commands.json"
echo "copied $src -> $root/compile_commands.json"
