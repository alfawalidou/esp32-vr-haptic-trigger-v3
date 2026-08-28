# Changelog

## Authorship and provenance pass — 2026-08-28

Project identity is now consistently recorded as **alfawalidou / McWall**.

Added:

- Apache License 2.0 and `NOTICE` attribution;
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
