# Private channels (MeshCore native)

mcRPC does **not** invent a second crypto/channel stack. It reuses MeshCore group datagrams.

## Channel selection

Configured in `mcrpc::Config` (`/mcrpc_cfg`):

| Field | Meaning |
|-------|---------|
| `channel_name` | Logical name (e.g. `mych`) — used for `group:<name>` addressing |
| `channel_psk[16]` | Raw shared secret (ASCII or binary) |
| `listen_enabled` | If 0, inbound GRP_TXT is ignored |

Build flags:

| Flag | Meaning |
|------|---------|
| `MCRPC_DEFAULT_CHANNEL` | Channel name string (e.g. `mcCtrl`) |
| `MCRPC_DEFAULT_PSK` | Optional ASCII ≤16 bytes (legacy lab) |
| `MCRPC_DEFAULT_PSK_HEX` | **Preferred**: 32 hex chars → 16 raw bytes (same form as HA / companion) |
| `MCRPC_FORCE_CHANNEL_DEFAULTS` | If `1`, overwrite `/mcrpc_cfg` channel+PSK from build flags each boot |

**Secrets:** put real `MCRPC_DEFAULT_PSK_HEX` only in gitignored `platformio.secrets.ini`
(copy from `platformio.secrets.ini.example`). Never commit PSKs.

Default seed once unless `MCRPC_FORCE_CHANNEL_DEFAULTS=1`; after first boot without force,
`/mcrpc_cfg` is the source of truth.

## PSK handling

Same as companion / `BaseChatMesh::addChannel`:

1. Store 16-byte (or conceptually 32-byte) secret
2. `mesh::Utils::sha256(hash, secret)` → 1-byte channel hash on the wire
3. Encrypt/MAC via `Utils::encryptThenMAC` / `MACThenDecrypt`

There is **no join handshake**. Knowing the PSK is membership.

## Packet filtering

`McRpcMesh::searchChannelsByHash` returns the configured `GroupChannel` only when the inbound hash matches. MeshCore then tries decrypt; MAC failure rejects impostors/collisions.

`onGroupDataRecv` accepts only `PAYLOAD_TYPE_GRP_TXT` + plain text type, then passes the payload body to `McRpc::handleIncomingText`.

## Configuration flow

```
Build defaults (MCRPC_DEFAULT_*)
        ↓
Config::setDefaults()
        ↓
Config::begin(fs) → load /mcrpc_cfg or save defaults
        ↓
McRpcMesh::rebuildChannel() → secret + sha256 hash
        ↓
listen on that channel only
```

Admin password / NodePrefs (`/com_prefs`) remain MeshCore’s — orthogonal to the mcRPC group PSK.

## Home Assistant

HA (or a bridge) must join the **same** channel name + PSK (e.g. via MeshCore companion). Firmware never embeds MQTT.
