# Home Assistant integration architecture

MeshCore firmware is only a **transport consumer** of mcRPC. The Home Assistant
integration lives in a separate repository and must remain transport-oriented.

**Canonical HA fork (branch `mcrpc`):**  
https://github.com/tj57/meshcore-ha

**Protocol library:**  
https://github.com/tj57/mcrpc (`python/` package + C++ reference)

Do **not** re-implement the mcRPC grammar inside Home Assistant. Depend on the
standalone `mcrpc` Python package (same golden/compliance tests as C++).

---

## Architecture

```
┌──────────────────────────────────────────────────┐
│ Home Assistant                                   │
│  meshcore integration (USB / BLE / TCP)          │
│         │                                        │
│         ▼                                        │
│  meshcore.request / broadcast / raw              │
│  McRpcBridge · Node Registry · events            │
│         │ uses package: mcrpc (Python)           │
│         ▼                                        │
│  channel text send/receive (existing MeshCore)   │
└──────────────────────────────────────────────────┘
              ▲ text on mesh channel
┌─────────────┴────────────────────────────────────┐
│ Mesh device firmware (libmcrpc + Feature SDK)    │
│ examples/mcrpc/ — GRP_TXT adapter only           │
└──────────────────────────────────────────────────┘
```

HA is a **peer** on the channel. Prefer **not** embedding C++ `McRpc` /
FeatureManager in HA.

---

## Public HA API (user-facing)

| Service | Role |
|---------|------|
| `meshcore.request` | Normal node request (`wait` / `parse` / `response_variable`) |
| `meshcore.broadcast` | All nodes → `responses[]` |
| `meshcore.raw` | Advanced arbitrary text |
| `meshcore.send_mcrpc` | Debug alias of `raw` |
| `meshcore.list_nodes` | Node Registry cache |
| `meshcore.has_capability` | Capability check |

Events: `meshcore_response`, `meshcore_event` (protocol-agnostic names).

Optional — disabled until **Configure → Mesh Node Requests (mcRPC)**.

That section configures listening channels, accepted addressing, allowed senders,
reply identity, and diagnostics. Secure defaults: no answer on Public, bare
commands off. See HA `docs/MCRPC.md` (Configuration / Security / Migration).

Docs in the HA repo: `docs/MCRPC.md`, `docs/ARCHITECTURE_MCRPC.md`,
`examples/automations/`.

---

## Compatibility

- Existing MeshCore HA messaging/entities unchanged when node requests are off.
- Firmware builds continue to consume mcRPC via PlatformIO (`symlink://` dev or
  git tag release) — see `doc/MCRPC_DEPENDENCY.md`.
- Protocol contract = `mcrpc` golden + compliance tests.

---

## Status

Implemented in **tj57/meshcore-ha** (`mcrpc` branch). This MeshCore tree only
documents the consumer relationship.
