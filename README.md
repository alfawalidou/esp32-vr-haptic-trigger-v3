# ESP32 VR Haptic Trigger V3

Standalone, hardware-validated ESP32 firmware for a DIY VR haptic gun / stock controller using Bluetooth Classic SPP and a ForceTube-compatible four-byte haptic protocol.

This repository intentionally contains **Trigger V3 only**. Historical prototypes, V1/V2 variants, recovery experiments, internal handoff notes and stale wiring concepts are excluded.

> Not affiliated with ProTubeVR. `ForceTube` is referenced only to describe protocol compatibility observed during development.

## Validated baseline

- Standalone source snapshot: `v3.0.0-validated`
- Original internal development snapshot: `913fefff49d51f9b65072eacdb34fa90991b474a`
- ESP32 target: classic ESP32 with Bluetooth Classic SPP
- Runtime networking: **Bluetooth only; no Wi-Fi**
- OLED: SSD1306 128Ã—64, I2C address `0x3C`
- Physical actuators: recoil solenoid + rumble motor stage
- Local controls: trigger + one profile/mode button
- PlatformIO environments:
  - `trigger-v3-safe`: actuator output disabled
  - `trigger-v3-compat`: physical actuator output enabled

The validated COMPAT firmware passed local trigger/profile tests, APK haptic tests, Bluetooth OFF/ON, APK kill/reopen, physical reset, OLED operation and sustained mixed haptic traffic.

## Features

- Bluetooth Classic SPP server named `ForceTubeVR 1187883197`
- Streaming ForceTube-compatible packet parser
- Non-blocking recoil state machine
- Rumble PWM with watchdog and bounded hardware update rate
- OLED status UI
- WS2812B status LEDs
- Trigger fallback mode for standalone/local firing
- One-button profile selection and mode toggle
- Six manual profiles: PISTOL, SNIPER, M16, P90, PKM, LASER
- Boot health diagnostics
- Immediate output shutdown on Bluetooth disconnect
- Overlapping non-zero KICK requests are inhibited instead of queued/replayed

## Pinout

| Function | ESP32 GPIO |
|---|---:|
| Trigger | 13 |
| Profile / mode button | 14 |
| Rumble PWM | 17 |
| BTS7960 RPWM | 23 |
| BTS7960 LPWM | 5 |
| WS2812B data | 16 |
| Buzzer | 27 |
| OLED SDA | 21 |
| OLED SCL | 22 |
| Free / unused | 4 |

Buttons are active LOW with `INPUT_PULLUP`: switch between GPIO and GND.

## Manual profiles

| Profile | Behavior | Kick | Rumble | Timing |
|---|---|---:|---:|---|
| PISTOL | SINGLE | 120 | 0 | no rumble |
| SNIPER | SINGLE | 255 | 128 | short 120 ms rumble pulse |
| M16 | AUTO | 240 | 125 | 150 ms repeat |
| P90 | AUTO | 220 | 159 | 150 ms repeat |
| PKM | AUTO | 129 | 255 | 150 ms repeat |
| LASER | CHARGE_RELEASE | 255 | 0â†’255 | 20 steps over 2600 ms, then KICK, then rumble stop |

Short press on GPIO14 selects the next profile. Long press (~1 s) toggles `HAPTIC_ONLY` / `TRIGGER_FALLBACK`.

## Quick build

Safe build:

```powershell
pio run -e trigger-v3-safe
```

Physical-actuator build:

```powershell
pio run -e trigger-v3-compat
```

See [docs/FLASHING.md](docs/FLASHING.md) before flashing physical hardware.

## Documentation

- [Hardware and wiring](docs/HARDWARE.md)
- [Bill of materials](docs/BOM.md)
- [Profiles and local controls](docs/PROFILES.md)
- [ForceTube-compatible protocol](docs/PROTOCOL.md)
- [Firmware architecture](docs/ARCHITECTURE.md)
- [Build, erase, flash and serial monitor](docs/FLASHING.md)
- [Validated test matrix](docs/VALIDATION.md)
- [Future public-release checklist](docs/PUBLIC_RELEASE_CHECKLIST.md)

## Repository layout

```text
.
â”œâ”€â”€ include/
â”‚   â”œâ”€â”€ BootHealth.h
â”‚   â”œâ”€â”€ ForceTubeProtocol.h
â”‚   â”œâ”€â”€ HapticController.h
â”‚   â”œâ”€â”€ HapticProfiles.h
â”‚   â””â”€â”€ TriggerV3Config.h
â”œâ”€â”€ src/
â”‚   â”œâ”€â”€ BootHealth.cpp
â”‚   â”œâ”€â”€ HapticController.cpp
â”‚   â””â”€â”€ main.cpp
â”œâ”€â”€ docs/
â”œâ”€â”€ scripts/
â”œâ”€â”€ platformio.ini
â”œâ”€â”€ CHANGELOG.md
â””â”€â”€ VERSION
```

## Safety

The ESP32 GPIOs are logic/control signals only. Do not power the recoil solenoid or rumble motors directly from GPIO pins.

The validated prototype uses external high-current stages, a common reference ground, transient protection and separate actuator power distribution. Verify your exact supply voltage, current capability, wiring, cooling, fuse/protection and mechanical mounting before energizing a rebuild.

## License

No open-source license is selected yet. Before making this repository public, choose and add the intended license; see `docs/PUBLIC_RELEASE_CHECKLIST.md`.