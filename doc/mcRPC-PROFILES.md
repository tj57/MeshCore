# mcRPC Profiles

Profiles are soft contracts: they suggest which features a node should enable. They do not change the parser.

| Profile | Typical board | Features | Default name |
|---------|---------------|----------|--------------|
| `switch` | Heltec V3 | core, battery, button | `button` |
| `tracker` | LW010 / WisMesh Tag | core, battery, button, gps | `tracker` |
| `sensor` | any | core, battery, (+ sensors later) | `node` |
| `relay` | future | core, battery, relay | `relay` |
| `display` | future | core, display | `display` |
| `gateway` | future companion bridge | core + HA-facing helpers | *(identity name — not RF role)* |

Set via `MCRPC_DEFAULT_PROFILE` at build time or `ConfigPrefs.profile` in `/mcrpc_cfg`.
UI tag may mirror profile (`tag=`); **never** use tag/profile as an RF address (RFC-0001).

## Lab channel (do not commit secrets)

Private channel name + PSK for on-air tests live in **gitignored**
`platformio.secrets.ini` (see `platformio.secrets.ini.example`).

Typical HA QA channel: name `mcCtrl`, PSK as **32 hex chars** (`MCRPC_DEFAULT_PSK_HEX`).
Use `MCRPC_FORCE_CHANNEL_DEFAULTS=1` when flashing so `/mcrpc_cfg` cannot keep an old secret.

## Profile → commands

### switch (Heltec button)

Mandatory: `ping status discovery help caps`  
Feature: `button button_state battery voltage charging`

Events on the configured private channel only:

- `event button_down` — press edge
- `event button_up count=N` — release edge
- `event button_pressed count=N` — short click (compat)
- `event battery_low`

### tracker (LW010 GPS)

Mandatory + `gps location track battery voltage charging button button_state`

Events:

- `event button_down` / `event button_up` / `event button_pressed`
- `event gps_fix` (after on-demand fix) + `gps lat=… lon=…` data line
- `event gps_nofix` / `err gps_no_fix` on timeout
- `event battery_low`

Button **down** starts an on-demand GPS session. GPS is powered only while waiting
for a fix (or until timeout), then powered off again.
