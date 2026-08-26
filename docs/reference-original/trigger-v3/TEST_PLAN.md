# Trigger V3 test plan

This plan separates software verification, unpowered checks, SAFE validation, and powered COMPAT validation. A successful build is not permission to flash or energize hardware.

## 0. Recovery and known evidence

A full flash backup of the known-good prototype is kept locally by the owner for emergency restoration on the original ESP32 board. It is not a development input and must not be added to Git.

The first earlier `trigger-v2-safe` firmware was successfully flashed on the real prototype and confirmed ESP32-D0WD-V3, 4 MB flash, Bluetooth SPP initialization, ForceTube parser readiness, historical LEDs, and disabled actuators. That evidence does not physically validate the new trigger, buttons, OLED, buzzer, SINGLE, AUTO, or powered COMPAT behavior.

## 1. Build-only gate

- Build `trigger-v3-safe` and `trigger-v3-compat` from the same revision.
- Confirm classic `esp32dev`, the pinned Arduino Core ESP32 3.3.11 stack, and Bluetooth Classic SPP.
- Confirm `min_spiffs.csv`, two 1,966,080-byte OTA slots, and no `huge_app` selection.
- Confirm no network library or service is linked by the application dependency graph.
- Confirm no `.bin`, `.pio`, dump, credential, scratch directory, or temporary patch is staged.
- Confirm GPIO17/GPIO23/GPIO5/GPIO16, the kick constants, direct BTS7960 direction, `0/0` stop, and ForceTube bytes remain unchanged.
- Record exact RAM, flash, slot margin, and change from the last network-enabled builds.
- Stop before upload.

Pass: both environments report `SUCCESS`, fit one OTA slot, and the intended source/documentation-only diff is clean.

## 2. Static architecture and SAFE-gate audit

- Search the complete Trigger V3 variant for abandoned network dependencies, services, addresses, assets, and telemetry.
- Confirm `BluetoothSerial`, `ForceTubeProtocolParser`, and `HapticController` remain present.
- Inspect every write to GPIO17, GPIO23, and GPIO5.
- Confirm Bluetooth KICK, Bluetooth RUMBLE, SINGLE, and AUTO reach outputs only through `HapticController`.
- Confirm `setArmed()` requires both `TRIGGER_V3_ACTUATORS_ENABLED` and successful haptic initialization.
- Confirm SAFE nonzero requests are rejected and all allowed SAFE writes to historical actuator pins are zero/off writes.
- Confirm AUTO uses `millis()`, shares the 150 ms controller rate limit, and contains no delay.
- Confirm the loop contains no `delay()`; only the validated 100 µs direction dead time may use `delayMicroseconds()`.
- Confirm the rumble watchdog timestamp changes only on a valid, nonzero RUMBLE command.
- Confirm runtime hot paths use fixed/static storage and no repeated dynamic `String` construction.

Pass: no input, parser, mode, profile, callback, boot path, or watchdog can bypass the SAFE gate.

## 3. Parser software cases

Feed the parser in a test harness or through the Android test application while observing Serial:

- partial header and partial four-byte packet;
- valid KICK: `0x2A 0xB0 0x00 intensity`;
- valid RUMBLE: `0x2A 0xB0 0x01 intensity`;
- two adjacent packets;
- invalid leading bytes followed by a valid packet;
- repeated `0x2A` before a valid `0xB0`;
- invalid channel followed immediately by a new `0x2A 0xB0` header;
- intensity values 0 and 255.

Pass: only complete valid packets emit commands, channel 0 remains KICK, channel 1 remains RUMBLE, and the next header restores synchronization.

## 4. Unpowered wiring review

- Verify every GPIO against the exact board and existing prototype wiring.
- Verify GPIO4, GPIO13, GPIO14, GPIO21, GPIO22, and GPIO27 are available before connecting optional hardware.
- Account for GPIO4 boot-strapping behavior before installing its button.
- Trace BTS7960 RPWM, LPWM, enables, logic supply, grounds, power terminals, and inductive protection.
- Identify the GPIO17 rumble driver stage; never drive a motor directly from the ESP32.
- Measure solenoid coil resistance without powering it.
- Define fuse, current limit, wiring gauge, connector, clamp, and emergency disconnect.

Pass: reviewed wiring with common ground, no unintended boot-pin state, and a documented current-limited power path.

## 5. First physical validation: SAFE with historical minimal hardware

Flash only after separate explicit owner authorization. Use `trigger-v3-safe` with the existing ESP32, BTS7960 wiring, solenoid, rumble, and two LEDs; the optional GPIO4/GPIO13/GPIO14/GPIO21/GPIO22/GPIO27 devices may remain disconnected.

1. Boot and leave the unit stable for at least 10 minutes.
2. Confirm no reboot, watchdog reset, brownout, or blocking behavior.
3. Confirm `HAPTIC_ONLY`, `SINGLE`, and `PISTOL` at startup.
4. Confirm Bluetooth name `ForceTubeVR 1187883197` is visible.
5. Connect the Android test application.
6. Exercise representative PISTOL, RIFLE, and SNIPER application commands and confirm their resulting ForceTube packets in Serial; application weapon names do not alter packet framing.
7. Send multiple KICK intensities and confirm `[FT]` diagnostics.
8. Send RUMBLE and confirm diagnostics and the 500 ms logical timeout.
9. Confirm no physical kick and no physical rumble for every command.
10. Confirm both historical LEDs remain stable and useful.
11. Confirm no Wi-Fi radio/service from Trigger V3 is visible.
12. Disconnect Bluetooth and confirm parser/output stop diagnostics.

Pass: stable baseline communication and logic with zero actuator movement.

## 6. Optional hardware absence and BootHealth

With optional devices disconnected:

- Trigger, Profile, Mode, and Buzzer report `RESERVED`, physical presence `UNKNOWN`.
- WS2812, I2C, Bluetooth, and ForceTube report software configuration, not physical detection.
- OLED reports `NOT DETECTED` after no ACK at `0x3C` and boot continues promptly.
- SAFE Rumble, BTS7960, and Solenoid report `RESERVED`.
- Bluetooth and ForceTube continue without OLED, buzzer, or buttons.

Pass: no false `DETECTED` claim, crash, boot delay, false press, or false shot.

## 7. Trigger GPIO13: SAFE first

- Boot with GPIO13 released: no trigger event before a real press.
- Boot with GPIO13 held LOW: no event until stable release followed by a new press.
- Verify debounced `[TRIGGER] pressed` and `[TRIGGER] released` events.
- In `HAPTIC_ONLY`, press repeatedly: diagnostics only, no local KICK request.
- Short-press GPIO14 to select `TRIGGER_FALLBACK`.
- In `SINGLE`, one press produces one local request; holding produces no repeats.
- Release and press again to obtain the next request.
- Long-press GPIO14 for at least 800 ms to select `AUTO`.
- In `AUTO`, verify one immediate request then requests at 150 ms while held, without blocking Bluetooth traffic.
- Change mode or fire mode while holding the trigger; verify local fire remains inhibited until release/repress.
- Select LASER and confirm no local output request is sent to the haptic controller.
- Confirm all SAFE requests remain physically inhibited.

Pass: deterministic edges, no boot shot, correct mode separation, and zero movement.

## 8. Profile and Mode buttons

GPIO4 Profile:

- verify 35 ms debounce and exactly one profile step per press;
- verify sequence `PISTOL -> SNIPER -> M16 -> P90 -> LASER -> PISTOL`;
- verify operation and Serial output without OLED or buzzer;
- verify profile changes never request a kick.

GPIO14 Mode:

- short press toggles `HAPTIC_ONLY` / `TRIGGER_FALLBACK` once;
- long press of at least 800 ms toggles `SINGLE` / `AUTO` once and does not also toggle operating mode;
- verify no unintended change at boot or from switch bounce;
- verify Bluetooth remains connected and parsing continues through every change.

Pass: unambiguous short/long interaction with no haptic side effect.

## 9. OLED and buzzer

OLED absent: confirm normal headless boot and `NOT DETECTED`.

OLED present at `0x3C`:

- confirm ACK-based `DETECTED` and successful SSD1306 initialization;
- confirm BT state, operating mode, profile, fire mode, and SAFE/COMPAT state;
- confirm changes appear no faster than the 250 ms display cadence;
- exercise Bluetooth packets during screen updates and check for no parser loss or disconnect.

Buzzer absent: confirm no change in behavior or health claim.

Buzzer present:

- verify short feedback on profile, operating-mode, and fire-mode changes;
- verify tone stops after 60 ms without blocking the loop;
- verify no boot tone and LOW idle state.

Pass: optional UI cannot gate or destabilize Bluetooth/haptics.

## 10. COMPAT rumble test — current limited

Proceed only after SAFE passes and the driver is electrically reviewed.

- Build and manually flash `trigger-v3-compat` only after separate authorization.
- Keep the solenoid power path disconnected.
- Measure GPIO17 at 175 Hz and verify 8-bit duty response at low intensities first.
- Verify zero command stops rumble.
- Send one nonzero RUMBLE command and no refresh; verify stop at approximately 500 ms.
- Continue valid refresh packets; verify the watchdog does not stop active refreshed rumble.
- Stop refresh; verify the timeout is measured from the last valid command, not continuously extended by active state.
- Verify Bluetooth disconnect stops rumble immediately.

Pass: bounded output, corrected watchdog, no reset, and acceptable driver temperature.

## 11. COMPAT solenoid test — isolated single pulse

- Mechanically secure the actuator and keep body parts clear.
- Use a fused, current-limited actuator supply and emergency disconnect.
- Verify RPWM/LPWM idle is `(0,0)` before connecting the solenoid.
- Confirm GPIO5/LPWM is the intended forward direction before increasing power.
- Begin with one low-intensity Bluetooth KICK; measure pulse width, current, supply dip, and return.
- Verify mapped PWM remains 215–255 and forward duration approximately 30 ms.
- Verify reverse duration remains 0 ms and both inputs return to zero.
- Send faster commands and verify no accepted physical kicks closer than 150 ms and no pulse overlap.
- Compare several received intensities with the historical prototype behavior.
- Repeat with local `SINGLE`; validate `AUTO` only after a safe cadence is physically established.

Pass: one bounded pulse per accepted request, reliable stop, no brownout, overheating, reverse motion, or binding.

## 12. Long-duration Bluetooth priority

- Run extended Android/Quest Bluetooth sessions in both operating modes.
- Mix partial/invalid/adjacent packets, KICK, refreshed RUMBLE, local button activity, OLED updates, and AUTO in SAFE first.
- Monitor connection transitions, packet latency, heap stability, watchdogs, and Serial volume.
- Repeat COMPAT only after all current-limited stages pass.

Pass: Bluetooth remains the priority runtime path with no blocking loop or accumulating allocation.

## Remaining physical-validation boundary

Compilation and static review cannot validate optional-device wiring, switch polarity, OLED signal integrity, buzzer audibility, Bluetooth RF stability, driver electronics, solenoid direction/current, rumble duty/current, power integrity, mechanical cadence, or final profile feel. Every profile intensity and the 150 ms AUTO cadence remain **TUNING REQUIRED** until measured on the real prototype.
