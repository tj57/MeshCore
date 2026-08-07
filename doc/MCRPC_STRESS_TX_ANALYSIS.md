# Stress / TX pipeline analysis

## Historical symptom (why the old burst failed)

Under an unrealistic Chat stress of **100** back-to-back requests, labs observed
roughly **~40** correlated responses and **~8** successful on-air transmissions,
with companion errors often reported as `ERR_CODE_NOT_FOUND`.

That burst is **not** the supported test methodology. Expecting 100% RF delivery
from a 100-ping flood is incorrect for MeshCore flood GRP_TXT.

## Supported methodology

See meshcore-ha [`docs/STRESS_METHODOLOGY.md`](https://github.com/tj57/meshcore-ha/blob/mcrpc/docs/STRESS_METHODOLOGY.md):

| Profile | Pattern |
|---------|---------|
| Warm-up | 5 requests, 1 s interval |
| Light load | 10 requests, 2 s interval |
| Continuous short | paced traffic for 5 minutes |
| Continuous long | paced traffic for 30 minutes |

Measure: reply success, average RTT, queue drops, packet drops, memory growth, CPU.

Saturation signals (`busy`, `queue_full`, `table_full`, backpressure) are
**envelope reports**, not unexpected protocol failures under load.

Always on the **private** channel / Config Entry path — **never Public**.

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

Diagnostics record `tx_pipeline`, `recent_traces` (`tx_ok` / `tx_error` with
`classified`, `raw_tx` / `transmitted_payload`), RTT, packet loss, pending
requests, and parser statistics.

## Root causes (code-backed)

### 1. Companion error conflation (HA-visible `ERR_CODE_NOT_FOUND`)

`CMD_SEND_CHANNEL_TXT_MSG` returned `ERR_CODE_NOT_FOUND` when **either**
invalid `channel_idx` **or** `sendGroupMessage()` failed (pool/queue full).

**Fix:** `NOT_FOUND` only for bad channel; `TABLE_FULL` when send fails.

### 2. mcRPC single-flight silent drop

Previously `getOutboundTotal() > 0` returned false with no queue.

**Fix:** reply queue depth `MCRPC_TX_QUEUE` (default 8) + counters
`tx_ok` / `tx_queued` / `tx_drop_busy` / `tx_drop_alloc` / `tx_drop_queue_full`.

### Backpressure vs retry (RC decision)

| Option | Verdict |
|--------|---------|
| Larger queue | Helps micro-bursts only |
| Infinite retry | Stale/`#id` duplicates; fights airtime |
| Counted drop when full | **Current policy** — visible in counters / HA timeouts |
| Block inbound until TX free | Risks stalling embedded RX loop |

Do **not** add blind retries in RC. When `tx_drop_queue_full` rises, slow the
requester (spacing / concurrency). Queue is backpressure signal, not a lossy
black hole without metrics.

### 3. Airtime / pool — hard RF limit

MeshCore `tx_budget` (~50% airtime by default) and pools (SensorMesh 32,
companion 16) **cap** sustained flood GRP_TXT. The operating envelope is the
paced profiles above — not 1:1 completion of a 100-burst.

### 4. HA correlation window

Timeouts, policy denials, and dedup reduce correlated responses under overload.

## Expected post-fix behavior

| Stage | Before | After |
|-------|--------|-------|
| Companion pool exhaustion | `NOT_FOUND` | `TABLE_FULL` |
| Busy radio on node | Silent drop | Queue then counted drop |
| Diagnostics | Opaque | `tx_pipeline` + classified traces |

Re-run stress with [STRESS_METHODOLOGY.md](https://github.com/tj57/meshcore-ha/blob/mcrpc/docs/STRESS_METHODOLOGY.md)
on the private channel only (Config Entry title may be `lab-channel`).
