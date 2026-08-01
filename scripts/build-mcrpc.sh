#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
ENV_NAME="${1:-Heltec_v3_mcrpc_button}"
IMAGE="meshcore-heltec-button:latest"
VOL="heltec-button_pio-cache"
case "$ENV_NAME" in
  LW010*|RAK_*) IMAGE="meshcore-lw010-gps:latest"; VOL="meshcore_pio-cache" ;;
esac
if [[ "${USE_DOCKER:-1}" == "1" ]] && command -v docker >/dev/null; then
  docker run --rm -v "$ROOT:/workspace" -v "$VOL:/opt/platformio" \
    -w /workspace -e PLATFORMIO_CORE_DIR=/opt/platformio \
    "$IMAGE" bash -lc "pio run -e ${ENV_NAME}"
else
  pio run -e "$ENV_NAME"
fi
