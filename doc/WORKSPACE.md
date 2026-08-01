# Workspace layout (MeshCore ↔ mcRPC)

Canonical documentation: **`/data/projects/README.md`**.

| Path | Role |
|------|------|
| `/data/projects/mcrpc` | Standalone library (only protocol implementation) |
| `/data/projects/meshcore` | This repository — adapter in `examples/mcrpc` |

Dependency: `lib_extra_dirs = ..` + `lib_deps = mcrpc` (in-place sibling, no symlink, no vendored copy).

```
Update mcRPC → run tests → build MeshCore → flash → repeat
```
