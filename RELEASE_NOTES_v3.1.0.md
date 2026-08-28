# ESP32 VR Haptic Trigger V3 — v3.1.0

First public binary release of the standalone Trigger V3 project.

## Highlights

- Built from the physically validated Trigger V3 firmware baseline.
- New `VR HapGunV3` OLED boot identity splash.
- Authorship and provenance metadata for **alfawalidou / McWall**.
- Wokwi simulation environment for OLED/UI and simulated haptic input testing.
- Current first-party project code and documentation licensed under **PolyForm Noncommercial License 1.0.0**.
- Personal, hobby and other noncommercial use permitted under the license terms.
- Commercial use requires separate prior written permission from **alfawalidou / McWall**.

## Planned downloadable files

- `VR-HapGunV3-v3.1.0-compat.bin` — normal firmware for the validated hardware configuration.
- `VR-HapGunV3-v3.1.0-safe.bin` — actuator-disabled diagnostic firmware.
- `VR-HapGunV3-v3.1.0-factory.bin` — complete flash image for a clean ESP32 installation, if generated during packaging.
- `SHA256SUMS.txt` — checksums for published binaries.

## Hardware note

The validated prototype powers the OLED VCC from the external regulated 5 V buck output. Verify the voltage requirements and I2C pull-up arrangement of your exact OLED breakout before reproducing this wiring.

## Attribution

ESP32 VR Haptic Trigger V3 — by **alfawalidou / McWall**  
GitHub: `@alfawalidou`  
Discord: `.mcwall`  
Telegram: `@McWall07`
