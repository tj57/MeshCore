# Features

| Feature | Capability | Commands | Events | Status |
|---------|------------|----------|--------|--------|
| core | — | ping, status, discover, help, caps | — | done |
| battery | battery | battery, voltage, charging | battery_low | done |
| button | button | button, button_state | button.pressed (+ call) | done |
| gps | gps | gps, location, track | gps_fix | done |
| relay | relay | relay, toggle, power | relay_changed | planned |
| display | display | display, text, clear | — | planned |
| led | led | led | — | planned |
| ota | ota | ota | — | planned |
| sensors | temperature… | get temp… | — | planned |

## core

Mandatory on every mcRPC firmware. Identical behavior across boards.

## battery

Reads via `HostServices::readBattery`. Publishes `event battery_low` once when voltage crosses the low threshold (hysteresis +0.1 V to clear).

## button

Physical debounce stays in `MomentaryButton` (upstream UI helper). App calls `ButtonFeature::notifyPressed()` on click. Commands report counters/state only.

## gps

On-demand session (`OnDemandGps`) powers GPS through `EnvironmentSensorManager` (`gps` setting) and `PIN_GPS_EN` when defined. Async completion publishes a `gps …` data line and `event gps_fix`.
