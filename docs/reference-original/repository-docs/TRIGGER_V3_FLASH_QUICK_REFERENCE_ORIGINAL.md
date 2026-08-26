# Trigger V3 — Flash Quick Note

Quick reference for building, flashing and monitoring the Trigger V3 prototype.

## 1. PlatformIO core directory

From the repository root in PowerShell:

```powershell
$env:PLATFORMIO_CORE_DIR = "<LOCAL_PLATFORMIO_CORE_DIR>"
```

## 2. Clean COMPAT build

```powershell
pio run `
  -d ".\variants\trigger-v3" `
  -e trigger-v3-compat `
  -t clean
```

## 3. Build COMPAT

```powershell
pio run `
  -d ".\variants\trigger-v3" `
  -e trigger-v3-compat
```

## 4. Full erase before physical flash

For prototype validation, perform a full flash erase before every physical flash:

```powershell
pio run `
  -d ".\variants\trigger-v3" `
  -e trigger-v3-compat `
  -t erase `
  --upload-port COMx
```

## 5. Upload

```powershell
pio run `
  -d ".\variants\trigger-v3" `
  -e trigger-v3-compat `
  -t upload `
  --upload-port COMx
```

## 6. Serial monitor — IMPORTANT

Use this command for Trigger V3 tests:

```powershell
pio device monitor -p COMx -b 115200 --rts 0 --dtr 0
```

`--rts 0 --dtr 0` is mandatory for the current prototype validation workflow. Opening the PlatformIO monitor without these options was observed to disturb ESP32 behavior and made earlier stability tests unreliable.

Do **not** use the old command during validation:

```powershell
pio device monitor -p COMx -b 115200
```

## Validated quick checks

After flashing, the current V3 test sequence should cover:

- APK ForceTube KICK/RUMBLE effects
- LASER ramp from low rumble up to 255, followed by KICK and rumble stop
- Bluetooth OFF/ON recovery
- APK kill/reopen recovery
- physical reset-button recovery
- GPIO13 trigger
- GPIO14 short press for profile change
- GPIO14 long press for HAPTIC_ONLY / TRIGGER_FALLBACK toggle
- local PISTOL, SNIPER, M16 and P90 behavior

Keep each test controlled and change only one variable at a time when investigating regressions.
