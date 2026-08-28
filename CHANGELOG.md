# Changelog

## v3.1.0 — 2026-08-28

First public binary release preparation for the standalone Trigger V3 project.

Highlights:

- version bumped from `3.0.0-validated` to `3.1.0` for the public release line;
- new `VR HapGunV3` OLED boot identity splash;
- authorship and provenance metadata consolidated under **alfawalidou / McWall**;
- Wokwi simulation environment added for OLED/UI and simulated ForceTube input testing;
- current first-party project code and documentation moved to **PolyForm Noncommercial License 1.0.0**;
- planned downloadable SAFE, COMPAT and complete/factory flash binaries with SHA-256 checksums.

The physical firmware behavior remains based on the previously validated Trigger V3 baseline. The new OLED splash was visually validated in Wokwi before this release preparation.

## Noncommercial licensing policy — 2026-08-28

The current project is now licensed under **PolyForm Noncommercial License 1.0.0** (`PolyForm-Noncommercial-1.0.0`).

- Personal, hobby, research, testing and other noncommercial uses are permitted under the license terms.
- Commercial use, sale, resale, paid-product integration or use for commercial advantage is **not licensed** by the project license.
- Any commercial use requires a separate prior written license from **alfawalidou / McWall**.
- First-party source files and build scripts now use the SPDX identifier `PolyForm-Noncommercial-1.0.0`.
- Previously distributed commits or tags that were made available under Apache License 2.0 remain subject to the rights already granted for those historical versions; the new license applies prospectively to the current and future project versions.

Third-party libraries and dependencies keep their own licenses.

## Authorship and provenance pass — 2026-08-28

Project identity is now consistently recorded as **alfawalidou / McWall**.

Added:

- at that stage, Apache License 2.0 and `NOTICE` attribution (later superseded for current/future first-party project versions by PolyForm Noncommercial License 1.0.0 as recorded above);
- `AUTHORS.md` and GitHub-compatible `CITATION.cff`;
- `PROJECT_PROVENANCE.md` with the historical source baseline, characteristic technical fingerprint and passive firmware origin marker;
- `include/ProjectIdentity.h` with project, author, repository and contact metadata;
- project authorship headers in first-party firmware source/header files and build/flash scripts;
- visible boot diagnostics showing `by alfawalidou / McWall`, official repository, Discord `.mcwall` and Telegram `@McWall07`;
- beginner walkthrough and README attribution links.

The passive origin marker performs no tracking, network access, telemetry or phone-home behavior.

## Wiring correction — 2026-08-28

Hardware troubleshooting identified a stability issue related to powering/routing OLED VCC from the ESP32 `3V3` pin on the validated prototype.

Final recommended wiring:

- OLED VCC -> external regulated 5 V buck output
- OLED GND -> common GND
- OLED SDA -> GPIO21
- OLED SCL -> GPIO22
- do **not** use ESP32 `3V3` as OLED VCC on this prototype

With the exact OLED module tested, VCC measured 5.0 V while SDA/SCL remained at approximately 3.2 V. Repeated autonomous multi-minute haptic tests were stable after moving OLED VCC to the external 5 V buck output.

This is module-specific: another SSD1306 breakout must be verified for 5 V VCC and ESP32-safe I2C pull-up levels before using the same wiring.

## v3.0.0-validated — 2026-08-26

Initial standalone Trigger V3 snapshot.

Highlights:

- Bluetooth Classic SPP ForceTube-compatible haptic input
- non-blocking recoil controller
- stabilized rumble output
- OLED status UI
- two-button final control layout
- calibrated 2600 ms LASER profile
- manual PISTOL / SNIPER / M16 / P90 / PKM / LASER profiles
- SNIPER 120 ms rumble pulse
- validated Bluetooth reconnect / APK restart / reset recovery
- validated serial monitor workflow with RTS/DTR forced inactive

Source snapshot provenance: internal development commit `913fefff49d51f9b65072eacdb34fa90991b474a`.
