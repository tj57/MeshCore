#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIB="$ROOT/lib/mcrpc"
SRC="$LIB/src/mcrpc"

g++ -std=c++17 -Wall -Wextra -I "$LIB/src" -o /tmp/test_mcrpc \
  "$ROOT/test/mcrpc/test_mcrpc.cpp" \
  "$SRC/Parser.cpp" \
  "$SRC/CommandRegistry.cpp" \
  "$SRC/CapabilityRegistry.cpp" \
  "$SRC/Dispatcher.cpp" \
  "$SRC/EventBus.cpp" \
  "$SRC/FeatureManager.cpp" \
  "$SRC/McRpc.cpp" \
  "$SRC/Config.cpp" \
  "$SRC/Registry.cpp" \
  "$SRC/features/core/CoreFeature.cpp" \
  "$SRC/features/battery/BatteryFeature.cpp" \
  "$SRC/features/button/ButtonFeature.cpp" \
  "$SRC/features/gps/GpsFeature.cpp" \
  "$SRC/features/relay/RelayFeature.cpp" \
  "$SRC/features/display/DisplayFeature.cpp"
/tmp/test_mcrpc
