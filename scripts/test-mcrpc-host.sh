#!/usr/bin/env bash
# Run standalone mcRPC full test suite (sibling repo).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MCRPC="$(cd "$ROOT/../mcrpc" && pwd)"
exec "$MCRPC/scripts/test/run-all.sh"
