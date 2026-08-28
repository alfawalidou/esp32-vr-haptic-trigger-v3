# ESP32 VR Haptic Trigger V3

**Created and maintained by [@alfawalidou](https://github.com/alfawalidou) / McWall**  
Discord: `.mcwall` · Telegram: `@McWall07`

> If you reuse, fork, modify, redistribute, demonstrate or publish work based on this project, please preserve the original attribution: **ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall**.

> **Personal / noncommercial use only:** the current first-party project is licensed under the **PolyForm Noncommercial License 1.0.0** (`PolyForm-Noncommercial-1.0.0`). Commercial use, sale, resale, incorporation into a paid product or service, or use for commercial advantage is **not licensed**. Any commercial use requires separate prior written permission from **alfawalidou / McWall**. See [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE).

<!-- AUTO REF 01 -->
![](ref/01.jpg)

![Trigger V3 concept gunstock](assets/design/trigger-v3-concept-gunstock.png)

Standalone, hardware-validated ESP32 firmware for a DIY VR haptic gun / gunstock controller using Bluetooth Classic SPP and a ForceTube-compatible haptic protocol.

This repository contains **Trigger V3 only**. Historical V1/V2 firmware variants are not part of this standalone project.

> Not affiliated with ProTubeVR. `ForceTube` is referenced only to describe protocol compatibility observed during development.

> ## New to ESP32, Git or PlatformIO?
>
> **Start here: [`START_HERE.md`](START_HERE.md)**
>
> It is the single recommended Windows beginner path from prerequisites -> cloning -> wiring -> COM-port detection -> build -> full erase -> flash -> Bluetooth test -> `HAPTIC_ONLY` / `TRIGGER_FALLBACK` test.
>
> If your goal is simply to build and use the haptic gun, follow `START_HERE.md` from top to bottom and ignore the advanced/manual build commands until you need troubleshooting or development details.

## Validated baseline

- Firmware tag: `v3.0.0-validated`
- Original internal V3 source baseline: `913fefff49d51f9b65072eacdb34fa90991b474a`
- Target: classic ESP32 with Bluetooth Classic SPP
- Runtime communication: **Bluetooth only - no Wi-Fi**
- Final controls: GPIO13 TRIGGER + GPIO14 PROFILE/MODE
- GPIO4: unused/free
- OLED: SSD1306 128x64 at I2C address `0x3C`
- **OLED VCC: strongly recommended and validated from the external regulated 5 V buck output, not from the ESP32 3V3 pin**
- Recoil: BTS7960 + solenoid
- Rumble: GPIO17 PWM through an external MOSFET driver stage
- Status LEDs: two WS2812B on GPIO16

The final COMPAT build was physically validated with OLED, buttons, local profiles, Bluetooth/APK haptics, Bluetooth disconnect/reconnect, APK restart and physical reset.

> **Important OLED power note:** on the validated prototype, routing OLED VCC from the ESP32 `3V3` pin caused severe instability under haptic operation. Moving OLED VCC to the regulated external **5 V buck output** removed the fault during repeated autonomous testing. For the exact OLED module tested, SDA and SCL remained at about 3.2 V while VCC was 5 V. If you use a different OLED breakout, verify its allowed VCC range and confirm that its I2C pull-ups do not drive SDA/SCL above the ESP32 3.3 V logic level.

## Design concept

![Trigger V3 design idea](assets/design/trigger-v3-design-idea.png)

These original Trigger V3 concept images were recovered from the development repository. They are design references, not final mechanical dimensions.

See [docs/DESIGN.md](docs/DESIGN.md).

## Runtime architecture

```text
Quest 3 / Android haptic application
                |
        Bluetooth Classic SPP
                |
        ForceTube packet parser
                |
         HapticController
          /           \
 GPIO17 rumble      BTS7960
      |               |
 MOSFET stage      recoil solenoid
      |
 rumble motors

GPIO13 trigger -> TRIGGER_FALLBACK -> selected local profile
GPIO14 short   -> next profile
GPIO14 long    -> HAPTIC_ONLY / TRIGGER_FALLBACK
```

The Bluetooth and local-trigger paths share the same `HapticController`. Overlapping non-zero KICK requests are inhibited instead of queued and replayed late.

## Final pinout

| Function | ESP32 GPIO | Notes |
|---|---:|---|
| Trigger | 13 | active LOW, `INPUT_PULLUP` |
| Profile / mode button | 14 | short = next profile; long ~1 s = mode toggle |
| Rumble PWM | 17 | 175 Hz, 8-bit |
| BTS7960 RPWM | 23 | recoil control |
| BTS7960 LPWM | 5 | recoil control |
| WS2812B data | 16 | two LEDs |
| Buzzer | 27 | optional feedback |
| OLED SDA | 21 | I2C, 3.3 V logic |
| OLED SCL | 22 | I2C, 3.3 V logic |
| OLED VCC | — | **external regulated 5 V buck output strongly recommended; do not use ESP32 3V3 on the validated prototype** |
| Free / unused | 4 | intentionally unused |

Buttons connect between their GPIO and GND and use internal pull-ups.

See [docs/PINOUT.md](docs/PINOUT.md) for the exact 30-pin breakout terminal mapping.

## Manual profiles

| Profile | Behavior | Kick | Rumble | Timing |
|---|---|---:|---:|---|
| PISTOL | SINGLE | 120 | 0 | no rumble |
| SNIPER | SINGLE | 255 | 128 | short 120 ms rumble pulse |
| M16 | AUTO | 240 | 125 | 150 ms repeat |
| P90 | AUTO | 220 | 159 | 150 ms repeat |
| PKM | AUTO | 129 | 255 | 150 ms repeat |
| LASER | CHARGE_RELEASE | 255 | 0 to 255 | 20 steps over 2600 ms, then KICK and rumble stop |

SNIPER rumble stops automatically after about 120 ms even if the trigger remains held. M16, P90 and PKM keep their manual rumble active while the trigger is held.

## ForceTube-compatible Bluetooth protocol

```text
byte 0 = 0x2A
byte 1 = 0xB0
byte 2 = 0x00 KICK / 0x01 RUMBLE
byte 3 = intensity 0..255
```

Bluetooth name:

```text
ForceTubeVR 1187883197
```

See [docs/PROTOCOL.md](docs/PROTOCOL.md).

## Rumble hardware

![Xbox rumble wiring overview](assets/hardware/xbox-rumble-wiring-overview.png)

The detailed original wiring reconstruction is preserved and indexed in [docs/RUMBLE_WIRING.md](docs/RUMBLE_WIRING.md).

## Build — developer / manual reference

**First-time users should use [`START_HERE.md`](START_HERE.md) instead of this section.**

Safe build:

```powershell
pio run -e trigger-v3-safe
```

Physical actuator build:

```powershell
pio run -e trigger-v3-compat
```

Before a manual physical flash, read [docs/FLASHING.md](docs/FLASHING.md).

The beginner helper used by `START_HERE.md` is:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\first-flash.ps1
```

It guides the user through COM-port selection and wiring confirmation, then calls the validated COMPAT workflow: clean -> build -> full erase -> upload.

For diagnostics, use the safe serial-monitor helper:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\monitor.ps1 -Port COMx
```

The monitor helper always uses 115200 baud with RTS and DTR inactive.

## Author, attribution and provenance

ESP32 VR Haptic Trigger V3 is the original project of **alfawalidou / McWall**.

- GitHub: [@alfawalidou](https://github.com/alfawalidou)
- Discord: `.mcwall`
- Telegram: `@McWall07`
- Official repository: <https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3>

The firmware embeds readable project identity strings plus a passive provenance marker. It does **not** perform tracking, phone-home behavior or network telemetry for attribution. See [`PROJECT_PROVENANCE.md`](PROJECT_PROVENANCE.md).

Current first-party project code and documentation are licensed under **PolyForm Noncommercial License 1.0.0** (`PolyForm-Noncommercial-1.0.0`). Personal, hobby and other noncommercial uses are permitted according to the license. Commercial use is not granted; any commercial use requires separate prior written permission from **alfawalidou / McWall**. See [`LICENSE`](LICENSE), [`NOTICE`](NOTICE), [`AUTHORS.md`](AUTHORS.md) and [`CITATION.cff`](CITATION.cff).

Third-party libraries and dependencies keep their own licenses. Historical versions or commits that were already distributed under Apache License 2.0 retain the rights previously granted for those historical copies; the current noncommercial licensing policy applies prospectively to current and future first-party project versions.

## Documentation

- **[Beginner start-to-finish walkthrough](START_HERE.md)**
- [Author and attribution](AUTHORS.md)
- [Project provenance and origin markers](PROJECT_PROVENANCE.md)
- [Documentation index](docs/DOCUMENTATION_INDEX.md)
- [Firmware architecture](docs/ARCHITECTURE.md)
- [Final hardware and wiring](docs/HARDWARE.md)
- [Final pinout and breakout mapping](docs/PINOUT.md)
- [Bill of materials](docs/BOM.md)
- [Rumble / IRLZ44N wiring](docs/RUMBLE_WIRING.md)
- [Mechanical and visual design](docs/DESIGN.md)
- [Profiles and local controls](docs/PROFILES.md)
- [ForceTube-compatible protocol](docs/PROTOCOL.md)
- [Build, erase, flash and serial monitor](docs/FLASHING.md)
- [Validated test matrix](docs/VALIDATION.md)
- [Original Trigger V3 engineering archive](docs/reference-original/README.md)
- [Future public-release checklist](docs/PUBLIC_RELEASE_CHECKLIST.md)

## Repository layout

```text
.
|-- START_HERE.md
|-- AUTHORS.md
|-- PROJECT_PROVENANCE.md
|-- CITATION.cff
|-- LICENSE
|-- NOTICE
|-- include/
|   |-- BootHealth.h
|   |-- ForceTubeProtocol.h
|   |-- HapticController.h
|   |-- HapticProfiles.h
|   |-- ProjectIdentity.h
|   `-- TriggerV3Config.h
|-- src/
|   |-- BootHealth.cpp
|   |-- HapticController.cpp
|   `-- main.cpp
|-- assets/
|   |-- design/
|   `-- hardware/
|-- docs/
|   |-- reference-original/
|   `-- ...
|-- scripts/
|   |-- first-flash.ps1
|   |-- flash.ps1
|   |-- build.ps1
|   `-- monitor.ps1
|-- platformio.ini
|-- CHANGELOG.md
`-- VERSION
```
