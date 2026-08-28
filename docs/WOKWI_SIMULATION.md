# Wokwi simulation walkthrough

**ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall**  
GitHub: `@alfawalidou`  
Discord: `.mcwall`  
Telegram: `@McWall07`

This is the simulation path for Trigger V3. It exercises the real firmware logic without reconnecting the physical recoil/rumble hardware.

> Production hardware uses `trigger-v3-safe` or `trigger-v3-compat`. Wokwi uses the isolated `trigger-v3-wokwi` environment.

## What Wokwi covers

The supplied simulation can exercise:

- firmware boot;
- the current `VR HapGunV3` OLED splash;
- normal OLED UI;
- GPIO13 trigger input;
- GPIO14 short press for profile changes;
- GPIO14 long press for `HAPTIC_ONLY` / `TRIGGER_FALLBACK`;
- PISTOL / SNIPER / M16 / P90 / PKM / LASER local profile logic;
- GPIO17 rumble PWM;
- GPIO5 / GPIO23 recoil direction outputs;
- GPIO16 WS2812 output;
- GPIO27 buzzer feedback;
- ForceTube packet parsing and haptic-controller handling through the simulation transport shim;
- simulated Bluetooth disconnect/reconnect state handling.

## Bluetooth limitation

Wokwi does not simulate the ESP32 Bluetooth Classic hardware used by the production firmware. Only `trigger-v3-wokwi` force-includes `include/WokwiBluetoothShim.h`.

The shim replaces only the unavailable Bluetooth transport. It feeds the existing ForceTube parser with the same four-byte packet format used by production:

```text
0x2A 0xB0 channel intensity
```

The production SAFE and COMPAT environments do not use this shim.

## 1. Update the repository

Simulation support is now part of `main`. From the repository root:

```powershell
git switch main
git pull --ff-only origin main
```

There is no separate `feat/authorship-provenance` branch to pull.

## 2. Build the Wokwi firmware

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-wokwi.ps1
```

The helper builds:

```text
trigger-v3-wokwi
```

Expected outputs:

```text
.pio/build/trigger-v3-wokwi/firmware.bin
.pio/build/trigger-v3-wokwi/firmware.elf
```

`wokwi.toml` already points to those files.

If PlatformIO hits a Windows long-path/package-cache issue, a short core directory can be used for the current PowerShell session, for example:

```powershell
$env:PLATFORMIO_CORE_DIR = "D:\pio"
```

Then rerun the Wokwi build helper.

## 3. Start Wokwi

1. Open the repository in VS Code.
2. Install the official Wokwi extension if needed.
3. Open `diagram.json`.
4. Start the Wokwi simulator.

The diagram uses a classic ESP32 DevKitC V4 model and the same GPIO assignments as Trigger V3.

## 4. Current boot splash

At startup the virtual SSD1306 should briefly show:

```text
VR HapGunV3
---------------------
Discord: .mcwall
Telegram: @McWall07
GitHub: @alfawalidou
```

After about 1.5 seconds, the normal Trigger V3 UI takes over.

## 5. Test the virtual controls

The virtual buttons are:

```text
TRIGGER GPIO13
PROFILE / MODE GPIO14
```

Keyboard shortcuts:

```text
T = Trigger
P = Profile / Mode
```

A short press on GPIO14 advances the profile.

Hold GPIO14 for about one second to toggle:

```text
HAPTIC_ONLY <-> TRIGGER_FALLBACK
```

In `TRIGGER_FALLBACK`, press GPIO13 to exercise the selected local profile.

## 6. Inject simulated ForceTube commands

The Wokwi Serial Terminal accepts:

```text
KICK 255
RUMBLE 128
RUMBLE 0
BT OFF
BT ON
HELP
```

For example:

```text
KICK 120
```

becomes:

```text
2A B0 00 78
```

and:

```text
RUMBLE 159
```

becomes:

```text
2A B0 01 9F
```

The normal `ForceTubeProtocolParser` and `HapticController` process those packets.

`BT OFF` exercises the normal disconnect callback path and should stop haptic outputs. `BT ON` reconnects the simulated transport.

## 7. Virtual outputs and logic analyzer

The diagram exposes visual indicators for:

```text
GPIO17 -> rumble PWM
GPIO5  -> solenoid LPWM / forward path
GPIO23 -> solenoid RPWM / reverse path
```

The logic analyzer channels are:

```text
D0 = RUMBLE GPIO17
D1 = LPWM GPIO5
D2 = RPWM GPIO23
D3 = WS2812 GPIO16
D4 = TRIGGER GPIO13
D5 = PROFILE GPIO14
```

This is useful for checking the 30 ms Forward pulse, 2 ms Reverse pulse, PWM behavior and button timing.

## 8. OLED power representation

The virtual OLED VCC is connected to the ESP32 model's `5V` node only as a representation of the validated external regulated 5 V buck output.

The physical prototype wiring remains:

```text
OLED VCC -> external regulated 5 V buck OUT
OLED GND -> common GND
OLED SDA -> GPIO21
OLED SCL -> GPIO22
```

Do not reinterpret the Wokwi diagram as permission to power the physical OLED from ESP32 `3V3`.

## What Wokwi cannot validate

Wokwi does not replace physical validation for:

- real Bluetooth Classic SPP RF behavior;
- BTS7960 electrical behavior under solenoid current;
- actual recoil force;
- real rumble motor current;
- EMI / conducted-noise issues;
- buck converter behavior;
- USB/external-supply backfeed;
- wiring resistance, connector heating or fuse sizing;
- the exact physical OLED module's electrical behavior.

## Production reminder

For the physical device use only:

```text
trigger-v3-safe
trigger-v3-compat
```

Do not flash `trigger-v3-wokwi` as the production reference firmware; it deliberately substitutes the Bluetooth transport for simulation.
