# Hardware and wiring

This document describes the **current Trigger V3 validated wiring only**.

## ESP32 requirements

Use a classic ESP32 variant that supports **Bluetooth Classic SPP**. Do not assume ESP32-S2/S3/C3 are drop-in replacements for this firmware.

## Validated GPIO map

| GPIO | Function | Notes |
|---:|---|---|
| 23 | BTS7960 RPWM | recoil driver control |
| 5 | BTS7960 LPWM | recoil driver control |
| 17 | Rumble PWM | 175 Hz, 8-bit |
| 16 | WS2812B data | two status LEDs |
| 13 | Trigger | active LOW, `INPUT_PULLUP` |
| 14 | Profile / mode button | active LOW, `INPUT_PULLUP` |
| 27 | Buzzer | local feedback |
| 21 | OLED SDA | I2C |
| 22 | OLED SCL | I2C |
| 4 | Free | intentionally unused |

OLED address: `0x3C`, 128Ã—64 SSD1306.

## Recoil path

```text
ESP32 GPIO23 / GPIO5
        â”‚
        â–¼
     BTS7960
        â”‚
        â–¼
 recoil solenoid
```

Validated haptic constants:

- kick command range: 215â€“255 after mapping
- forward duration: 30 ms
- reverse pulse: 2 ms
- reverse level: 25% of requested kick
- prototype class referenced in firmware: 1564B + BTS7960

The solenoid is not powered from the ESP32.

## Rumble path

```text
ESP32 GPIO17 PWM
        â”‚
        â–¼
  MOSFET driver stage
        â”‚
        â–¼
 rumble motors
```

Validated prototype topology uses an external IRLZ44N-based low-side driver stage. The motor power rail feeds the motors; GPIO17 drives only the MOSFET gate/control path. Grounds are common.

The hardware stage must include appropriate gate biasing, flyback/transient protection and current-rated wiring.

Firmware values:

- PWM frequency: 175 Hz
- resolution: 8-bit
- intensity: 0â€“255
- watchdog: 500 ms
- hardware apply interval: 10 ms

## BTS7960 protection

The validated prototype includes a `1.5KE24CA` TVS across the BTS7960 motor outputs (`M+` / `M-`).

## Buttons

Both buttons use internal pull-ups.

```text
GPIO13 â”€â”€ trigger switch â”€â”€ GND
GPIO14 â”€â”€ profile switch â”€â”€ GND
```

GPIO14 behavior:

- short press: next profile
- long press (~1 s): toggle `HAPTIC_ONLY` / `TRIGGER_FALLBACK`

GPIO4 is not used by the final two-button design.

## OLED

```text
GPIO21 â”€â”€ SDA
GPIO22 â”€â”€ SCL
GND    â”€â”€ GND
VCC    â”€â”€ supply appropriate for the exact OLED module
```

The ESP32 I2C logic is 3.3 V. Verify the power and pull-up arrangement of the exact OLED breakout used.

## WS2812B LEDs

GPIO16 drives two addressable LEDs. They indicate Bluetooth and haptic/profile state.

## Power architecture

Treat logic power and actuator power as separate engineering responsibilities even if both originate from one battery/source.

Recommended structure:

```text
battery / main supply
       â”‚
       â”œâ”€â”€ regulator / logic rail â”€â”€ ESP32 + OLED + logic
       â”‚
       â”œâ”€â”€ recoil rail â”€â”€ BTS7960 â”€â”€ solenoid
       â”‚
       â””â”€â”€ rumble rail â”€â”€ motors + MOSFET stage

all required control stages share the correct reference ground
```

Do not select wire gauge, fuse size, regulator rating or connector rating from firmware values alone; size them from measured actuator current.