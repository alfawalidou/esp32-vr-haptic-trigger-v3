# Public release status and future checklist

**ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall**

The standalone repository is now public and the first binary release, **v3.1.0**, was published on 2026-08-28.

Release page: <https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3/releases/tag/v3.1.0>

## Completed for the first public release

- [x] Choose a source-available noncommercial software license and add `LICENSE` — PolyForm Noncommercial License 1.0.0 (`PolyForm-Noncommercial-1.0.0`).
- [x] State clearly that commercial use requires separate prior written permission from `alfawalidou / McWall`.
- [x] Add `NOTICE`, `AUTHORS.md`, `CITATION.cff` and `PROJECT_PROVENANCE.md`.
- [x] Embed the original project identity and passive provenance marker in the firmware source.
- [x] Add intentional public project contacts: GitHub `@alfawalidou`, Discord `.mcwall`, Telegram `@McWall07`.
- [x] Keep the production Bluetooth identity fixed at `ForceTubeVR 1187883197` for compatibility.
- [x] Remove local drive paths, Windows username/hostname references and development COM-port values from the public history.
- [x] Make the standalone repository public.
- [x] Publish the first GitHub Release as `v3.1.0`.
- [x] Build both `trigger-v3-safe` and `trigger-v3-compat` successfully for release packaging.
- [x] Publish SAFE and COMPAT application binaries.
- [x] Publish SAFE and COMPAT combined/factory binaries.
- [x] Publish `SHA256SUMS.txt` for all four firmware binaries.
- [x] Review the non-affiliation wording for ForceTube / ProTubeVR compatibility references.
- [x] Repair public documentation links and remove stale references to deleted development branches.

## Still useful for future releases

- [ ] Add clear photographs specifically labelled as the final assembled hardware revision.
- [ ] Produce a final electrical schematic from the physically validated wiring, not only a terminal/pin map.
- [ ] Record exact resistor values in the IRLZ44N rumble stage.
- [ ] Record supply voltage, measured solenoid current and rumble-motor current.
- [ ] Record fuse and connector ratings.
- [ ] Record the exact OLED breakout model/part number and its documented VCC range.
- [ ] Test a fresh clone on a second machine.
- [ ] Repeat SAFE and COMPAT builds from a completely clean checkout before a future firmware release.
- [ ] Repeat the physical validation matrix after any dependency/toolchain update that can change firmware behavior.
- [ ] Consider cryptographic signing in addition to the SHA-256 checksums already published.
- [ ] Add a dedicated mechanical safety section covering recoil mounting, pinch points and fastener inspection.

## Validation boundary for v3.1.0

The original Trigger V3 firmware baseline was physically validated. The exact v3.1.0 packaged binaries were successfully compiled and the OLED/UI changes were validated in Wokwi, but those exact release binaries did not undergo a new complete physical hardware validation cycle before publication.

See [`VALIDATION.md`](VALIDATION.md) for the physical baseline validation matrix and [`../RELEASE_NOTES_v3.1.0.md`](../RELEASE_NOTES_v3.1.0.md) for the published release assets.

## Licensing note

PolyForm Noncommercial 1.0.0 is a noncommercial source-available license, not an OSI-approved open-source license.

Historical versions or commits previously distributed under Apache License 2.0 retain the rights already granted for those historical copies. The current noncommercial policy applies prospectively to current and future first-party project versions.
