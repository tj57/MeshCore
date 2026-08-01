# mcRPC Architecture

## Design decision

mcRPC is a **standalone library** (`lib/mcrpc`) consumed by MeshCore, desktop, and (later) Home Assistant. BSP stays in upstream `variants/`. Mesh transport adapters stay in `examples/mcrpc/`.

```
lib/mcrpc/src/mcrpc/        # transport-independent library
  FeatureSdk.h
  Parser / Dispatcher / CommandRegistry / CapabilityRegistry
  EventBus / StatusBuilder / DiscoverBuilder / OutboundBuilder
  InboundMessage / McRpc / FeatureManager / Config (+ ConfigStore)
  HostServices.h
  features/<name>/

examples/mcrpc/             # MeshCore consumer only
  McRpcMesh.*               # GRP_TXT ↔ InboundMessage
  ArduinoFsConfigStore.h
  drivers/OnDemandGps.h
```

## Layers

```
┌──────────────────────────────────────────────┐
│ Features  (Feature SDK only)                 │
├──────────────────────────────────────────────┤
│ CommandRegistry ← Dispatcher ← Parser        │
│ CapabilityRegistry / Status / Discover       │
│ EventBus                                     │
├──────────────────────────────────────────────┤
│ McRpc facade + FeatureManager + Config       │
├──────────────────────────────────────────────┤
│ HostServices (app/board callbacks)           │
├──────────────────────────────────────────────┤
│ McRpcMesh transport (SensorMesh + GRP_TXT)   │
├──────────────────────────────────────────────┤
│ Radio / MainBoard (variants/)                │
└──────────────────────────────────────────────┘
```

### Invariants

1. Parser never includes Arduino, RadioLib, MeshCore Packet, or board headers  
2. Features never parse MeshCore packets  
3. Dispatcher never knows Feature types — only CommandRegistry  
4. Adding a command = `registerCommand()` inside `registerCommands()`  
5. Capabilities come only from CapabilityRegistry  

## Feature lifecycle

```
construct → FeatureManager::add
         → start:
              setup(FeatureContext)
              registerCommands(CommandRegistry)
              registerCapabilities(CapabilityRegistry)
         → loop*
         → stop: shutdown (reverse order)
```

`FeatureContext` provides `commands`, `capabilities`, `events`, `manager`.

Features publish with `publishEvent(name, kv)` → EventBus → subscribers.

## Packet lifecycle

```
Raw radio bytes
  → MeshCore decrypt GRP_TXT (native channel hash + PSK)
  → "Sender: <mcRPC line>" text
  → InboundMessage { text, optional rssi }
  → Parser::stripSenderPrefix
  → Parser::parse → Request
  → Dispatcher (addressing) → CommandRegistry → handler
  → ReplyBuffer → PublishFn → createGroupDatagram → flood
```

Parser is host-testable without MeshCore (`test/mcrpc`).

## Event lifecycle

```
Feature::publishEvent("button_pressed", "count=3")
  → EventBus::publish
  → each subscriber
       default: McRpc formats "event button_pressed count=3" → mesh
       future: HA bridge, logger, display, BLE, storage
```

Features never know who consumes events.

## Capability registration

```
Feature::registerCapabilities(caps)
  caps.registerCapability("gps");
```

`caps` command → `CapabilityRegistry::writeTo` (one name per line). No hardcoded lists.

## Status / discover generation

```
status:
  McRpc::buildStatus
    + identity fields (name, profile, fw, uptime, rssi)
    + FeatureManager::collectStatus → each Feature::contributeStatus
  → StatusBuilder::writeTo → "status key=value ..."

discover:
  McRpc::buildDiscover
    + name + profile + fw
    + Feature::contributeDiscover
  → "name profile=… fw=… gps=yes battery=yes ..."
```

## Configuration

| Store | Role |
|-------|------|
| `/com_prefs` | MeshCore NodePrefs (radio, admin) |
| `/mcrpc_cfg` | mcRPC profile, channel, PSK, feature flags |

Build macros seed defaults once; filesystem wins afterward. See [PRIVATE_CHANNELS.md](PRIVATE_CHANNELS.md).

## Board independence

Features use `HostServices` only. GPS power sessions live in `drivers/OnDemandGps` (host wiring in `McRpcMesh`), not inside `GpsFeature`. No `#ifdef BOARD_*` in feature sources.
