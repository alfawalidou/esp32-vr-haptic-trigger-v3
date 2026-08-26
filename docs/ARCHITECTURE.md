# Firmware architecture

Trigger V3 is intentionally non-blocking in its main runtime paths.

## Main components

### `ForceTubeProtocol`

Streaming four-byte packet parser. It recognizes the ForceTube-compatible header and emits KICK/RUMBLE commands.

### `HapticController`

Owns physical output state:

- recoil forward state
- short reverse state
- idle state
- rumble output
- rumble watchdog
- overlap inhibition

A new non-zero KICK is not queued while recoil is active.

### Local controls

GPIO13 is the trigger. GPIO14 combines profile selection and operating-mode toggle.

Local profile behavior is kept separate from the incoming Bluetooth protocol so APK haptic values remain authoritative when using Bluetooth.

### OLED UI

The OLED is optional at boot. Address `0x3C` is probed; if no ACK is received, the firmware continues without the display.

### Boot health

Startup reports configuration/detection status for:

- WS2812
- trigger/profile pins
- buzzer
- I2C
- OLED
- rumble
- BTS7960 / solenoid
- Bluetooth SPP
- ForceTube parser

### Timing values

| Function | Value |
|---|---:|
| recoil forward | 30 ms |
| recoil reverse | 2 ms |
| reverse level | 25% |
| rumble watchdog | 500 ms |
| rumble hardware apply interval | 10 ms |
| local charge rumble refresh | 100 ms |
| SNIPER rumble pulse | 120 ms |
| button debounce | 35 ms |
| profile long press | 1000 ms |
| OLED refresh | 100 ms |
| LED refresh | 20 ms |
| Bluetooth RX budget | 256 bytes / loop |

## Concurrency rule

Bluetooth callbacks only update connection state flags. Heavy haptic/LED/UI work is handled from the normal runtime path rather than performing complex output operations directly inside the SPP callback.