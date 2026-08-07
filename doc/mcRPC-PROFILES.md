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

Mandatory: `ping status discovery help caps call`  
Feature: `button button_state battery voltage charging`

Hardware (Heltec WiFi LoRa 32 V3):

- OLED SSD1306 (enabled via `DISPLAY_CLASS`)
- **PRG** = `PIN_USER_BTN` (GPIO 0) → button id **1**
- Optional external button on **`PIN_USER_BTN2`** (default GPIO **4**, GND when pressed) → button id **2**

Events on the configured private channel only (RFC-0002 dotted names):

- `event button.pressed count=N` / `event button.down` / `event button.up count=N` (btn 1)
- `… id=2 …` variants for btn 2
- `event battery.low`

Optional RPC to HA (host-configured target):

```text
ha call button.pressed count=4
→ ok
```

### tracker (LW010 GPS)

Mandatory + `gps location track battery voltage charging button button_state`

Events:

- `event button.down` / `event button.up` / `event button.pressed`
- `event gps.fix` (after on-demand fix) + `gps lat=… lon=…` data line
- `event gps.nofix` / `err gps_no_fix` on timeout
- `event battery.low`

Button **down** starts an on-demand GPS session. GPS is powered only while waiting
for a fix (or until timeout), then powered off again.
