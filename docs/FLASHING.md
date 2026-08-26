# Build, erase, flash and monitor

## Environments

`platformio.ini` defines two environments.

### Safe

```powershell
pio run -e trigger-v3-safe
```

Compiles with physical actuators disabled.

### COMPAT / physical actuators enabled

```powershell
pio run -e trigger-v3-compat
```

Use only after wiring has been verified.

## Clean build

```powershell
pio run -e trigger-v3-compat -t clean
pio run -e trigger-v3-compat
```

## Full erase before physical validation flash

The validated development workflow performs a full erase before each physical flash:

```powershell
pio run -e trigger-v3-compat -t erase --upload-port COMx
```

Replace `COMx` with the actual serial port.

## Upload

```powershell
pio run -e trigger-v3-compat -t upload --upload-port COMx
```

## Serial monitor â€” important

Use RTS and DTR forced inactive:

```powershell
pio device monitor -p COMx -b 115200 --rts 0 --dtr 0
```

During development, opening the PlatformIO monitor without these options was observed to disturb ESP32 behavior and invalidate stability tests.

Do not use this as the validation reference command:

```powershell
pio device monitor -p COMx -b 115200
```

The reset performed by the upload tool after flashing is separate and expected.

## Windows path-length note

One validated Windows setup used:

```powershell
$env:PLATFORMIO_CORE_DIR = "<LOCAL_PLATFORMIO_CORE_DIR>"
```

to keep PlatformIO package paths short. This is a workstation-specific workaround, not a firmware requirement.