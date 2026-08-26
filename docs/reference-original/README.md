# Original Trigger V3 development documentation

This directory preserves technical documentation and assets recovered from the original private Trigger V3 development repository.

The current standalone documents in `docs/` are authoritative.

Some archived diagrams predate the final migration to GPIO14 and may show GPIO4 as PROFILE. The final validated two-button layout is:

```text
GPIO13 = TRIGGER
GPIO14 = PROFILE / MODE
GPIO4  = UNUSED
```

The archive includes:

- original Trigger V3 README
- BOOT_HEALTH.md
- DESIGN_IDEA.md
- HARDWARE_VALIDATION.md
- PROJECT_STATE.md
- SIMPLIFIED_CONTROLS_WIRING.md
- TEST_PLAN.md
- VALIDATION_LOG.md
- XBOX_RUMBLE_WIRING.md
- original Trigger V3 wiring/design images
- selected repository-level BOM/hardware/flash references

`AI_HANDOFF_PROMPT.md` is intentionally excluded because it is an internal AI-session handoff artifact, not product documentation.

Review this archive before changing the repository from private to public.
