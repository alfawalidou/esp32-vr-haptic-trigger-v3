# Validation status

Validated on physical hardware with the `trigger-v3-compat` environment.

## Final functional matrix

| Test | Result |
|---|---|
| COMPAT build | PASS |
| Full erase + upload | PASS |
| Boot health | PASS |
| Bluetooth SPP connect | PASS |
| APK KICK 255 / RUMBLE 255 | PASS |
| APK KICK 56 / RUMBLE 125 | PASS |
| APK KICK 79 / RUMBLE 159 | PASS |
| APK KICK 129 / RUMBLE 255 | PASS |
| APK LASER ramp to 255 + kick + stop | PASS |
| Bluetooth OFF -> ON recovery | PASS |
| APK kill -> reopen recovery | PASS |
| Physical ESP32 reset recovery | PASS |
| OLED detected at 0x3C | PASS |
| Trigger GPIO13 | PASS |
| Profile short press GPIO14 | PASS |
| Profile long press / mode toggle | PASS |
| PISTOL local profile | PASS |
| SNIPER local pulse rumble | PASS |
| M16 local auto + rumble | PASS |
| P90 local auto + rumble | PASS |
| PKM local auto + rumble | PASS |
| LASER local 2600 ms charge/release | PASS |
| Mixed long haptic traffic with OLED attached | PASS |

## Final build footprint observed

A final COMPAT build reported approximately:

```text
RAM   14.0%
Flash 58.2%
```

## Serial monitor validation rule

Stability testing must use:

```powershell
pio device monitor -p COMx -b 115200 --rts 0 --dtr 0
```

Substitute the correct port.

## Expected `INHIBITED`

An occasional:

```text
[FT] KICK intensity=... result=INHIBITED
```

is expected when a new non-zero KICK arrives while a previous physical recoil cycle is still active. The event is intentionally discarded instead of being queued and replayed late.