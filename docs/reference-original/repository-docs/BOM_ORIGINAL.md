# Bill of materials

This BOM lists parts implied by the recovered firmware and parts required to make those signals electrically usable. It does not assign unverified voltages, currents, or exact models.

## Reference build

| Component | Quantity | Role | Exact model | Status |
|---|---:|---|---|---|
| Classic ESP32 board with Bluetooth Classic SPP | 1 | Main controller, Bluetooth protocol, PWM, LED control | To be confirmed | Required by firmware; exact module unknown |
| BTS7960 H-bridge / motor-driver module | 1 | High-current bidirectional drive for recoil actuator | To be confirmed | Referenced by firmware; exact board and EN wiring unknown |
| Recoil solenoid / linear actuator | 1 | Mechanical recoil impulse | To be confirmed | Implied by code/comments; voltage, current, resistance unknown |
| Rumble actuator / LRA / vibration motor | 1 | Continuous vibration effect | To be confirmed | Actuator terminology differs between variants |
| Rumble power-driver stage | 1 | Interfaces GPIO17 PWM to the rumble actuator | To be confirmed | Electrically required for a nontrivial load; absent from recovered documentation |
| WS2812B-compatible RGB LED | 2 | Bluetooth and haptic status | To be confirmed | Quantity and data pin confirmed; package and wiring unknown |
| Logic power supply / regulator | 1 or more | Stable power for ESP32 and logic peripherals | To be confirmed | Voltage, topology, and rating unknown |
| Solenoid power supply | 1 | Supplies BTS7960 and recoil actuator current | To be confirmed | May share an upstream source only if engineered correctly; details unknown |
| Rumble power supply or suitable rail | 1 | Supplies rumble actuator current | To be confirmed | Could be a regulated rail; details unknown |
| Fuse and holder | As required | Protects actuator power wiring | To be confirmed | Recommended; not observed in source |
| Inductive-transient protection network | As required | Limits actuator switching transients | To be confirmed | Diode/snubber/TVS choice depends on real H-bridge topology |
| Bulk and local decoupling capacitors | As required | Reduces supply dips and switching noise | To be confirmed | Recommended; values must be engineered |
| Power and signal wiring | As required | Carries logic and actuator current | To be confirmed | Gauge, insulation, colors, and routing unknown |
| Polarized connectors / terminals | As required | Serviceable, safe interconnection | To be confirmed | Current and voltage ratings unknown |
| Mechanical solenoid mount and recoil linkage | 1 set | Transfers actuator motion safely | To be confirmed | Not represented in recovered source |

## Trigger-experimental additions

| Component | Quantity | Role | Exact model | Status |
|---|---:|---|---|---|
| Trigger switch | 1 | Active-low local fire input on recovered GPIO15 | To be confirmed | Firmware feature; GPIO assignment unsafe until board is known |
| Profile button | 1 | Active-low profile selection on GPIO4 | To be confirmed | Firmware feature; physical wiring unknown |
| Mode button | 1 | Active-low semi/auto selection on recovered GPIO8 | To be confirmed | Firmware feature; GPIO8 likely unavailable on common WROOM modules |
| SSD1306 OLED, 128×64, I2C address `0x3C` | 1 | Shows profile, fire mode, and Bluetooth state | To be confirmed | Controller/resolution/address confirmed; module and I2C pins unknown |
| Buzzer or sounder | 1 | Profile/mode acknowledgement tones on recovered GPIO9 | To be confirmed | Type and driver unknown; GPIO9 likely unavailable on common WROOM modules |
| Switch connectors / harness | As required | Connects trigger and buttons | To be confirmed | Recommended; pinout unknown |
| OLED connector / harness | 1 | I2C and power connection | To be confirmed | SDA/SCL and supply unknown |

## Procurement and validation notes

- Do not buy an ESP32-S2/S3/C3 as a drop-in replacement solely because it is newer; this firmware depends on Bluetooth Classic SPP.
- Select the BTS7960 module, solenoid, power supply, fuse, wiring, and connectors as one current-rated power system.
- Measure the solenoid coil resistance and current before selecting a supply or protection components.
- Identify whether the rumble device is an LRA, ERM motor, or another actuator before selecting its driver and PWM strategy.
- Confirm the physical prototype before ordering replacement switch, OLED, buzzer, LED, or connector parts.
- Every `To be confirmed` entry is **To be confirmed on the physical prototype.**
