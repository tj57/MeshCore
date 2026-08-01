#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/src/mcrpc"
g++ -std=c++17 -Wall -Wextra -I "$ROOT/src" -o /tmp/test_mcrpc \
  "$ROOT/test/mcrpc/test_mcrpc.cpp" \
  "$SRC/Parser.cpp" \
  "$SRC/CommandRegistry.cpp" \
  "$SRC/CapabilityRegistry.cpp" \
  "$SRC/Dispatcher.cpp" \
  "$SRC/EventBus.cpp" \
  "$SRC/FeatureManager.cpp" \
  "$SRC/McRpc.cpp" \
  "$SRC/Registry.cpp"
/tmp/test_mcrpc
