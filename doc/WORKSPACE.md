# Workspace layout (MeshCore ↔ mcRPC)

Canonical: **`/data/projects/README.md`**.

| Path | Role |
|------|------|
| `/data/projects/mcrpc` | Library |
| `/data/projects/meshcore` | This repo — `examples/mcrpc` adapter |

**Dev:** `platformio.local.ini` → `symlink://../mcrpc`  
**Release:** committed `[mcrpc_lib]` GitHub pin  

See `doc/MCRPC_DEPENDENCY.md`.
