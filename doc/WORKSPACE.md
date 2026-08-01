# Workspace layout (MeshCore ↔ mcRPC)

Canonical documentation: **`/data/projects/README.md`**.

| Path | Role |
|------|------|
| `/data/projects/mcrpc` | Standalone library (only protocol implementation) |
| `/data/projects/meshcore` | This repository — adapter in `examples/mcrpc` |

Dependency: `lib_deps = symlink://../mcrpc` (PlatformIO local URI → in-place sibling).

```
Update mcRPC → run tests → build MeshCore → flash → repeat
```
