# mcRPC Profiles

Profiles are soft contracts: they suggest which features a node should enable. They do not change the parser.

| Profile | Typical board | Features | Default name |
|---------|---------------|----------|--------------|
| `switch` | Heltec V3 | core, battery, button | `button` |
| `tracker` | LW010 / WisMesh Tag | core, battery, button, gps | `tracker` |
| `sensor` | any | core, battery, (+ sensors later) | `node` |
| `relay` | future | core, battery, relay | `relay` |
| `display` | future | core, display | `display` |
| `gateway` | future companion bridge | core + HA-facing helpers | `ha` |

Set via `MCRPC_DEFAULT_PROFILE` at build time or `ConfigPrefs.profile` in `/mcrpc_cfg`.

## Profile → commands

### switch (Heltec button)

Mandatory: `ping status discover help caps`  
Feature: `button button_state battery voltage charging`

Events: `event button_pressed`, `event battery_low`

### tracker (LW010 GPS)

Mandatory + `gps location track battery voltage charging button button_state`

Events: `event gps_fix`, `event button_pressed`, `event battery_low`

Button press on tracker also starts an on-demand GPS session.
