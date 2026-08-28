# Documentation index

**ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall**  
GitHub `@alfawalidou` · Discord `.mcwall` · Telegram `@McWall07`

## Start here for a first build

- [`../START_HERE.md`](../START_HERE.md) - **single-path beginner walkthrough** for Windows: prerequisites, clone, wiring, COM-port detection, build, full erase, flash, Bluetooth test and `HAPTIC_ONLY` / `TRIGGER_FALLBACK` verification

If you are building the project for the first time and do not need firmware-development details, use `START_HERE.md` and do not mix it with the manual PlatformIO paths below.

## Author, license and provenance

- [`../AUTHORS.md`](../AUTHORS.md) - original creator identity and preferred attribution
- [`../PROJECT_PROVENANCE.md`](../PROJECT_PROVENANCE.md) - historical provenance, technical fingerprint and passive firmware origin marker
- [`../CITATION.cff`](../CITATION.cff) - machine-readable citation metadata for GitHub and research/publication use
- [`../LICENSE`](../LICENSE) - Apache License 2.0
- [`../NOTICE`](../NOTICE) - redistribution attribution notice

## Simulation

- [`WOKWI_SIMULATION.md`](WOKWI_SIMULATION.md) - Wokwi setup for OLED, buttons, LEDs, PWM/recoil outputs, logic analyzer and simulated ForceTube input through the Serial Terminal
- [`../diagram.json`](../diagram.json) - virtual ESP32 Trigger V3 circuit
- [`../wokwi.toml`](../wokwi.toml) - Wokwi firmware/ELF configuration

The Wokwi environment is simulation-only. Real hardware must continue to use `trigger-v3-safe` or `trigger-v3-compat`.

## Current authoritative documentation

- `ARCHITECTURE.md` - firmware modules and runtime data flow
- `HARDWARE.md` - final physical architecture and wiring
- `PINOUT.md` - final validated GPIO map and 30-pin breakout terminal mapping
- `BOM.md` - current bill of materials
- `RUMBLE_WIRING.md` - Xbox-style rumble motors and IRLZ44N driver notes
- `DESIGN.md` - original gunstock concept images and design notes
- `PROFILES.md` - final local profiles and control behavior
- `PROTOCOL.md` - ForceTube-compatible Bluetooth protocol
- `FLASHING.md` - build, erase, upload and safe serial monitor
- `VALIDATION.md` - final physical validation matrix
- `PUBLIC_RELEASE_CHECKLIST.md` - tasks before a future public release

## Preserved original Trigger V3 engineering archive

`reference-original/` contains the original V3 technical documentation and assets recovered from the old private repository.

The archive preserves boot-health semantics, design reasoning, hardware validation procedures, test plans, chronological validation logs, rumble-driver reconstruction and original diagrams.

When an archived value conflicts with current documentation, the current standalone documentation is authoritative.

Important known historical conflict:

```text
FINAL: GPIO13 = TRIGGER
       GPIO14 = PROFILE / MODE
       GPIO4  = UNUSED

SOME OLD DIAGRAMS:
       GPIO4  = PROFILE
```
