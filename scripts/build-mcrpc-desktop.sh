#!/usr/bin/env bash
# Run standalone mcRPC desktop build (sibling repo).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MCRPC="$(cd "$ROOT/../mcrpc" && pwd)"
exec "$MCRPC/scripts/build/desktop.sh"
