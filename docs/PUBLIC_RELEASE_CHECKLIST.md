# Future public-release checklist

**ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall**

This standalone repository is prepared to become public later, but the following decisions should be completed first.

- [x] Choose an open-source license and add `LICENSE` — Apache License 2.0.
- [x] Add `NOTICE`, `AUTHORS.md`, `CITATION.cff` and `PROJECT_PROVENANCE.md`.
- [x] Embed the original project identity and passive provenance marker in the firmware source.
- [x] Add intentional public project contacts: GitHub `@alfawalidou`, Discord `.mcwall`, Telegram `@McWall07`.
- [ ] Decide whether the Bluetooth name / numeric device identifier should remain fixed or become configurable.
- [ ] Add clear photographs of the final hardware.
- [ ] Produce a final schematic from the physically validated wiring.
- [ ] Record exact resistor values in the IRLZ44N rumble stage.
- [ ] Record supply voltage, measured solenoid current and rumble-motor current.
- [ ] Record fuse and connector ratings.
- [ ] Verify OLED module power requirements for the exact breakout.
- [ ] Test a fresh clone on a second machine.
- [ ] Run both `trigger-v3-safe` and `trigger-v3-compat` builds from a clean checkout.
- [ ] Repeat the validation matrix after any dependency/toolchain update.
- [ ] Consider producing signed/checksummed firmware binaries for a GitHub Release.
- [ ] Add a mechanical safety section for recoil mounting and pinch points.
- [ ] Review trademark wording and keep the non-affiliation disclaimer.
- [ ] Remove any future secrets, tokens, private serials, local paths or unintended personal metadata before changing repository visibility.

The repository should remain private until this checklist is intentionally reviewed.

- [x] remove local drive paths, Windows username/hostname references and development COM port from Git history

Intentional project identity/contact information (`alfawalidou / McWall`, `.mcwall`, `@McWall07`) is public attribution metadata and should not be removed by the privacy-cleanup step.
