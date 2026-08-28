# Project provenance and origin markers

ESP32 VR Haptic Trigger V3 is the original project of **alfawalidou / McWall**.

Official identities:

- GitHub: `@alfawalidou`
- Discord: `.mcwall`
- Telegram: `@McWall07`
- Official repository: <https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3>

## Release lineage

The standalone repository was extracted from Trigger V3 development work preserved in the larger historical repository.

The original internal V3 source baseline recorded by the standalone project is:

```text
913fefff49d51f9b65072eacdb34fa90991b474a
```

The immutable physically validated standalone baseline tag is:

```text
v3.0.0-validated
```

The first public binary release is:

```text
v3.1.0
```

Release page: <https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3/releases/tag/v3.1.0>

`v3.0.0-validated` remains the historical physical-validation anchor and is not moved or reused. `v3.1.0` is the public release line that added release packaging, authorship/provenance presentation, Wokwi support, the `VR HapGunV3` splash and the current noncommercial licensing policy.

Git history, tags, validation documents, hardware photos, profile tuning and the preserved engineering archive provide chronological provenance for the project.

## Characteristic technical fingerprint

The following combination is characteristic of this Trigger V3 implementation:

```text
Bluetooth Classic SPP
ForceTubeVR 1187883197
GPIO13 = TRIGGER
GPIO14 = PROFILE / MODE
GPIO4 = unused
HAPTIC_ONLY <-> TRIGGER_FALLBACK
PISTOL / SNIPER / M16 / P90 / PKM / LASER
SNIPER rumble pulse = 120 ms
AUTO repeat tuning = 150 ms
LASER = 20 charge steps over 2600 ms
recoil forward = 30 ms
reverse = 2 ms at 25%
rumble watchdog = 500 ms
```

Individual values are not claimed to be unique by themselves. The history and combination of implementation details are useful when identifying derivatives of this project.

## Firmware origin marker

Official firmware source contains a passive provenance marker:

```text
ALFAWALIDOU_MCWALL_TRIGGER_V3_ORIGIN_2026
```

It is intentionally retained in compiled firmware images. On systems with a `strings` utility, a firmware binary can often be checked with a command similar to:

```bash
strings firmware.bin | grep -E "alfawalidou|McWall|ALFAWALIDOU_MCWALL_TRIGGER_V3_ORIGIN_2026"
```

The project also embeds readable identity strings for the author and official repository.

**This provenance marker is passive. It does not transmit data, identify users, access the network, contact a server, collect telemetry or perform any phone-home behavior.**

## Attribution expected from derivatives

Please preserve this attribution when publishing or redistributing work derived from the project for a permitted purpose:

> ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall

## Current licensing policy

Current first-party project code and documentation are licensed under **PolyForm Noncommercial License 1.0.0** (`PolyForm-Noncommercial-1.0.0`). The project is intended for personal, hobby and other noncommercial use. Commercial use, sale, resale, paid-product integration or use for commercial advantage is not licensed and requires separate prior written permission from **alfawalidou / McWall**.

Third-party dependencies keep their own licenses. Historical versions or commits previously distributed under Apache License 2.0 retain the rights already granted for those historical copies; the current noncommercial license applies prospectively to current and future first-party project versions.

See [`LICENSE`](LICENSE), [`NOTICE`](NOTICE), [`AUTHORS.md`](AUTHORS.md) and [`CITATION.cff`](CITATION.cff).
