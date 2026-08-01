# MeshCore + mcRPC

Long-term maintainable extension of [MeshCore](https://github.com/meshcore-dev/MeshCore).

**mcRPC is a standalone, transport-independent library** (`lib/mcrpc`) with a Feature SDK. MeshCore firmware is one consumer; desktop tools and the Home Assistant MeshCore integration will use the **same library** so parse/build behaviour stays identical across platforms.

This repository is a clean checkout of upstream MeshCore on branch `mcrpc`. The library lives under `lib/mcrpc/`; MeshCore-specific transport code stays in `examples/mcrpc/`.

## Project goals

- Keep MeshCore radio/routing/crypto untouched
- Speak a stable text protocol over private group channels
- Organize firmware around **features**, not boards
- Stable Feature SDK: `setup` / `registerCommands` / `registerCapabilities` / `loop` / `shutdown`
- EventBus, CapabilityRegistry, Status/Discover builders assemble protocol responses
- Remain easy to sync with upstream for the next five years

## Architecture

```
┌─────────────────────────────────────────────┐
│  lib/mcrpc  (transport-independent)         │
│  Parser · Registry · EventBus · Feature SDK │
│  Status/Discover/Outbound builders          │
└─────────────────────────────────────────────┘
        ↑                    ↑
 MeshCore consumer      Desktop / HA
 examples/mcrpc/        CMake / bindings
 (GRP_TXT transport)
```

```
Radio → MeshCore → McRpcMesh → lib/mcrpc → Features → HostServices
```

| Layer | Owns | Must not know |
|-------|------|----------------|
| Parser | Grammar only | Hardware, MeshCore |
| CommandRegistry | Command → handler | Features, pins |
| CapabilityRegistry | `caps` list | Commands |
| EventBus | Pub/sub events | Mesh framing |
| Features | Commands + contributions | Packet wire format |
| HostServices | Battery/GPS/button IO | Protocol grammar |

See [doc/mcRPC-ARCHITECTURE.md](doc/mcRPC-ARCHITECTURE.md) and [doc/PRIVATE_CHANNELS.md](doc/PRIVATE_CHANNELS.md).

## mcRPC

Text protocol (v1). Example session on channel `#mych`:

```
tracker#18 gps
→ tracker: #18 gps lat=50.12 lon=19.93 alt=231.0 sat=12

button ping
→ button: pong

all discover
→ tracker profile=tracker fw=mcrpc-0.1.0
```

Full grammar and semantics: [doc/mcRPC-CORE.md](doc/mcRPC-CORE.md).

## Supported mcRPC targets

| Env | Board | Profile | Features |
|-----|-------|---------|----------|
| `Heltec_v3_mcrpc_button` | Heltec WiFi LoRa 32 V3 | switch | button, battery, core |
| `LW010_mcrpc_gps` | MOKO LW010-R / RAK WisMesh Tag | tracker | gps, button, battery, core |
| `RAK_WisMesh_Tag_mcrpc_gps` | same hardware | tracker | same |

Upstream MeshCore still supports ~79 board variants / 500+ envs — see [doc/BOARDS.md](doc/BOARDS.md).

## Building

Docker (recommended on this host):

```bash
# Heltec V3 button
docker run --rm -v "$PWD:/workspace" -v heltec-button_pio-cache:/opt/platformio \
  -w /workspace -e PLATFORMIO_CORE_DIR=/opt/platformio \
  meshcore-heltec-button:latest \
  bash -lc 'pio run -e Heltec_v3_mcrpc_button'

# LW010 GPS
docker run --rm -v "$PWD:/workspace" -v meshcore_pio-cache:/opt/platformio \
  -w /workspace -e PLATFORMIO_CORE_DIR=/opt/platformio \
  meshcore-lw010-gps:latest \
  bash -lc 'pio run -e LW010_mcrpc_gps'
```

Local PlatformIO:

```bash
pio run -e Heltec_v3_mcrpc_button
pio run -e LW010_mcrpc_gps
```

Library / host tests:

```bash
./scripts/test-mcrpc-host.sh          # g++ against lib/mcrpc
./scripts/build-mcrpc-desktop.sh      # CMake + ctest
```

See [lib/mcrpc/README.md](lib/mcrpc/README.md).

## Configuration

| Store | Path | Contents |
|-------|------|----------|
| MeshCore `NodePrefs` | `/com_prefs` | node name, radio, admin password (unchanged) |
| mcRPC `Config` | `/mcrpc_cfg` | profile, channel name, channel PSK, feature flags |

**Private channels** reuse MeshCore group crypto: 16/32-byte PSK → SHA-256 → 1-byte channel hash on the wire. There is no join handshake — knowing the PSK is membership.

Default LW010 channel: name `mych`, PSK ASCII `mych-gps-channel` (base64 `bXljaC1ncHMtY2hhbm5lbA==`).

Change defaults via build flags `MCRPC_DEFAULT_*` or persist new values in `/mcrpc_cfg`.

## Adding a new feature

1. `#include <mcrpc/FeatureSdk.h>`
2. Create `src/mcrpc/features/<name>/` and subclass `mcrpc::Feature`
3. Implement `registerCommands`, `registerCapabilities`, optional `contributeStatus` / `contributeDiscover`
4. Publish async notifications with `publishEvent()` (EventBus) — do not call MeshCore
5. Use `HostServices` for hardware — never parse packets; no board `#ifdef` in the feature
6. `features().add(&myFeature)` in the app **before** `McRpc::begin()`
7. Document it in [doc/FEATURES.md](doc/FEATURES.md)

## Adding a new board

1. Prefer an existing `variants/<board>/` from upstream
2. Append an `[env:…_mcrpc_…]` block at the **end** of that variant’s `platformio.ini` (do not rewrite upstream envs)
3. Select features with `-D MCRPC_ENABLE_*`
4. Document in [doc/BOARDS.md](doc/BOARDS.md)

## Relationship with upstream MeshCore

| Path | Policy |
|------|--------|
| `src/Mesh*.cpp`, `Dispatcher.*`, `Packet.*` | Do not modify |
| `src/helpers/**` (upstream) | Prefer extend, avoid edit |
| `src/mcrpc/**` | mcRPC-owned |
| `examples/mcrpc/**` | mcRPC-owned |
| `variants/*/platformio.ini` | Append-only env blocks |
| `doc/mcRPC-*.md`, `doc/state.md` | mcRPC-owned |

Sync:

```bash
git fetch origin
git merge origin/main   # or rebase onto origin/dev if that is your upstream track
```

Conflict hotspots to watch: giant `examples/*/MyMesh.cpp`, `CommonCLI` prefs layout, mid-file variant env edits. Keeping mcRPC additive avoids most of them.

## Roadmap

See [doc/state.md](doc/state.md) and [doc/mcRPC-DEVELOPMENT.md](doc/mcRPC-DEVELOPMENT.md).

Near term: relay/display/LED features, Home Assistant MQTT bridge notes, deeper native tests, OTA profile.

## Documentation index

| Doc | Purpose |
|-----|---------|
| [doc/state.md](doc/state.md) | Living architecture analysis |
| [doc/mcRPC-CORE.md](doc/mcRPC-CORE.md) | Protocol specification |
| [doc/mcRPC-ARCHITECTURE.md](doc/mcRPC-ARCHITECTURE.md) | SDK, lifecycles, builders |
| [doc/PRIVATE_CHANNELS.md](doc/PRIVATE_CHANNELS.md) | Native MeshCore channel/PSK |
| [doc/mcRPC-PROFILES.md](doc/mcRPC-PROFILES.md) | Device profiles |
| [doc/mcRPC-DEVELOPMENT.md](doc/mcRPC-DEVELOPMENT.md) | Contributor guide |
| [doc/FEATURES.md](doc/FEATURES.md) | Feature catalog |
| [doc/BOARDS.md](doc/BOARDS.md) | Board inventory |

## License

Same as upstream MeshCore (see `license.txt`).
