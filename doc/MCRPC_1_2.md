# mcRPC 1.2 consumer notes (MeshCore)

Depends on **mcrpc `v1.2.3`** (`platformio.ini` `[mcrpc_lib]`).

See [RFC-0002](https://github.com/tj57/mcrpc/blob/main/docs/rfc/RFC-0002-mcrpc-1.2-slim-call.md)
in the mcrpc tree (`/data/projects/mcrpc/docs/rfc/RFC-0002-mcrpc-1.2-slim-call.md`).

## Firmware behaviour

- Discovery (slim): `id` (8 hex), `fw`, `v=1.2`, `tag`, `up`, `caps` — no
  `protocol*` / `sdk` / `features=` / `transport=` / `profile=` / full id.
- Status (rich): `id_full`, rssi/snr, battery, heap, `transport=meshcore`, …
- Unknown discover/status fields MUST be ignored by clients.
- `call ns.action` is a normal command (parser-neutral); replies use
  `ok` / `err <code>` / `busy` / `retry` with optional `key=value` only.
- Events: prefer dotted names (`event button.pressed count=N`).
- `setNodeId` = full public-key hex; discovery truncates to 8; `@` prefix match unchanged.
- Dispatcher matches **name** or **@id** only — never tag.

## Local dev

```bash
cp platformio.local.ini.example platformio.local.ini
# uses symlink://../mcrpc
pio run -e Heltec_v3_mcrpc_button
```

## Host tests

Prefer the canonical suite in `/data/projects/mcrpc` (`./scripts/release-check` or CMake tests).
