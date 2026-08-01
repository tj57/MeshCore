# MeshCore ↔ mcRPC dependency

## Modes

| Mode | How | When |
|------|-----|------|
| **Development** | `platformio.local.ini` → `symlink://../mcrpc` | Daily work under `/data/projects` |
| **Release** | committed `[mcrpc_lib] lib = https://github.com/…#vX.Y.Z` | CI, tagged firmware, production |
| **Future** | `mcrpc @ ^1.0.0` | After PlatformIO Registry publish |

## Setup (development)

```bash
cd /data/projects/meshcore
cp platformio.local.ini.example platformio.local.ini
# already gitignored
```

## Switching to release locally

```bash
rm platformio.local.ini
# or comment out [mcrpc_lib] override
pio run -e Heltec_v3_mcrpc_button -t clean
pio run -e Heltec_v3_mcrpc_button
```

## Variable

All mcRPC firmware envs depend on `${mcrpc_lib.lib}` defined once in root
`platformio.ini` and optionally overridden by `platformio.local.ini`
(loaded via `extra_configs`).
