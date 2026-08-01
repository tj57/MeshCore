# mcRPC Architecture

## Design decision (vs proposed tree)

The suggested top-level `src/config`, `src/hardware/lw010`, … would fight PlatformIO’s existing `variants/` + `helpers/` layout and create permanent merge friction.

**Chosen layout (5-year maintainer view):**

```
src/mcrpc/                 # all mcRPC code (additive filter)
  config → Config.*
  feature_manager → FeatureManager.*
  features/<name>/
  Parser / Dispatcher / Registry / McRpc
examples/mcrpc/            # thin SensorMesh transport
variants/<board>/          # append-only [env:*_mcrpc_*]
```

Hardware BSP stays in upstream `variants/`. Features never include board headers.

## Layers

```
┌─────────────────────────────────────────┐
│ Features (gps, button, battery, core…)  │
├─────────────────────────────────────────┤
│ Registry  ←  Dispatcher  ←  Parser      │
├─────────────────────────────────────────┤
│ McRpc facade + Config + FeatureManager  │
├─────────────────────────────────────────┤
│ HostServices (implemented by McRpcMesh) │
├─────────────────────────────────────────┤
│ MeshCore SensorMesh / GroupChannel TXRX │
├─────────────────────────────────────────┤
│ Radio / MainBoard (variants/)           │
└─────────────────────────────────────────┘
```

### Invariants

1. Parser never includes Arduino, RadioLib, or board headers
2. Features never parse MeshCore packets
3. Business logic never lives in the parser
4. Adding a command = one `registerCommand` call

## Packet flow

**Inbound**

1. Radio RX → Dispatcher → Mesh decrypts GRP_TXT via `searchChannelsByHash`
2. `McRpcMesh::onGroupDataRecv` extracts plain text after timestamp/type bytes
3. `McRpc::handleIncomingText` strips optional `Sender: ` prefix
4. Parser → addressing check → Registry → handler → ReplyBuffer
5. `PublishFn` → `sendChannelText` → `createGroupDatagram` → `sendFlood`

**Outbound events**

`McRpc::publishEvent("button_pressed", "count=3")` → same publish path, no request id.

## Command registration

```cpp
registry.registerCommand("ping", &CoreFeature::cmdPing, "connectivity test", nullptr);
registry.registerCommand("gps", &GpsFeature::cmdGps, "get GPS fix", "gps");
```

Capability strings feed `caps`. Core commands omit capability (not listed).

## Configuration model

- **Radio / node identity / admin password:** MeshCore `NodePrefs` (`/com_prefs`)
- **mcRPC channel + profile + feature flags:** `mcrpc::Config` (`/mcrpc_cfg`)
- Channel secret: 16 ASCII bytes by default; hash via `mesh::Utils::sha256` — same as `BaseChatMesh::addChannel`

## Why SensorMesh?

Repeaters/room servers do not store group channels. Sensor role already has telemetry, battery hooks, and CommonCLI. Extending SensorMesh for transport reuses the most appropriate upstream role without inventing a parallel mesh stack.

## Home Assistant integration sketch

1. Phone/companion or ESP gateway joins `#mych` with the shared PSK
2. Gateway publishes received mcRPC lines to MQTT
3. HA automations map `event button_pressed` → entities; `tracker gps` → device_tracker

Firmware does not embed MQTT — keeps flash small and transport-independent (protocol requirement).
