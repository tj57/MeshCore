# Stress / TX pipeline analysis (100 → 40 → 8)

## Symptom

Under Chat stress: **100** requests → **~40** correlated responses → **~8**
successful on-air transmissions, with companion errors reported as
`ERR_CODE_NOT_FOUND`.

## Instrumented path

```
HA Chat / meshcore.request
  → McRpcBridge._async_send_line / _async_send_answer
  → companion CMD_SEND_CHANNEL_TXT_MSG
  → sendGroupMessage → queueOutbound / radio
  → mcRPC node onGroupDataRecv → Dispatcher → publishRaw
  → McRpcMesh::sendChannelText → sendFlood
  → HA classify_inbound / correlator
```

Diagnostics now record `tx_pipeline`, `recent_traces` (`tx_ok` / `tx_error` with
`classified`, `raw_tx` / `transmitted_payload`), RTT, packet loss, pending
requests, and parser statistics.

## Root causes (code-backed)

### 1. Companion error conflation (HA-visible `ERR_CODE_NOT_FOUND`)

`CMD_SEND_CHANNEL_TXT_MSG` returned `ERR_CODE_NOT_FOUND` when **either**:

- `channel_idx` was invalid, **or**
- `sendGroupMessage()` failed (packet pool / outbound queue full)

So stress-driven pool exhaustion was mislabeled as “not found”.

**Fix:** return `ERR_CODE_NOT_FOUND` only for bad channel; return
`ERR_CODE_TABLE_FULL` when send fails (matches `CMD_SEND_CHANNEL_DATA`).

### 2. mcRPC firmware single-flight silent drop (response funnel)

`McRpcMesh::sendChannelText` previously:

```cpp
if (_mgr->getOutboundTotal() > 0) return false;
```

Under burst load, dispatch succeeded but replies were **discarded** with no
queue and no error text. That explains many handled requests with few RF
responses.

**Fix:** small outbound reply queue drained in `loopMcRpc()`, plus counters
`tx_ok` / `tx_queued` / `tx_drop_*`.

### 3. Airtime / queue limits (on-air ≈ 8)

MeshCore duty-cycle (`tx_budget`) and shared packet pool (SensorMesh pool 32,
companion pool 16) defer or drop floods. Companion may still have accepted
earlier frames as OK while later ones fail with `TABLE_FULL`. This matches a
small number of completed RF transmissions during a 100-burst.

### 4. HA correlation window

Timeouts, policy denials, and dedup reduce correlated “responses” below TX
attempts (100 → ~40) even when some RF traffic occurred.

## Expected post-fix behavior

| Stage | Before | After |
|-------|--------|-------|
| Companion pool exhaustion | `ERR_CODE_NOT_FOUND` | `ERR_CODE_TABLE_FULL` |
| Busy radio on mcRPC node | Silent reply drop | Queued (up to 8) then counted drop |
| Diagnostics | Opaque errors | `tx_pipeline` + classified traces |

Re-run stress on **mcCtrl only**; download diagnostics and confirm
`tx_errors_table_full` vs `tx_errors_not_found` split.
