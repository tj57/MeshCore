# mcRPC Development Guide

## Branching

- Working branch: `mcrpc`
- Upstream remote: `origin` → `https://github.com/meshcore-dev/MeshCore.git`
- Prefer small, logical commits
- Never rewrite upstream env blocks — append

## Sync with upstream

```bash
git fetch origin
git merge origin/main
# fix conflicts only in append regions / src/mcrpc / examples/mcrpc / doc/
```

If a conflict appears inside `src/Mesh.cpp` or `CommonCLI.cpp`, stop and re-evaluate — mcRPC should not need those edits.

## Coding rules

- C++17-ish subset used by Arduino cores; avoid heap when possible
- 2-space indent (match `.clang-format`)
- New protocol behavior → update `doc/mcRPC-CORE.md` **before** code if it changes the wire contract
- Features: no `#include <target.h>`

## Build & test loop

1. Host tests: `test/mcrpc/test_mcrpc.cpp`
2. `pio run -e Heltec_v3_mcrpc_button`
3. `pio run -e LW010_mcrpc_gps`
4. Update `doc/state.md` build table

## Adding a feature (checklist)

- [ ] `#include <mcrpc/FeatureSdk.h>` only (no Parser/Dispatcher)
- [ ] `src/mcrpc/features/<name>/` subclassing `Feature`
- [ ] `registerCommands` + `registerCapabilities`
- [ ] Optional `contributeStatus` / `contributeDiscover`
- [ ] Events via `publishEvent()` / EventBus
- [ ] Extend `HostServices` only if new IO is required
- [ ] Board drivers go under `src/mcrpc/drivers/` or the app — not in the feature
- [ ] `features().add()` before `McRpc::begin()`
- [ ] Gate with `MCRPC_ENABLE_<NAME>`
- [ ] Docs: FEATURES.md + PROFILES.md
- [ ] Host test for pure logic if any

## Adding a board (checklist)

- [ ] Confirm upstream `variants/<board>/` exists and builds a sensor env
- [ ] Append `[env:Board_mcrpc_…]` at file end
- [ ] `-I src -I examples/simple_sensor -I examples/mcrpc`
- [ ] `build_src_filter` includes `mcrpc` + `examples/mcrpc` + SensorMesh sources
- [ ] Document in BOARDS.md

## Commit style

Follow the suggested sequence when possible: analysis → architecture docs → config → feature manager → parser → dispatcher → registry → core → gps → button → docs → cleanup.
