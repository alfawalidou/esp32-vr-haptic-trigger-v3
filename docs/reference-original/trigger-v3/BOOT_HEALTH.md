# Trigger V3 boot health check

The boot health check reports software readiness without turning output-only GPIO configuration into false physical detection. It prints during startup, then shows the same entries through the two historical WS2812 LEDs with a nonblocking state machine. Bluetooth and ForceTube processing continue while the LED sequence is active.

## Strict status contract

| Status | Exact meaning |
|---|---|
| `DETECTED` | A supported physical probe received a positive response. Only an I2C ACK at address `0x3C` currently qualifies. The ACK proves an address responder exists; it does not prove the display model or pixel operation. |
| `CONFIGURED` | Firmware successfully initialized a pin, interface, parser, or service. It is not proof that external hardware is installed. |
| `RESERVED` | The GPIO/capability is retained but is optional, gated, or not declared physically validated. It is not a physical detection result. |
| `NOT DETECTED` | A supported physical probe was attempted and received no response. It is currently valid only for no ACK at `0x3C`. |
| `ERROR` | Software initialization failed. Optional-component failure does not block baseline startup. |

Trigger, Profile, Mode, Buzzer, WS2812, Rumble, BTS7960, and Solenoid have no electrical feedback input in the documented design. They must never be reported as `DETECTED` or `NOT DETECTED`. Bluetooth SPP and ForceTube are software services, so readiness is `CONFIGURED`, not physical `DETECTED`.

## Boot order and safe electrical state

1. Configure GPIO17, GPIO23, GPIO5, and GPIO27 as outputs and drive them LOW.
2. Initialize Serial.
3. Initialize the two GPIO16 WS2812 LEDs.
4. Initialize GPIO13 Trigger, GPIO4 Profile, and GPIO14 Mode as `INPUT_PULLUP` with the boot-release guard.
5. Retain GPIO27 Buzzer LOW.
6. Initialize I2C GPIO21/GPIO22 and probe OLED `0x3C`.
7. Initialize `HapticController`; its first writes remain rumble 0 and BTS7960 `(0,0)`.
8. Initialize Bluetooth Classic SPP and the ForceTube parser.
9. Start the nonblocking WS2812 health sequence.
10. Enter the normal nonblocking loop.

`trigger-v3-safe` cannot arm the outputs at step 7 because the compile-time actuator gate is false. A PWM initialization error also prevents arming in either build.

## OLED probe

I2C starts on GPIO21/GPIO22 at 400 kHz with a 20 ms bus timeout. The firmware probes only `0x3C` with `beginTransmission()` / `endTransmission()`. No ACK skips SSD1306 allocation/initialization and continues headless. An ACK followed by successful SSD1306 initialization is `DETECTED`; an ACK followed by driver failure is `ERROR`.

## Expected Serial entries

Each entry uses:

```text
[HEALTH] <component>              <STATUS>     <detail>
```

| Component | `trigger-v3-safe` | `trigger-v3-compat` | Honest basis |
|---|---|---|---|
| WS2812 GPIO16 | `CONFIGURED` | `CONFIGURED` | Two outputs configured; physical LEDs not detectable |
| Trigger GPIO13 | `RESERVED` | `RESERVED` | Optional `INPUT_PULLUP` plus boot-release guard; switch not detectable |
| Profile GPIO4 | `RESERVED` | `RESERVED` | Optional `INPUT_PULLUP`; switch not detectable |
| Mode GPIO14 | `RESERVED` | `RESERVED` | Optional `INPUT_PULLUP`; switch not detectable |
| Buzzer GPIO27 | `RESERVED` | `RESERVED` | Optional output starts LOW; buzzer not detectable |
| I2C GPIO21/22 | `CONFIGURED` | `CONFIGURED` | Controller/bus configured; wiring not detectable |
| OLED I2C `0x3C` | `DETECTED`, `NOT DETECTED`, or `ERROR` | same | Actual ACK probe; `ERROR` only after ACK plus driver failure |
| Rumble GPIO17 | `RESERVED` | `CONFIGURED` or `ERROR` | SAFE-gated, or PWM initialized/failed; motor not detectable |
| BTS7960 GPIO5/23 | `RESERVED` | `CONFIGURED` or `ERROR` | SAFE-gated, or controller initialized/failed; driver not detectable |
| Solenoid | `RESERVED` | `CONFIGURED` or `ERROR` | SAFE-gated, or controller initialized/failed; coil not detectable |
| Bluetooth SPP | `CONFIGURED` or `ERROR` | same | Local stack/server initialization only |
| ForceTube protocol | `CONFIGURED` | `CONFIGURED` | Fixed streaming parser ready |

There are no other BootHealth components.

## WS2812 health sequence

- LED 0 is red while Bluetooth is disconnected and blue while connected.
- LED 1 shows each health entry for 75 ms.
- green = `DETECTED`;
- blue = `CONFIGURED`;
- orange = `RESERVED`;
- purple = `NOT DETECTED`;
- red = `ERROR`.

After the sequence, LED 0 returns to connection state. LED 1 shows active kick/rumble when available, otherwise orange in SAFE or the selected profile color in COMPAT. The health sequence calls no `delay()`.

## Optional-input boot guard

GPIO13, GPIO4, and GPIO14 require a stable released HIGH state before accepting a debounced LOW press. If an input is LOW during reset, that state is ignored; returning HIGH only arms future detection. A fresh HIGH-to-LOW transition is required. This applies equally to trigger, profile, short Mode, and long Mode interaction.

The startup state is always:

```text
Operating mode: HAPTIC_ONLY
Fire mode: SINGLE
Profile: PISTOL
```

In HAPTIC_ONLY, a valid trigger edge is diagnostic only. In SAFE, even a valid Trigger Fallback request is blocked by the shared actuator gate.
