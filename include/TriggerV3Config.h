/*
 * ESP32 VR Haptic Trigger V3
 * Original project by alfawalidou / McWall
 * GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
 * Discord: .mcwall | Telegram: @McWall07
 *
 * Copyright 2026 alfawalidou / McWall
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <Arduino.h>

#ifndef TRIGGER_V3_ACTUATORS_ENABLED
#define TRIGGER_V3_ACTUATORS_ENABLED 0
#endif

#ifndef TRIGGER_V3_SERIAL_TELEMETRY
#define TRIGGER_V3_SERIAL_TELEMETRY 1
#endif

namespace trigger_v3 {
namespace config {

// Hardware-tested working baseline outputs.
static constexpr uint8_t kRumblePin = 17;
static constexpr uint8_t kSolenoidRpwmPin = 23;
static constexpr uint8_t kSolenoidLpwmPin = 5;
static constexpr uint8_t kLedDataPin = 16;

// Optional Trigger V3 inputs and local UI.
static constexpr uint8_t kProfileButtonPin = 14;
static constexpr uint8_t kTriggerPin = 13;
static constexpr uint8_t kBuzzerPin = 27;
static constexpr uint8_t kOledSdaPin = 21;
static constexpr uint8_t kOledSclPin = 22;

static constexpr uint8_t kLedCount = 2;
static constexpr uint8_t kConnectionLedIndex = 0;
static constexpr uint8_t kStatusLedIndex = 1;
static constexpr uint8_t kLedBrightness = 50;

static constexpr uint32_t kRumbleFrequencyHz = 175;
static constexpr uint8_t kPwmResolutionBits = 8;
static constexpr uint8_t kSolenoidMinPwm = 215;
static constexpr uint8_t kSolenoidMaxPwm = 255;
static constexpr uint8_t kReverseKickPercent = 25;
static constexpr uint32_t kKickDurationMs = 30;

// Physically validated on the current 1564B + BTS7960 prototype.
// A 2 ms reverse pulse at 25% gives a clean return between kicks.
static constexpr uint32_t kReverseDurationMs = 2;
static constexpr uint32_t kRumbleTimeoutMs = 500;
// Bluetooth commands are parsed immediately, but physical rumble PWM is
// applied at a bounded rate so fast RUMBLE 255/0 streams cannot hammer LEDC.
static constexpr uint32_t kRumbleHardwareApplyIntervalMs = 10;
static constexpr uint32_t kLocalChargeRumbleRefreshMs = 100;
// SINGLE profiles use a short rumble impulse independent of trigger hold time.
// AUTO profiles keep rumble active while the trigger is held.
static constexpr uint32_t kSingleRumblePulseMs = 120;
static constexpr uint32_t kButtonDebounceMs = 35;
static constexpr uint32_t kProfileLongPressMs = 1000;
static constexpr uint32_t kSerialRumbleTelemetryIntervalMs = 1000;
// ForceTube automatic weapons can emit KICK/KICK0 packets very quickly.
// Keep serial diagnostics useful without letting Serial.printf() dominate the loop.
static constexpr uint32_t kSerialKickTelemetryIntervalMs = 100;
// Bound work per loop so Bluetooth is drained continuously while the scheduler,
// haptic state machine, OLED and watchdogs still get CPU time.
static constexpr uint16_t kBluetoothRxByteBudgetPerLoop = 256;
static constexpr uint32_t kDisplayRefreshMs = 100;
static constexpr uint32_t kKickUiLatchMs = 150;
static constexpr uint32_t kFtLiveHoldMs = 1500;
static constexpr uint32_t kFtRateResetMs = 1500;
static constexpr uint8_t kFtRateSampleCount = 4;
static constexpr uint32_t kLedRefreshMs = 20;
static constexpr uint32_t kBootHealthLedStepMs = 75;
static constexpr uint32_t kI2cTimeoutMs = 20;
static constexpr uint32_t kI2cClockHz = 400000;

static constexpr uint16_t kProfileBuzzerFrequencyHz = 1000;
static constexpr uint16_t kModeBuzzerFrequencyHz = 1200;
static constexpr uint32_t kBuzzerFeedbackMs = 60;

static constexpr uint8_t kOledWidth = 128;
static constexpr uint8_t kOledHeight = 64;
static constexpr uint8_t kOledAddress = 0x3C;

static constexpr char kBluetoothName[] = "ForceTubeVR 1187883197";
static constexpr bool kActuatorsEnabled = TRIGGER_V3_ACTUATORS_ENABLED != 0;

static_assert(kTriggerPin != kProfileButtonPin, "Trigger and profile pins must differ");
static_assert(kBuzzerPin != kLedDataPin, "Buzzer and LED pins must differ");
static_assert(kOledSdaPin != kOledSclPin, "I2C pins must differ");
static_assert(
  kLocalChargeRumbleRefreshMs > 0 && kLocalChargeRumbleRefreshMs < kRumbleTimeoutMs,
  "Local charge rumble refresh must run before the rumble watchdog"
);

}  // namespace config
}  // namespace trigger_v3
