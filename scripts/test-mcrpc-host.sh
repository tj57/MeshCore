#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
g++ -std=c++17 -Wall -Wextra -I "$ROOT/src" -o /tmp/test_mcrpc \
  "$ROOT/test/mcrpc/test_mcrpc.cpp" \
  "$ROOT/src/mcrpc/Parser.cpp" \
  "$ROOT/src/mcrpc/Registry.cpp" \
  "$ROOT/src/mcrpc/Dispatcher.cpp" \
  "$ROOT/src/mcrpc/FeatureManager.cpp"
/tmp/test_mcrpc
