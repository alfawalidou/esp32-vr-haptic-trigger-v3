# ESP32 VR Haptic Trigger V3 — v3.1.0

First public binary release of the standalone **Trigger V3** project.

Release page: <https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3/releases/tag/v3.1.0>

## Highlights

- Based on the previously physically validated Trigger V3 firmware baseline.
- Current `VR HapGunV3` OLED boot identity splash.
- Authorship and provenance metadata for **alfawalidou / McWall**.
- Wokwi simulation environment for OLED/UI and simulated haptic input testing.
- SAFE and COMPAT firmware builds completed successfully for the release packaging step.
- Current first-party project code and documentation licensed under **PolyForm Noncommercial License 1.0.0**.
- Personal, hobby and other noncommercial use permitted under the license terms.
- Commercial use requires separate prior written permission from **alfawalidou / McWall**.

> **Validation note:** the original Trigger V3 baseline was physically validated. The v3.1.0 release adds packaging, identity/provenance, licensing, Wokwi support and the new OLED splash. The exact v3.1.0 release binaries were successfully compiled and the visual/UI changes were validated in Wokwi, but those exact packaged binaries did not undergo a new complete physical hardware validation cycle before publication.

## Published firmware assets

### Normal / COMPAT

- `VR-HapGunV3-v3.1.0-compat-factory.bin` — complete combined image intended for a clean / first ESP32 installation.
- `VR-HapGunV3-v3.1.0-compat.bin` — application firmware image for the normal hardware configuration.

### SAFE diagnostic

- `VR-HapGunV3-v3.1.0-safe-factory.bin` — complete combined SAFE image with actuator outputs disabled.
- `VR-HapGunV3-v3.1.0-safe.bin` — SAFE application firmware with actuator outputs disabled.

### Integrity

- `SHA256SUMS.txt` — SHA-256 checksums for all four published firmware binaries.

For a normal first installation, use the **COMPAT factory image**.

## Hardware note

The validated prototype powers OLED VCC from the external regulated **5 V buck output**, not from the ESP32 `3V3` pin.

Verify the voltage requirements and I2C pull-up arrangement of your exact OLED breakout before reproducing this wiring.

## Bluetooth compatibility

The production firmware keeps the Bluetooth Classic SPP identity:

```text
ForceTubeVR 1187883197
```

## License

Current first-party project files are licensed under:

**PolyForm Noncommercial License 1.0.0**  
SPDX: `PolyForm-Noncommercial-1.0.0`

Commercial use, sale, resale, paid-product integration or use for commercial advantage requires separate prior written permission from **alfawalidou / McWall**.

Third-party libraries and dependencies retain their respective licenses.

## Attribution

ESP32 VR Haptic Trigger V3 — **by alfawalidou / McWall**

GitHub: `@alfawalidou`  
Discord: `.mcwall`  
Telegram: `@McWall07`
