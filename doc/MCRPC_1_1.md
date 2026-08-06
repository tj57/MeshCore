# mcRPC 1.1 consumer notes (MeshCore)

Depends on **mcrpc `v1.1.0-beta`** (`platformio.ini` `[mcrpc_lib]`).

## Firmware behaviour (Phase 2)

- Discovery emits RFC-0001 fields via `McRpc::buildDiscover` (id, tag, caps,
  features, uptime, protocol_min/max).
- `setNodeId` = full public-key hex; clients may address `@` + unique prefix.
- `setTag` = config profile string (UI hint); `profile=` still emitted for 1.0.
- Dispatcher matches **name** or **@id** only — never tag/capability.

## Local dev

```bash
cp platformio.local.ini.example platformio.local.ini
# uses symlink://../mcrpc
pio run -e Heltec_v3_mcrpc_button
```

## Host tests

```bash
# from MeshCore tree with mcrpc headers on include path
g++ -std=c++17 -I../mcrpc/include test/mcrpc/test_mcrpc.cpp ...
```

Prefer running the canonical suite in `/data/projects/mcrpc` (`./scripts/release-check`).
