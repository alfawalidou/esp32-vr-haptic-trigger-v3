# START HERE — Beginner build and first flash

This is the **single recommended beginner path** for building and flashing ESP32 VR Haptic Trigger V3 on **Windows 10/11**.

If you are not a developer and you simply want to build the hardware, clone the firmware, flash it, and test both operating modes, follow this page from top to bottom. Do not mix these steps with the advanced/manual PlatformIO commands in the other documents unless you are troubleshooting.

> **Target:** current `main` branch, Trigger V3, classic ESP32 with Bluetooth Classic SPP.
>
> **Firmware behavior:** the device boots in `HAPTIC_ONLY`. A long press of the GPIO14 button toggles `HAPTIC_ONLY` / `TRIGGER_FALLBACK`.

---

## 1. What you need

Main parts used by the validated prototype:

| Part | Purpose |
|---|---|
| Classic ESP32 DevKit with Bluetooth Classic SPP | main controller |
| BTS7960 H-bridge | recoil solenoid driver |
| 1564B-class recoil solenoid / linear actuator | recoil |
| IRLZ44N N-channel MOSFET | rumble motor driver |
| 2 × Xbox-style ERM rumble motors | vibration |
| 2 × WS2812B-compatible LEDs | status |
| SSD1306 128×64 I2C OLED at `0x3C` | display |
| Momentary trigger switch | physical trigger |
| Momentary profile/mode switch | profile + operating mode |
| Buzzer on GPIO27 | optional feedback |
| Regulated 5 V buck supply | ESP32 VIN/5V + OLED VCC |
| `1.5KE24CA` TVS | BTS7960 output protection |
| Suitable actuator power source, fuse, wiring and connectors | power system |

**Important:** the repository does not define one universal battery/current/fuse rating because those values depend on the actual solenoid, rumble motors and power source. Do not guess high-current power ratings. Size them from the real hardware.

---

## 2. Install the software on Windows

Open **PowerShell**.

Check whether Git and Python are already installed:

```powershell
git --version
python --version
```

If both commands work, continue.

If one is missing, install Git for Windows and a current Python 3 release, then open a new PowerShell window and run the two checks again.

Install PlatformIO:

```powershell
python -m pip install --upgrade platformio
```

Confirm it works:

```powershell
pio --version
```

Do not continue until `pio --version` returns a PlatformIO version.

---

## 3. Clone the firmware

Choose a folder where you want the project and run:

```powershell
git clone https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3.git
cd esp32-vr-haptic-trigger-v3
```

Confirm that you are on the normal release branch:

```powershell
git branch --show-current
```

Expected result:

```text
main
```

You do not need to edit the source code for the validated hardware configuration.

---

## 4. Wire the ESP32 controls

Use this GPIO map:

| Function | ESP32 connection |
|---|---:|
| Trigger switch | GPIO13 -> switch -> GND |
| Profile / mode switch | GPIO14 -> switch -> GND |
| Rumble MOSFET control | GPIO17 |
| BTS7960 RPWM | GPIO23 |
| BTS7960 LPWM | GPIO5 |
| WS2812B data | GPIO16 |
| Buzzer | GPIO27 |
| OLED SDA | GPIO21 |
| OLED SCL | GPIO22 |
| GPIO4 | unused |

The trigger and profile buttons use the ESP32 internal pull-ups, so the switches connect their GPIO to GND when pressed.

### OLED wiring — use this exact validated arrangement

```text
OLED VCC -> external regulated 5 V buck OUT
OLED GND -> common GND
OLED SDA -> GPIO21
OLED SCL -> GPIO22
```

**Do not power OLED VCC from the ESP32 `3V3` pin on the validated prototype.**

The exact OLED module used during validation was tested at:

```text
VCC = 5.0 V
SDA ≈ 3.2 V
SCL ≈ 3.2 V
```

If you use a different SSD1306 module, confirm that it accepts 5 V VCC and that its I2C pull-ups do not drive SDA/SCL to 5 V before connecting it to the ESP32.

---

## 5. Wire the haptic power stages

### Recoil

```text
ESP32 GPIO23 / GPIO5
        |
        v
     BTS7960
        |
        v
 recoil solenoid
```

The solenoid is powered by the actuator power system, **not by the ESP32**.

The validated prototype uses a `1.5KE24CA` TVS across the BTS7960 motor outputs (`M+` / `M-`).

### Rumble

```text
ESP32 GPIO17
      |
      v
IRLZ44N MOSFET driver stage
      |
      v
2 × rumble motors
```

GPIO17 is only the PWM control signal. The motors are powered through the external MOSFET/power stage, not from the GPIO.

All required logic/control stages must share the correct common ground reference.

---

## 6. Check the wiring before powering anything

Before the first flash, verify all of these points:

- GPIO13 is the trigger and GPIO14 is the profile/mode button.
- GPIO4 is unused.
- GPIO23 and GPIO5 go only to the BTS7960 control inputs.
- GPIO17 goes only to the rumble MOSFET control path.
- The solenoid and rumble motors are not powered from an ESP32 GPIO.
- OLED VCC goes to the regulated external 5 V buck output, **not** ESP32 `3V3`.
- OLED SDA/SCL are GPIO21/GPIO22.
- Grounds are common where required.
- No exposed conductor can short the power rails.

For the initial USB flash, keep the external battery/main actuator supply **OFF**. Do not intentionally power this prototype from USB and the external main supply at the same time.

---

## 7. Connect the ESP32 by USB

Connect the ESP32 to the Windows PC with a USB data cable.

In PowerShell, from inside the repository, run:

```powershell
pio device list
```

Find the ESP32 serial port, for example:

```text
COM12
```

If no COM port appears, check the USB cable and the USB-serial driver used by your ESP32 board. Common DevKit boards use CH340 or CP210x USB-serial chips.

---

## 8. Build, fully erase and flash — one beginner command

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\first-flash.ps1
```

The helper will:

1. check that PlatformIO is available;
2. show the detected serial devices;
3. ask you for the ESP32 COM port;
4. require an explicit wiring confirmation;
5. clean the previous build;
6. build the validated `trigger-v3-compat` firmware;
7. perform a **full flash erase**;
8. upload the firmware.

When it finishes successfully, you should see:

```text
Flash completed successfully.
```

You do not need to run a separate build command before this step.

---

## 9. First standalone power-on

After the flash finishes:

1. disconnect USB from the PC;
2. keep the wiring exactly as validated;
3. power the completed haptic gun from its normal external power system;
4. wait for the ESP32/OLED to boot.

The firmware starts in:

```text
HAPTIC_ONLY
```

This means the Bluetooth haptic stream can drive recoil/rumble, but the physical trigger does not create a local shot yet.

---

## 10. Connect Bluetooth

On the compatible Android / VR haptic application, connect to:

```text
ForceTubeVR 1187883197
```

Test a normal Bluetooth haptic effect.

Expected behavior: Bluetooth KICK/RUMBLE commands drive the haptic hardware while the device remains in `HAPTIC_ONLY`.

---

## 11. Test the physical trigger mode

Hold the GPIO14 profile/mode button for about one second.

The operating mode changes to:

```text
TRIGGER_FALLBACK
```

Now GPIO13 acts as the local physical trigger.

Start with the `PISTOL` profile. It produces one reduced kick and no rumble.

A short press on GPIO14 selects the next profile in this order:

```text
PISTOL -> SNIPER -> M16 -> P90 -> PKM -> LASER -> PISTOL
```

Profile behavior:

| Profile | Local behavior |
|---|---|
| PISTOL | single reduced kick, no rumble |
| SNIPER | full kick + short 120 ms rumble pulse |
| M16 | automatic kick every 150 ms + rumble while held |
| P90 | automatic kick every 150 ms + rumble while held |
| PKM | automatic kick every 150 ms + strong rumble while held |
| LASER | 20-step rumble charge over 2600 ms, then kick and rumble stop |

Hold GPIO14 again for about one second to return to `HAPTIC_ONLY`.

---

## 12. You are finished when these checks pass

Your build is working correctly when:

```text
[OK] ESP32 boots without USB connected
[OK] OLED works from the external regulated 5 V buck output
[OK] Bluetooth connects as ForceTubeVR 1187883197
[OK] Bluetooth haptics work in HAPTIC_ONLY
[OK] GPIO14 short press changes profiles
[OK] GPIO14 long press toggles HAPTIC_ONLY / TRIGGER_FALLBACK
[OK] GPIO13 fires the selected local profile in TRIGGER_FALLBACK
[OK] recoil and rumble stop normally after their effects
```

At this point the normal end-user setup is complete.

---

## If something goes wrong

Use these checks in this order:

### PlatformIO not found

```powershell
python -m pip install --upgrade platformio
pio --version
```

### No ESP32 COM port

Try another USB data cable, another USB port, and verify the CH340/CP210x driver for your board.

### Upload fails

Close any serial monitor or program that may already be using the COM port, then run the beginner flash command again.

### Bluetooth device never appears

Confirm that the board is a **classic ESP32 with Bluetooth Classic SPP**. ESP32-S2/S3/C3 are not drop-in replacements for this firmware.

### OLED causes instability or does not behave correctly

Re-check the validated OLED wiring:

```text
VCC -> regulated external 5 V buck OUT
GND -> common GND
SDA -> GPIO21
SCL -> GPIO22
```

Do not move OLED VCC back to ESP32 `3V3` on this prototype.

### Need serial diagnostics

Use only the supplied monitor helper:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\monitor.ps1 -Port COM12
```

Replace `COM12` with your port.

The helper forces RTS and DTR inactive. Do not use a plain PlatformIO monitor command for hardware stability testing.

---

## Advanced documentation — only after the beginner path

You do **not** need these files to complete the normal first build and flash. They are available when you want deeper technical details:

- [`docs/HARDWARE.md`](docs/HARDWARE.md) — full hardware architecture
- [`docs/PINOUT.md`](docs/PINOUT.md) — detailed 30-pin breakout mapping
- [`docs/BOM.md`](docs/BOM.md) — bill of materials
- [`docs/RUMBLE_WIRING.md`](docs/RUMBLE_WIRING.md) — rumble/MOSFET wiring
- [`docs/PROFILES.md`](docs/PROFILES.md) — local profile details
- [`docs/PROTOCOL.md`](docs/PROTOCOL.md) — Bluetooth protocol
- [`docs/FLASHING.md`](docs/FLASHING.md) — manual PlatformIO build/flash commands
- [`docs/VALIDATION.md`](docs/VALIDATION.md) — physical validation results

Reference photos are stored in [`ref/`](ref/).
