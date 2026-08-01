# mcRPC Specification v1 (Draft)

**Status:** Draft

---

# 1. Introduction

mcRPC (MeshCore Remote Procedure Call) is a lightweight, text-based application protocol operating over MeshCore.

The protocol is designed for:

- Home Assistant
- Embedded devices
- Android/iOS applications
- CLI tools
- Manual use by humans

The protocol defines only the application layer.

Routing, encryption, transport reliability and mesh forwarding are provided by MeshCore.

---

# 2. Design Goals

Implementations MUST satisfy:

- Human readable
- Small packets
- Easy parsing
- Stateless
- Transport independent
- Backward compatible

The protocol MUST be usable without a dedicated application.

Every command should be possible to type manually.

---

# 3. Layers

mcRPC intentionally separates responsibilities.

```
Application
↑
mcRPC
↑
MeshCore
↑
Radio
```

MeshCore provides:

- encryption
- routing
- packet forwarding

mcRPC provides:

- addressing
- commands
- responses
- events

---

# 4. Grammar

ABNF (RFC5234 style)

```abnf
message     = target [request] SP command *(SP argument)

target      = identifier / "all" / "self" / group

group       = "group:" identifier

request     = "#" 1*DIGIT

command     = identifier

argument    = token

identifier  = 1*(ALPHA / DIGIT / "_" / "-")

token       = 1*(VCHAR)
```

Parser MUST ignore:

- repeated spaces
- leading spaces
- trailing spaces

Commands are case insensitive.

Arguments preserve original case.

---

# 5. Addressing

Examples

```
ha
tracker
relay1
sensor2
display
```

Reserved names

```
all
self
group:<name>
```

Examples

```
all ping

group:gps status

self reboot
```

---

# 6. Request ID

Optional.

```
ha ping
```

↓

```
ha pong
```

or

```
ha#42 ping
```

↓

```
ha#42 pong
```

Responses MUST preserve Request ID.

---

# 7. Core Commands (Mandatory)

Every mcRPC device MUST implement:

```
ping
status
discover
help
caps
```

Meaning

| Command | Description |
|----------|-------------|
| ping | Connectivity test |
| status | Current state |
| discover | Basic information |
| help | Supported commands |
| caps | Supported capabilities |

This guarantees interoperability.

---

# 8. Standard Commands (Optional)

Read

```
get battery

get temp

get version

get gps
```

Modify

```
set led on

set relay off

set interval 300
```

Execute

```
gps

reboot

ota

reset

beep
```

---

# 9. Responses

Success

```
ok
```

Failure

```
err timeout

err busy

err denied

err unsupported

err unknown_command

err invalid_argument

err internal
```

Data

```
battery value=97

temp value=24.1

gps lat=50.12 lon=19.93 acc=4 sat=18
```

Structured data MUST use:

```
key=value
```

---

# 10. Events

Devices MAY send asynchronous events.

Examples

```
event battery_low

event panic

event motion

event gps_fix

event button_pressed
```

Events never require requests.

---

# 11. Device Profiles

Every device SHOULD declare a profile.

Examples

```
gateway

tracker

relay

sensor

display

beacon

camera

weather

environment

lighting

switch

energy
```

Profiles define additional commands.

---

# 12. Discovery

```
all discover
```

↓

```
ha profile=gateway fw=2026.8

tracker profile=tracker fw=1.2

relay1 profile=relay fw=1.0
```

---

# 13. Status

Minimum required fields

```
status

name=

profile=

fw=

uptime=

rssi=
```

Recommended

```
battery=

mesh=

voltage=

temp=
```

---

# 14. Capabilities

Example

```
caps

gps

battery

ota

led

relay

temperature
```

Capabilities MUST be one per line.

---

# 15. Standard Error Codes

```
timeout

busy

denied

unsupported

unknown_command

invalid_argument

internal

gps_no_fix

low_battery
```

Response format

```
err timeout
```

---

# 16. Human Interface

Protocol SHOULD remain comfortable for humans.

Good

```
tracker gps

ha sw1 on

gate open
```

Bad

```
rpc.execute.device.command(...)
```

---

# 17. Reserved Keywords

Reserved

```
all

self

group

event

ok

err

status

discover

caps

help

ping

get

set
```

Applications MUST NOT redefine these.

---

# 18. Forward Compatibility

Unknown commands

↓

```
err unsupported
```

Unknown fields

↓

Ignored.

Future fields MUST NOT break existing parsers.

---

# 19. Device Profiles

## Gateway

Recommended

```
status

discover

caps

ping

scene

automation

entity
```

---

## Tracker

Recommended

```
gps

battery

sleep

wake

reboot
```

---

## Relay

Recommended

```
relay

status

power

energy
```

---

## Sensor

Recommended

```
temp

humidity

pressure

battery
```

---

# 20. Example Session

```
User

ha ping

↓

ha pong
```

```
tracker#18 gps

↓

tracker#18 gps lat=50.12 lon=19.93 acc=4 sat=18
```

```
all discover

↓

ha profile=gateway fw=2026.8

tracker profile=tracker fw=1.2

relay1 profile=relay fw=1.0
```

```
relay1 set relay on

↓

ok
```

---

# 21. Implementation Requirements

Every compliant implementation MUST:

✓ support UTF-8

✓ support Core Commands

✓ preserve Request ID

✓ ignore unknown fields

✓ ignore repeated whitespace

✓ return standard errors

✓ support key=value format

✓ never require JSON

✓ never require XML

✓ never depend on transport layer features
---

# 22. MeshCore Transport Binding (Implementation)

This section is informational and does not change the application grammar.

mcRPC messages travel as MeshCore `PAYLOAD_TYPE_GRP_TXT` on a private group channel:

1. Channel name + 16-byte PSK configured in `/mcrpc_cfg` (or build defaults)
2. Channel hash = first byte of SHA-256(PSK)
3. Wire payload: `timestamp(4) | TXT_TYPE_PLAIN | "NodeName: <mcRPC line>"`

Inbound handlers strip the MeshCore `NodeName: ` prefix before parsing.

Outbound replies are published the same way. When a request id is present, the mcRPC body is `#<id> <response>` (after the MeshCore sender prefix). Example on air:

```
tracker: #18 gps lat=50.12 lon=19.93
```

After sender-prefix strip, consumers see `#18 gps lat=…`, which preserves correlation.

## Listening scope

Firmware decrypts only the configured channel (via `searchChannelsByHash`). Other channels are ignored.

## Home Assistant

HA is a peer that joins the same channel (companion app, MQTT bridge, etc.). The firmware never embeds MQTT/HTTP.

---

# 23. Proposed protocol improvements (not implemented)

Documented for discussion — **not silently applied**:

| Idea | Benefit | Cost | Status |
|------|---------|------|--------|
| Emit bare mcRPC without MeshCore `Sender:` prefix | Cleaner lines | Breaks chat UX in companion apps | deferred |
| Multi-channel listen list | Gateways | More flash + config | deferred |
| `err unsupported` vs `unknown_command` nuance for aliases | Spec clarity | Minor | open |
| Binary TLV companion codec | Efficiency | Violates "human readable" goal | rejected |

