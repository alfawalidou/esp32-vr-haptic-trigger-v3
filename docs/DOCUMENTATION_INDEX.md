# Documentation index

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
