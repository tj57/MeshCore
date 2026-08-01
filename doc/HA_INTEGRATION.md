# Home Assistant integration notes

The MeshCore HA integration should depend on **`lib/mcrpc`**, not re-implement the grammar.

## Recommended usage

| Need | API |
|------|-----|
| Parse inbound channel text | `Parser::stripSenderPrefix` + `Parser::parse` → `Request` |
| Build outbound commands | `OutboundBuilder::request` / `requestWithArgs` |
| Parse/format events | look for `event ` prefix; `OutboundBuilder::event` |
| Status / discover lines | `StatusBuilder` / `DiscoverBuilder` (or parse `key=value`) |

Do **not** require `McRpc` / FeatureManager / HostServices in HA unless you embed a full node simulator.

## Binding options (future)

1. Compile `libmcrpc.a` / `.so` via CMake; call from Python (`ctypes` / `cffi` / pybind11)
2. Thin C API wrapper (`mcrpc_c.h`) for stable ABI
3. Pure reimplementation only if bindings are blocked — must stay bit-compatible with host tests

## Compatibility guarantee

Host tests in `test/mcrpc/` are the contract. HA CI should run `./scripts/build-mcrpc-desktop.sh` (or equivalent) against the same commit as firmware.
