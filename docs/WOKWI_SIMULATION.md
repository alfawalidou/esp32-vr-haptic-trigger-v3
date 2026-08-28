# Wokwi simulation walkthrough

**ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall**  
GitHub: `@alfawalidou`  
Discord: `.mcwall`  
Telegram: `@McWall07`

This is the simulation path for Trigger V3. It exists so you can exercise the real firmware logic without reconnecting the physical recoil/rumble hardware.

> The production `trigger-v3-safe` and `trigger-v3-compat` environments are unchanged. Simulation uses its own isolated `trigger-v3-wokwi` environment.

## What Wokwi can test here

Wokwi can simulate the classic ESP32, GPIO, I2C SSD1306, buttons, buzzer, WS2812 output and LEDC/PWM behavior. The supplied diagram also includes a logic analyzer connected to the key haptic output pins.

The simulation therefore covers:

- firmware boot;
- the author splash screen;
- OLED UI;
- GPIO13 trigger input;
- GPIO14 short press for profile changes;
- GPIO14 long press for `HAPTIC_ONLY` / `TRIGGER_FALLBACK`;
- PISTOL / SNIPER / M16 / P90 / PKM / LASER local profile logic;
- GPIO17 rumble PWM;
- GPIO5 / GPIO23 recoil direction outputs;
- GPIO16 WS2812 status output;
- GPIO27 buzzer feedback;
- ForceTube packet parsing and haptic-controller handling through the simulation transport shim;
- simulated Bluetooth disconnect/reconnect state handling.

## Important limitation: Bluetooth

Wokwi currently does **not** simulate ESP32 Bluetooth hardware. For that reason, only the `trigger-v3-wokwi` environment force-includes `include/WokwiBluetoothShim.h`.

The shim does not replace the ForceTube parser or haptic controller. Instead, text typed in the Wokwi Serial Terminal is converted into the same four-byte ForceTube packets that the production Bluetooth path feeds into the existing firmware:

```text
0x2A 0xB0 channel intensity
```

This keeps the packet parser and haptic logic under test while replacing only the unavailable Bluetooth transport.

The shim is simulation-only and is never used by the real SAFE or COMPAT firmware.

## 1. Pull the simulation files

From the repository branch containing the simulation support:

```powershell
git pull --ff-only origin feat/authorship-provenance
```

## 2. Build the Wokwi firmware

Run the supplied helper:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-wokwi.ps1
```

It builds:

```text
trigger-v3-wokwi
```

Expected outputs:

```text
.pio/build/trigger-v3-wokwi/firmware.bin
.pio/build/trigger-v3-wokwi/firmware.elf
```

`wokwi.toml` already points to those files.

## 3. Start Wokwi

Recommended local workflow:

1. Open the repository in VS Code.
2. Install the official Wokwi extension if it is not already installed.
3. Open `diagram.json`.
4. Start the Wokwi simulator.

The circuit uses a classic ESP32 DevKitC V4 model and the same GPIO numbers as the real Trigger V3 firmware.

## 4. Check the author splash

At startup, the virtual SSD1306 should briefly show the Trigger V3 author splash containing:

```text
TRIGGER V3
by alfawalidou /
McWall
GitHub: alfawalidou
@McWall07 | .mcwall
```

After the splash, the normal Trigger V3 interface takes over.

## 5. Test the physical buttons virtually

The virtual buttons are labeled:

```text
TRIGGER GPIO13
PROFILE / MODE GPIO14
```

Keyboard shortcuts are also configured:

```text
T = Trigger
P = Profile / Mode
```

A short press of PROFILE/MODE advances the profile.

Hold PROFILE/MODE for about one second to toggle:

```text
HAPTIC_ONLY <-> TRIGGER_FALLBACK
```

In `TRIGGER_FALLBACK`, press TRIGGER to exercise the selected local profile.

## 6. Inject simulated ForceTube commands

The Wokwi Serial Terminal accepts these commands:

```text
KICK 255
RUMBLE 128
RUMBLE 0
BT OFF
BT ON
HELP
```

Examples:

```text
KICK 120
```

injects the production-format packet:

```text
2A B0 00 78
```

and:

```text
RUMBLE 159
```

injects:

```text
2A B0 01 9F
```

The existing `ForceTubeProtocolParser` parses those bytes and the existing `HapticController` handles the resulting command.

`BT OFF` triggers the firmware's normal Bluetooth-disconnect callback path, which should stop haptic outputs. `BT ON` reconnects the simulated transport.

## 7. Watch the virtual outputs

The diagram provides three simple visual output indicators:

```text
GPIO17 -> yellow LED -> rumble PWM
GPIO5  -> red LED    -> solenoid LPWM / forward path
GPIO23 -> green LED  -> solenoid RPWM / reverse path
```

Short recoil pulses may be difficult to judge by eye, so the diagram also contains an 8-channel Wokwi Logic Analyzer.

Connected analyzer channels:

```text
D0 = RUMBLE GPIO17
D1 = LPWM GPIO5
D2 = RPWM GPIO23
D3 = WS2812 GPIO16
D4 = TRIGGER GPIO13
D5 = PROFILE GPIO14
```

This is useful for checking the 30 ms forward pulse, 2 ms reverse pulse, PWM behavior and button timing.

## 8. OLED power representation in the simulator

The virtual OLED VCC is connected to the ESP32 model's `5V` node **only as a representation of the validated external regulated 5 V buck output**.

On the physical prototype the authoritative wiring remains:

```text
OLED VCC -> external regulated 5 V buck OUT
OLED GND -> common GND
OLED SDA -> GPIO21
OLED SCL -> GPIO22
```

Do not reinterpret the virtual diagram as permission to route the physical OLED VCC from ESP32 `3V3`.

## What the simulation cannot validate

Wokwi does not replace physical validation for:

- real Bluetooth Classic SPP RF behavior;
- BTS7960 electrical behavior under solenoid current;
- actual recoil force;
- real rumble motor current;
- EMI / conducted-noise problems;
- buck converter behavior;
- USB/external-supply backfeed;
- wiring resistance, connector heating or fuse sizing;
- the exact physical OLED module's 5 V / I2C electrical behavior.

The simulator is therefore a firmware and logic validation layer, not an electrical-power validation tool.

## Production reminder

For the physical device continue to use only:

```text
trigger-v3-safe
trigger-v3-compat
```

`trigger-v3-wokwi` must never be flashed as the production reference firmware. It deliberately substitutes the Bluetooth transport for simulation.
