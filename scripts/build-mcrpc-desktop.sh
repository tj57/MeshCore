#!/usr/bin/env bash
# Configure + build + test the standalone mcRPC library (desktop).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build/mcrpc"
cmake -S "$ROOT/lib/mcrpc" -B "$BUILD" -DMCRPC_BUILD_TESTS=ON
cmake --build "$BUILD" -j"$(nproc 2>/dev/null || echo 2)"
ctest --test-dir "$BUILD" --output-on-failure
echo "Desktop library OK: $BUILD"
