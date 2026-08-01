# mcRPC library

Transport-independent **mcRPC** protocol library and Feature SDK.

| Consumer | Role |
|----------|------|
| MeshCore firmware (`examples/mcrpc`) | One transport: MeshCore group text |
| Desktop / CLI | Same parse/build semantics |
| Home Assistant MeshCore integration | Same library for parse + build (planned) |

## Public API

```cpp
#include <mcrpc/FeatureSdk.h>   // features
#include <mcrpc/Parser.h>       // parse requests
#include <mcrpc/McRpc.h>        // full framework (optional)
#include <mcrpc/OutboundBuilder.h>  // build request/event lines
```

No Arduino, MeshCore, or radio headers are required for the core library.

## Build

### PlatformIO (embedded)

Drop this folder under `lib/mcrpc` (already the case in this repo). Firmware `#include <mcrpc/...>` and LDF links the library.

### CMake (desktop / HA)

```bash
cmake -S lib/mcrpc -B build/mcrpc
cmake --build build/mcrpc
ctest --test-dir build/mcrpc
```

### Host tests (quick)

```bash
./scripts/test-mcrpc-host.sh
```

## Layout

```
lib/mcrpc/
  src/mcrpc/           # headers + sources (include root = src/)
  CMakeLists.txt
  library.json
  README.md
```

Persistence (`ConfigStore`) and GPS power drivers are **consumer concerns** — see `examples/mcrpc/ArduinoFsConfigStore.h`.
