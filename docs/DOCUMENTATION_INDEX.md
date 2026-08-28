# Documentation index

**ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall**  
GitHub `@alfawalidou` · Discord `.mcwall` · Telegram `@McWall07`

## Start here for a first build

- [`../START_HERE.md`](../START_HERE.md) — single-path beginner walkthrough for Windows: prerequisites, clone, wiring, COM-port detection, build, full erase, flash, Bluetooth test and `HAPTIC_ONLY` / `TRIGGER_FALLBACK` verification.

If you are building the project for the first time and do not need firmware-development details, use `START_HERE.md` and do not mix it with the manual PlatformIO paths below.

## Current public release

- [`../RELEASE_NOTES_v3.1.0.md`](../RELEASE_NOTES_v3.1.0.md) — exact v3.1.0 release assets, validation note, license and flashing-file guidance.
- [`PUBLIC_RELEASE_CHECKLIST.md`](PUBLIC_RELEASE_CHECKLIST.md) — status of the completed first public release plus remaining items for future releases.
- GitHub Release: <https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3/releases/tag/v3.1.0>

## Author, license and provenance

- [`../AUTHORS.md`](../AUTHORS.md) — original creator identity, preferred attribution and permitted-use summary.
- [`../PROJECT_PROVENANCE.md`](../PROJECT_PROVENANCE.md) — historical provenance, technical fingerprint, passive firmware origin marker and current release lineage.
- [`../CITATION.cff`](../CITATION.cff) — machine-readable citation metadata.
- [`../LICENSE`](../LICENSE) — PolyForm Noncommercial License 1.0.0 (`PolyForm-Noncommercial-1.0.0`).
- [`../NOTICE`](../NOTICE) — attribution and noncommercial-use notice.

## Simulation

- [`WOKWI_SIMULATION.md`](WOKWI_SIMULATION.md) — current `main`-branch Wokwi workflow for OLED, buttons, LEDs, PWM/recoil outputs, logic analyzer and simulated ForceTube input.
- [`../diagram.json`](../diagram.json) — virtual ESP32 Trigger V3 circuit.
- [`../wokwi.toml`](../wokwi.toml) — Wokwi firmware/ELF configuration.

The Wokwi environment is simulation-only. Real hardware must continue to use `trigger-v3-safe` or `trigger-v3-compat`.

## Current authoritative documentation

- [`ARCHITECTURE.md`](ARCHITECTURE.md) — firmware modules and runtime data flow.
- [`HARDWARE.md`](HARDWARE.md) — final physical architecture and wiring.
- [`PINOUT.md`](PINOUT.md) — final validated GPIO map, 30-pin breakout mapping and current shield SVG.
- [`BOM.md`](BOM.md) — current bill of materials.
- [`RUMBLE_WIRING.md`](RUMBLE_WIRING.md) — Xbox-style rumble motors and IRLZ44N driver notes.
- [`DESIGN.md`](DESIGN.md) — original gunstock concept images and design notes.
- [`PROFILES.md`](PROFILES.md) — final local profiles and control behavior.
- [`PROTOCOL.md`](PROTOCOL.md) — ForceTube-compatible Bluetooth protocol.
- [`FLASHING.md`](FLASHING.md) — build, erase, upload and safe serial monitor.
- [`VALIDATION.md`](VALIDATION.md) — final physical baseline validation matrix.

## Preserved original Trigger V3 engineering archive

[`reference-original/`](reference-original/) contains technical documentation and assets recovered from the earlier private development repository.

The archive preserves design reasoning, boot-health semantics, hardware-validation procedures, test plans, chronological validation logs, rumble-driver reconstruction and historical diagrams.

**Current standalone documents in `docs/` are authoritative.** Archived material may describe superseded pins, old branch names, earlier test boundaries or earlier OLED wiring. Historical content is retained for provenance, not as current build instructions.

Important known historical conflict:

```text
FINAL: GPIO13 = TRIGGER
       GPIO14 = PROFILE / MODE
       GPIO4  = UNUSED

SOME ARCHIVED MATERIAL:
       GPIO4  = PROFILE
```

For the current shield mapping, use [`PINOUT.md`](PINOUT.md), not an archived wiring diagram.
