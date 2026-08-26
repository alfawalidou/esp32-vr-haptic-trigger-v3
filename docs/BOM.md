# Bill of materials

This is a practical BOM for the current Trigger V3 prototype. Exact current/voltage ratings must be selected from the real actuators and power source.

| Item | Qty | Role |
|---|---:|---|
| Classic ESP32 DevKit with Bluetooth Classic SPP | 1 | controller |
| BTS7960 H-bridge module | 1 | recoil solenoid driver |
| 1564B-class recoil solenoid / linear actuator | 1 | recoil |
| IRLZ44N N-channel MOSFET | 1 | rumble low-side driver |
| Xbox-style ERM rumble motors | 2 | vibration |
| Flyback / transient protection for rumble stage | as required | motor switching protection |
| `1.5KE24CA` TVS | 1 | BTS7960 motor-output transient clamp |
| WS2812B-compatible RGB LEDs | 2 | status |
| SSD1306 OLED 128Ã—64 I2C `0x3C` | 1 | display |
| Momentary trigger switch | 1 | GPIO13 |
| Momentary profile/mode switch | 1 | GPIO14 |
| Buzzer / sounder | 1 | GPIO27 |
| Suitable actuator power source | 1 | recoil + rumble power |
| Logic regulator / buck as required | 1 | ESP32/peripherals |
| Gate resistor / gate pull-down | as required | MOSFET gate conditioning |
| Fuse / protection | as required | power safety |
| Current-rated wiring/connectors | as required | power and signal harness |

Before a public hardware release, record measured solenoid current, rumble motor current, supply voltage, fuse choice, MOSFET resistor values and connector ratings.