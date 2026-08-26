# ForceTube-compatible Bluetooth protocol

The firmware exposes a Bluetooth Classic SPP device:

```text
ForceTubeVR 1187883197
```

Runtime networking is Bluetooth only; Wi-Fi is not used.

## Packet format

Each command is four bytes:

| Byte | Value / meaning |
|---:|---|
| 0 | `0x2A` |
| 1 | `0xB0` |
| 2 | channel |
| 3 | intensity `0..255` |

Channels:

```text
0x00 = KICK
0x01 = RUMBLE
```

Examples:

```text
2A B0 00 FF  -> KICK 255
2A B0 01 7D  -> RUMBLE 125
2A B0 01 00  -> RUMBLE 0
```

## Runtime handling

- SPP input is drained continuously with a bounded byte budget per main loop.
- A non-zero KICK received while a recoil cycle is already active is **inhibited** instead of queued.
- KICK=0 is non-destructive and heavily suppressed in serial telemetry.
- Rumble PWM writes are coalesced/bounded to avoid excessive LEDC updates.
- Rumble has a 500 ms watchdog.
- Bluetooth disconnect resets the parser and immediately stops outputs.

## Observed LASER stream

The APK LASER effect validated against this firmware uses a ramp similar to:

```text
RUMBLE 12
RUMBLE 25
...
RUMBLE 242
RUMBLE 255
KICK   255
RUMBLE 0
```

The local LASER profile reproduces that concept with 20 charge steps over 2600 ms.

> Protocol compatibility was established empirically during project development. This repository is not affiliated with the original commercial product vendor.