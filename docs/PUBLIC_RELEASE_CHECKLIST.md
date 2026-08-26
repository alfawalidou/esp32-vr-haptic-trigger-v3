# Future public-release checklist

This standalone repository is prepared to become public later, but the following decisions should be completed first.

- [ ] Choose an open-source license and add `LICENSE`.
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
- [ ] Remove any future secrets, tokens, private serials, local paths or personal metadata before changing repository visibility.

The repository should remain private until this checklist is intentionally reviewed.