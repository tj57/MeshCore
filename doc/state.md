# state.md — Living architecture status

**Branch:** `mcrpc`  
**Upstream base:** `meshcore-dev/MeshCore` @ `03b6ef4b`  
**Last updated:** 2026-08-01 (Feature SDK phase)

## Verdict

mcRPC is now a **reusable Feature SDK / framework**, not a bag of ad-hoc commands. Future modules depend only on the stable Feature API (`FeatureSdk.h`); they never touch Parser, Dispatcher, or MeshCore packet types.

## Why the SDK was introduced

Phase-1 features called `McRpc::publishEvent` and formatted `status`/`caps` strings by hand. That would force every new feature to learn internal objects and would freeze bad patterns. The SDK makes the framework assemble protocol responses and route events, so features stay small and replaceable for years.

## Current architecture

```
Transport (McRpcMesh / MeshCore GRP_TXT)
    ↓ InboundMessage (text only)
Parser → CommandObject/Request
    ↓
Dispatcher → CommandRegistry → handler
    ↑
FeatureManager lifecycle
    setup → registerCommands → registerCapabilities → loop → shutdown
    ↓
Features (Feature SDK only)
    ↓ EventBus.publish
Subscribers (McRpc → mesh publish; future: HA, log, display…)
```

| Component | Role |
|-----------|------|
| `Feature` / `FeatureContext` | Stable SDK |
| `CommandRegistry` | Command → handler (no capabilities) |
| `CapabilityRegistry` | `caps` source of truth |
| `EventBus` | Decoupled async events |
| `StatusBuilder` / `DiscoverBuilder` | Assembled protocol lines |
| `InboundMessage` | Transport-agnostic input |
| `HostServices` | Hardware abstract IO |
| `drivers/OnDemandGps` | Host/driver — not a Feature |

## Feature lifecycle

Owned **only** by `FeatureManager`:

1. `add(feature)` before start  
2. `start` → `setup` → `registerCommands` → `registerCapabilities`  
3. `loop`  
4. `stop` → `shutdown` (reverse order)

Features must not register commands outside `registerCommands()`.

## Remaining weaknesses

- Static `g_*` pointers inside feature command handlers (multi-instance still limited)
- CoreFeature reaches `McRpc` via `HostServices::engine` for builders (acceptable, but a narrower `FrameworkServices` iface would be cleaner)
- Relay/display/LED stubs still `err unsupported`
- No multi-channel listen
- Default PSKs are demo secrets

## Future extension model

1. `#include <mcrpc/FeatureSdk.h>`  
2. Subclass `Feature`, implement the virtuals  
3. Use `HostServices` for IO; `publishEvent()` for async  
4. `features().add(&myFeature)` in the app before `McRpc::begin()`  
5. Never include Parser/Dispatcher/Mesh headers in the feature

## Private channels

See [PRIVATE_CHANNELS.md](PRIVATE_CHANNELS.md) — native MeshCore group crypto only.

## Build verification

| Target | Result |
|--------|--------|
| Host tests | ALL PASSED (108 assertions) |
| `Heltec_v3_mcrpc_button` | SUCCESS |
| `LW010_mcrpc_gps` | SUCCESS |

## Suitability as long-term base

**Yes** — with the additive layout + Feature SDK, this is suitable as the long-term base for MeshCore application extensions, provided the policy holds: no edits to MeshCore core, features only via SDK, append-only board envs.
