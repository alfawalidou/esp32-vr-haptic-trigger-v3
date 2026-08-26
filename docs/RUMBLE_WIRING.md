# Rumble motor and IRLZ44N wiring

## Overview

![Xbox rumble wiring overview](../assets/hardware/xbox-rumble-wiring-overview.png)

## Detailed original reconstruction

![Xbox rumble wiring detailed](../assets/hardware/xbox-rumble-wiring-detailed.png)

These images were recovered from the original Trigger V3 documentation.

Final firmware interface:

```text
GPIO17 -> PWM 175 Hz / 8-bit -> IRLZ44N driver stage -> rumble motors
```

GPIO17 is a control signal only. The motors are powered from the appropriate power rail through the external driver stage.

Validated firmware behavior:

- intensity 0..255
- watchdog 500 ms
- hardware apply interval 10 ms
- continuous AUTO rumble refreshed every 100 ms
- SNIPER rumble pulse 120 ms
- LASER 20-step charge ramp

The complete original engineering reconstruction is preserved at:

[`reference-original/trigger-v3/XBOX_RUMBLE_WIRING.md`](reference-original/trigger-v3/XBOX_RUMBLE_WIRING.md)
