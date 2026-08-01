# Workspace layout (MeshCore ↔ mcRPC)

Canonical documentation: **`/data/projects/README.md`**.

## Summary

| Path | Role |
|------|------|
| `/data/projects/mcrpc` | Standalone library (only protocol implementation) |
| `/data/projects/meshcore` | This repository — transport adapter in `examples/mcrpc` |

Dependency: `lib_deps = file://../mcrpc` (PlatformIO local path, no symlink).

```
Update mcRPC → run tests → build MeshCore → flash → repeat
```
