# state.md — Living architecture status

**Branch:** `mcrpc`  
**Last updated:** 2026-08-01 (standalone library extraction)

## Verdict

**mcRPC is a standalone, transport-independent library** (`lib/mcrpc`). MeshCore firmware is one consumer. Desktop/CMake builds and host tests use the same sources. Home Assistant can link the same library for identical parse/build behaviour.

## Layout

```
lib/mcrpc/                 # Standalone library (PlatformIO + CMake)
  src/mcrpc/               # Public headers + implementation
  CMakeLists.txt
  library.json
  README.md

examples/mcrpc/            # MeshCore consumer (transport + drivers)
  McRpcMesh.*              # GRP_TXT bridge
  ArduinoFsConfigStore.h   # FS persistence adapter
  drivers/OnDemandGps.h    # MeshCore GPS power helper

test/mcrpc/                # Host tests against the library
```

## Consumer model

| Consumer | Uses | Does not use |
|----------|------|----------------|
| MeshCore firmware | lib + McRpcMesh + ArduinoFsConfigStore + OnDemandGps | — |
| Desktop / CMake | lib only | Arduino, radio |
| Home Assistant (future) | Parser, OutboundBuilder, builders | Feature firmware loop |

## Why extracted

1. One protocol implementation across firmware and HA  
2. No Arduino/MeshCore headers in the core library  
3. Config persistence via `ConfigStore` interface  
4. CMake installable for Python/C++ HA components (bindings later)

## Build verification

| Target | Result |
|--------|--------|
| `./scripts/test-mcrpc-host.sh` | PASSED (108) |
| `./scripts/build-mcrpc-desktop.sh` | PASSED |
| `Heltec_v3_mcrpc_button` | SUCCESS |
| `LW010_mcrpc_gps` | SUCCESS |

## Remaining debt

- HA language bindings (ctypes/pybind) not yet written  
- Feature static `g_*` handlers  
- Optional: split “core protocol only” CMake target without embedded features for smaller HA deps  

## Suitability

**Yes** — protocol library + MeshCore adapter is the right long-term shape for multi-platform mcRPC.
