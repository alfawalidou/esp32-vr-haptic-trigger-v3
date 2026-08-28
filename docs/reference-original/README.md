# Original Trigger V3 development documentation

This directory preserves technical documentation and assets recovered from the original private Trigger V3 development repository.

The current standalone documents in [`docs/`](../) are authoritative.

Some archived documents and diagrams predate the final migration to GPIO14 and may show GPIO4 as PROFILE, older operating-mode logic or earlier OLED power assumptions. The final validated two-button layout is:

```text
GPIO13 = TRIGGER
GPIO14 = PROFILE / MODE
GPIO4  = UNUSED
```

The archive includes:

- original Trigger V3 README;
- BOOT_HEALTH.md;
- DESIGN_IDEA.md;
- HARDWARE_VALIDATION.md;
- PROJECT_STATE.md;
- SIMPLIFIED_CONTROLS_WIRING.md;
- TEST_PLAN.md;
- VALIDATION_LOG.md;
- XBOX_RUMBLE_WIRING.md;
- original Trigger V3 wiring/design images;
- selected repository-level BOM/hardware/flash references.

The original private `AI_HANDOFF_PROMPT.md` content is intentionally not published because it was an internal AI-session handoff artifact rather than product documentation. A small public placeholder is retained only so historical links do not lead to a 404.

A small archive compatibility shim is also retained at `trigger-v3/include/HapticProfiles.h` so old links to the catalog do not break; the current authoritative catalog remains [`../../include/HapticProfiles.h`](../../include/HapticProfiles.h).

For current build and wiring instructions, use [`../../START_HERE.md`](../../START_HERE.md), [`../PINOUT.md`](../PINOUT.md), [`../HARDWARE.md`](../HARDWARE.md) and [`../VALIDATION.md`](../VALIDATION.md).
