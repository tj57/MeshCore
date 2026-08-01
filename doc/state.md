# state.md — Living architecture status

**Branch:** `mcrpc`  
**Upstream base:** `meshcore-dev/MeshCore` @ `03b6ef4b` (2026-07-28)  
**Last updated:** 2026-08-01

## Verdict

mcRPC is implemented as an **additive application layer** on top of unchanged MeshCore. Upstream Heltec V3 sensor and RAK WisMesh Tag sensor builds succeed before changes. Both mcRPC targets (`Heltec_v3_mcrpc_button`, `LW010_mcrpc_gps`) build successfully after the refactor.

## Current directory layout (mcRPC-relevant)

```
src/mcrpc/                 # protocol + features (new)
  Parser/Registry/Dispatcher/McRpc/Config/Feature*
  features/{core,gps,battery,button}/
  features/gps/OnDemandGps.h
examples/mcrpc/            # SensorMesh transport + main
doc/                       # mcRPC documentation
test/mcrpc/                # host unit tests
variants/heltec_v3/        # append-only mcRPC envs
variants/rak_wismesh_tag/  # append-only mcRPC envs
```

Upstream layout (`src/Mesh*`, `examples/simple_*`, `variants/*`) is preserved.

## Current build system

- PlatformIO root `platformio.ini` + `variants/*/platformio.ini`
- mcRPC envs appended at end of variant files (merge-friendly)
- Docker images with PIO caches used for CI-like builds on this host

## Current command handling

```
GRP_TXT on configured channel
  → McRpcMesh::onGroupDataRecv
    → McRpc::handleIncomingText
      → Parser (strip "Sender: ")
        → Dispatcher (addressing)
          → Registry.find(command)
            → Feature handler
              → PublishFn → sendChannelText (flood)
```

Admin serial CLI still goes through SensorMesh / CommonCLI (unchanged).

## MeshCore architecture (reused)

| Piece | Role |
|-------|------|
| `Dispatcher` | Radio schedule, CAD, TX queue |
| `Mesh` | Payload dispatch, flood/direct |
| `GroupChannel` | `hash[1]` + `secret[32]` |
| `createGroupDatagram` | Encrypt + MAC group text |
| `searchChannelsByHash` | Decrypt candidates (collision-aware) |
| `SensorMesh` | Sensor role + NodePrefs + CommonCLI |
| `EnvironmentSensorManager` | GPS via `setSettingValue("gps",…)` |

### Private channels / passwords

- **Group PSK:** shared secret; hash = SHA256(secret)[0]; no join protocol
- **Admin password:** `NodePrefs` / ACL — orthogonal to mcRPC channel PSK
- mcRPC listens only on the channel configured in `/mcrpc_cfg`
- Home Assistant: run a companion/bridge that joins the same channel and maps mcRPC text ↔ entities (out of band; not in this firmware)

## Configuration system

| File | Owner | Notes |
|------|-------|-------|
| `/com_prefs` | CommonCLI / SensorMesh | Do not fork layout |
| `/mcrpc_cfg` | `mcrpc::Config` | mcRPC-only prefs |
| `/identity` | IdentityStore | Unchanged |

## Board abstraction

Unchanged: `MainBoard` + `variants/<board>/target.cpp`. Features talk to `HostServices`, implemented by `McRpcMesh`.

## Feature abstraction

`Feature` → `registerCommands(Registry&)` + optional `loop()`. Feature Manager owns lifecycle.

## Extension points

1. New feature under `src/mcrpc/features/`
2. New board env appended to variant `platformio.ini`
3. `HostServices` virtuals for new hardware capabilities
4. `PublishFn` for alternate transports (serial, BLE) later

## Code duplication (upstream debt — not ours to fix yet)

- `simple_repeater` / `room_server` / `SensorMesh` CLI paths
- Companion `MyMesh.cpp` monolith
- Variant env copy-paste

mcRPC deliberately does **not** copy those patterns into new switch trees.

## Merge conflict risk areas

| Area | Risk | Mitigation |
|------|------|------------|
| `src/Mesh.cpp` | High | Never touch |
| `CommonCLI` prefs | High | Never touch layout |
| Mid-file variant envs | Medium | Append-only blocks |
| `examples/simple_sensor/*` | Medium | Compile against, don't fork |

## Strengths

- Clear layering; parser is host-testable without Arduino
- Registry-based commands
- Upstream builds remain valid
- Both required targets compile

## Weaknesses / technical debt

- Feature static `g_*` singletons (simple, but limits multi-instance)
- Reply framing uses MeshCore `name: body` prefix; request-id appears inside body (`#42 pong`) rather than `name#42` at mesh layer — documented; protocol-compatible for HA parsers that strip sender prefix
- Relay/display/LED/OTA features not implemented (stubs pending)
- No on-device integration tests; host tests cover parser/dispatcher only
- Default channel PSKs are public demo secrets — must change in field
- `rssi` in status uses radio last RSSI (may be stale)
- GPS sats/hdop completeness depends on LocationProvider

## Recommended refactoring (next)

1. Replace static feature pointers with `CommandContext::user` carrying a small service locator
2. Add `RelayFeature` / `DisplayFeature` / `LedFeature`
3. Optional serial transport for mcRPC (not only group channel)
4. Documented HA MQTT gateway example
5. PlatformIO `native` env for tests in CI

## Estimated effort

| Item | Effort |
|------|--------|
| Done (core + 2 boards) | ~3–5 eng-days |
| Remaining feature stubs + HA bridge | 1–2 weeks |
| Multi-channel listen + ACL | 1 week |
| Upstream contribution of SensorMesh hooks | TBD (social/process) |

## Open questions

1. Should responses omit the MeshCore `Sender:` prefix and emit bare mcRPC lines? (Would need companion UX check.)
2. Multi-channel listen — one registry, many PSKs?
3. Track `origin/main` vs `origin/dev` as permanent upstream?
4. Publish request-id as `name#id` by rewriting the MeshCore sender prefix?

## Build verification log

| Target | Result |
|--------|--------|
| `Heltec_v3_sensor` (upstream) | SUCCESS |
| `RAK_WisMesh_Tag_sensor` (upstream) | SUCCESS |
| `Heltec_v3_mcrpc_button` | SUCCESS (Flash ~17%, RAM ~10%) |
| `LW010_mcrpc_gps` | SUCCESS (Flash ~54%, RAM ~12%) |
| Host `test/mcrpc` | ALL TESTS PASSED |

## Warnings

No compiler warnings were treated as errors. PIO builds use upstream `-w` on `arduino_base`. Host tests compiled with `-Wall -Wextra` cleanly.
