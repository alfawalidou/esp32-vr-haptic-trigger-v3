/*
 * ESP32 VR Haptic Trigger V3
 * Original project by alfawalidou / McWall
 * GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
 * Discord: .mcwall | Telegram: @McWall07
 *
 * Copyright 2026 alfawalidou / McWall
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Arduino.h>
#include <BluetoothSerial.h>
#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "BootHealth.h"
#include "ForceTubeProtocol.h"
#include "HapticController.h"
#include "HapticProfiles.h"
#include "ProjectIdentity.h"
#include "TriggerV3Config.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled for this ESP32 target.
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Bluetooth Classic SPP is required; use a compatible classic ESP32 target.
#endif

#if TRIGGER_V3_SERIAL_TELEMETRY
#define TV3_TELEMETRYF(...) Serial.printf(__VA_ARGS__)
#else
#define TV3_TELEMETRYF(...) do { } while (0)
#endif

namespace {

using trigger_v3::BootHealthReporter;
using trigger_v3::ForceTubeProtocolParser;
using trigger_v3::HapticChannel;
using trigger_v3::HapticCommand;
using trigger_v3::HapticController;
using trigger_v3::HealthStatus;
namespace config = trigger_v3::config;
namespace identity = trigger_v3::identity;
namespace profiles = trigger_v3::profiles;

enum class OperatingMode : uint8_t {
  HapticOnly,
  TriggerFallback,
};

enum class ButtonEvent : uint8_t {
  None,
  Pressed,
  Released,
};

class DebouncedButton {
 public:
  explicit DebouncedButton(uint8_t pin) : pin_(pin) {}

  void begin() {
    pinMode(pin_, INPUT_PULLUP);
    rawState_ = digitalRead(pin_);
    stableState_ = HIGH;
    rawChangedAt_ = millis();
    inputReleased_ = rawState_ == HIGH;
    validPressActive_ = false;
  }

  ButtonEvent update(uint32_t now) {
    const bool reading = digitalRead(pin_);
    if (reading != rawState_) {
      rawState_ = reading;
      rawChangedAt_ = now;
    }

    if (stableState_ == rawState_ ||
        static_cast<uint32_t>(now - rawChangedAt_) < config::kButtonDebounceMs) {
      return ButtonEvent::None;
    }

    stableState_ = rawState_;
    if (stableState_ == HIGH) {
      const bool validRelease = validPressActive_;
      inputReleased_ = true;
      validPressActive_ = false;
      return validRelease ? ButtonEvent::Released : ButtonEvent::None;
    }

    if (!inputReleased_) {
      validPressActive_ = false;
      return ButtonEvent::None;
    }

    inputReleased_ = false;
    validPressActive_ = true;
    return ButtonEvent::Pressed;
  }

  bool isPressed() const {
    return stableState_ == LOW && validPressActive_;
  }

 private:
  uint8_t pin_;
  bool rawState_ = HIGH;
  bool stableState_ = HIGH;
  bool inputReleased_ = false;
  bool validPressActive_ = false;
  uint32_t rawChangedAt_ = 0;
};

struct DisplayUiState {
  bool localKickLatched = false;
  bool ftLiveActive = false;
  bool ftKickLatched = false;
  bool hasPreviousFtKick = false;

  uint8_t localKickIntensity = 0;
  uint8_t ftKickIntensity = 0;
  uint8_t ftRumbleIntensity = 0;
  uint8_t manualRumbleIntensity = 0;
  uint8_t ftIntervalCount = 0;
  uint8_t ftIntervalIndex = 0;

  uint32_t localKickStartedAt = 0;
  uint32_t ftKickStartedAt = 0;
  uint32_t lastFtCommandAt = 0;
  uint32_t lastFtKickAt = 0;
  uint32_t ftKickIntervals[config::kFtRateSampleCount] = {};
};

BluetoothSerial gBluetooth;
ForceTubeProtocolParser gProtocol;
HapticController gHaptics;
CRGB gLeds[config::kLedCount];
Adafruit_SSD1306 gDisplay(config::kOledWidth, config::kOledHeight, &Wire, -1);
BootHealthReporter gBootHealth(
  gLeds,
  config::kConnectionLedIndex,
  config::kStatusLedIndex
);

DebouncedButton gProfileButton(config::kProfileButtonPin);
DebouncedButton gTriggerButton(config::kTriggerPin);
DisplayUiState gDisplayUi;

volatile bool gBluetoothConnected = false;
volatile bool gBluetoothStateChanged = false;
bool gBluetoothReady = false;
bool gDisplayReady = false;
bool gDisplayDirty = true;
bool gBuzzerActive = false;
bool gProfileLongPressHandled = false;
bool gLocalFireCycleActive = false;
bool gLocalChargeActive = false;
bool gLocalChargeOwnsRumble = false;
bool gLocalProfileOwnsRumble = false;
OperatingMode gOperatingMode = OperatingMode::HapticOnly;
size_t gProfileIndex = profiles::kDefaultProfileIndex;
uint32_t gLastDisplayRefreshAt = 0;
uint32_t gLastLedRefreshAt = 0;
uint32_t gLastAutoKickRequestAt = 0;
uint32_t gLocalChargeStartedAt = 0;
uint32_t gLastLocalChargeRumbleAt = 0;
uint32_t gLastLocalProfileRumbleAt = 0;
uint32_t gLocalProfileRumbleStartedAt = 0;
uint32_t gProfilePressedAt = 0;
uint32_t gBuzzerStartedAt = 0;
uint32_t gBuzzerDurationMs = 0;
uint32_t gAuthorSplashUntil = 0;
uint8_t gLastLocalChargeRumbleIntensity = 0;
uint8_t gLocalProfileRumbleIntensity = 0;

#if TRIGGER_V3_SERIAL_TELEMETRY
uint32_t gLastRumbleTelemetryAt = 0;
uint8_t gLastRumbleTelemetryIntensity = 0xFF;
uint32_t gLastKickTelemetryAt = 0;
uint8_t gLastKickTelemetryIntensity = 0xFF;
#endif

const profiles::HapticProfile& currentProfile() {
  return profiles::kHapticProfiles[gProfileIndex];
}

const char* operatingModeName(OperatingMode mode) {
  switch (mode) {
    case OperatingMode::HapticOnly:
      return "HAPTIC_ONLY";
    case OperatingMode::TriggerFallback:
      return "TRIGGER_FALLBACK";
  }
  return "UNKNOWN";
}

const char* profileBehaviorName(profiles::ProfileBehavior behavior) {
  switch (behavior) {
    case profiles::ProfileBehavior::Single:
      return "SINGLE";
    case profiles::ProfileBehavior::Auto:
      return "AUTO";
    case profiles::ProfileBehavior::ChargeRelease:
      return "CHARGE";
  }
  return "UNKNOWN";
}

void recordLocalKickUi(uint8_t intensity, uint32_t now) {
  gDisplayUi.localKickLatched = true;
  gDisplayUi.localKickIntensity = intensity;
  gDisplayUi.localKickStartedAt = now;
  gDisplayDirty = true;
}

void resetFtRateEstimate() {
  gDisplayUi.hasPreviousFtKick = false;
  gDisplayUi.ftIntervalCount = 0;
  gDisplayUi.ftIntervalIndex = 0;
}

void recordForceTubeUiCommand(const HapticCommand& command, uint32_t now) {
  gDisplayUi.ftLiveActive = true;
  gDisplayUi.lastFtCommandAt = now;

  if (command.channel == HapticChannel::Kick && command.intensity > 0) {
    if (gDisplayUi.hasPreviousFtKick) {
      const uint32_t intervalMs =
        static_cast<uint32_t>(now - gDisplayUi.lastFtKickAt);
      if (intervalMs >= config::kFtRateResetMs) {
        gDisplayUi.ftIntervalCount = 0;
        gDisplayUi.ftIntervalIndex = 0;
      } else if (intervalMs > 0) {
        gDisplayUi.ftKickIntervals[gDisplayUi.ftIntervalIndex] = intervalMs;
        gDisplayUi.ftIntervalIndex = static_cast<uint8_t>(
          (gDisplayUi.ftIntervalIndex + 1) % config::kFtRateSampleCount
        );
        if (gDisplayUi.ftIntervalCount < config::kFtRateSampleCount) {
          ++gDisplayUi.ftIntervalCount;
        }
      }
    }

    gDisplayUi.hasPreviousFtKick = true;
    gDisplayUi.lastFtKickAt = now;
    gDisplayUi.ftKickLatched = true;
    gDisplayUi.ftKickIntensity = command.intensity;
    gDisplayUi.ftKickStartedAt = now;
  } else if (command.channel == HapticChannel::Rumble) {
    gDisplayUi.ftRumbleIntensity = command.intensity;
  }

  gDisplayDirty = true;
}

void onBluetoothEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t*) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    gBluetoothConnected = true;
    gBluetoothStateChanged = true;
  } else if (event == ESP_SPP_CLOSE_EVT) {
    gBluetoothConnected = false;
    gBluetoothStateChanged = true;
  }
}

void startBuzzerFeedback(uint16_t frequencyHz, uint32_t durationMs, uint32_t now) {
  tone(config::kBuzzerPin, frequencyHz);
  gBuzzerActive = true;
  gBuzzerStartedAt = now;
  gBuzzerDurationMs = durationMs;
}

void updateBuzzer(uint32_t now) {
  if (!gBuzzerActive ||
      static_cast<uint32_t>(now - gBuzzerStartedAt) < gBuzzerDurationMs) {
    return;
  }

  noTone(config::kBuzzerPin);
  digitalWrite(config::kBuzzerPin, LOW);
  gBuzzerActive = false;
}

void resetLocalFireCycleState() {
  gLocalFireCycleActive = false;
  gLocalChargeActive = false;
  gLocalChargeOwnsRumble = false;
  gLocalProfileOwnsRumble = false;
  gLastLocalChargeRumbleIntensity = 0;
  gLocalProfileRumbleIntensity = 0;
}

void cancelLocalFireCycle(uint32_t now) {
  if (gLocalChargeOwnsRumble || gLocalProfileOwnsRumble) {
    gHaptics.setRumble(0, now);
  }
  resetLocalFireCycleState();
}

void traceCurrentProfile() {
  const profiles::HapticProfile& profile = currentProfile();
  switch (profile.behavior) {
    case profiles::ProfileBehavior::Single:
      TV3_TELEMETRYF(
        "[PROFILE] %s behavior=SINGLE kick=%u rumble=%u\n",
        profile.name,
        profile.kickIntensity,
        profile.manualRumbleIntensity
      );
      break;

    case profiles::ProfileBehavior::Auto:
      TV3_TELEMETRYF(
        "[PROFILE] %s behavior=AUTO kick=%u rumble=%u auto_ms=%u\n",
        profile.name,
        profile.kickIntensity,
        profile.manualRumbleIntensity,
        profile.autoRepeatMs
      );
      break;

    case profiles::ProfileBehavior::ChargeRelease:
      TV3_TELEMETRYF(
        "[PROFILE] %s behavior=CHARGE_RELEASE kick=%u charge_steps=%u "
        "charge_ms=%u status=%s\n",
        profile.name,
        profile.kickIntensity,
        profile.chargeStepCount,
        profile.chargeDurationMs,
        profiles::isChargeReleaseConfigured(profile) ? "CONFIGURED" : "UNCALIBRATED"
      );
      break;
  }
}

void applyOperatingMode(OperatingMode mode, uint32_t now) {
  gOperatingMode = mode;
  cancelLocalFireCycle(now);
  gDisplayDirty = true;
  TV3_TELEMETRYF("[MODE] %s\n", operatingModeName(gOperatingMode));
  startBuzzerFeedback(config::kModeBuzzerFrequencyHz, config::kBuzzerFeedbackMs, now);
}

void toggleOperatingMode(uint32_t now) {
  applyOperatingMode(
    gOperatingMode == OperatingMode::HapticOnly
      ? OperatingMode::TriggerFallback
      : OperatingMode::HapticOnly,
    now
  );
}

void cycleProfile(uint32_t now) {
  cancelLocalFireCycle(now);
  gProfileIndex = (gProfileIndex + 1) % profiles::kHapticProfileCount;
  gDisplayDirty = true;
  traceCurrentProfile();
  startBuzzerFeedback(config::kProfileBuzzerFrequencyHz, config::kBuzzerFeedbackMs, now);
}

void traceForceTubeCommand(
  const HapticCommand& command,
  uint32_t now,
  bool outputAccepted
) {
#if TRIGGER_V3_SERIAL_TELEMETRY
  if (command.channel == HapticChannel::Kick) {
    // KICK=0 is extremely frequent in the APK stream and is intentionally
    // non-destructive in HapticController. Do not flood Serial with it.
    if (command.intensity == 0) {
      return;
    }

    const bool changed = command.intensity != gLastKickTelemetryIntensity;
    const bool periodic =
      static_cast<uint32_t>(now - gLastKickTelemetryAt) >=
        config::kSerialKickTelemetryIntervalMs;
    if (!changed && !periodic) {
      return;
    }
    gLastKickTelemetryAt = now;
    gLastKickTelemetryIntensity = command.intensity;
  } else {
    const bool changed = command.intensity != gLastRumbleTelemetryIntensity;
    const bool periodicActive = command.intensity > 0 &&
      static_cast<uint32_t>(now - gLastRumbleTelemetryAt) >=
        config::kSerialRumbleTelemetryIntervalMs;
    if (!changed && !periodicActive) {
      return;
    }
    gLastRumbleTelemetryAt = now;
    gLastRumbleTelemetryIntensity = command.intensity;
  }

  Serial.printf(
    "[FT] %s intensity=%u result=%s\n",
    command.channel == HapticChannel::Kick ? "KICK" : "RUMBLE",
    command.intensity,
    outputAccepted ? "ACCEPTED" : "INHIBITED"
  );
  if (command.intensity > 0 && !outputAccepted && !config::kActuatorsEnabled) {
    Serial.printf(
      "[SAFE] actuator request blocked source=BLUETOOTH_%s\n",
      command.channel == HapticChannel::Kick ? "KICK" : "RUMBLE"
    );
  }
#else
  (void)command;
  (void)now;
  (void)outputAccepted;
#endif
}

void processHapticCommand(const HapticCommand& command, uint32_t now) {
  bool outputAccepted = false;
  if (command.channel == HapticChannel::Kick) {
    const bool kickStarted = gHaptics.requestKick(command.intensity, now);
    outputAccepted = command.intensity == 0 || kickStarted;
  } else {
    gHaptics.setRumble(command.intensity, now);
    outputAccepted = command.intensity == 0 || gHaptics.isArmed();
  }
  traceForceTubeCommand(command, now, outputAccepted);
}

void processBluetooth(uint32_t now) {
  // Never queue ForceTube fire events behind a physical solenoid cycle.
  // Drain SPP continuously; HapticController itself rejects a non-zero KICK
  // while Forward/Reverse is already active. A missed real-time impulse is
  // preferable to replaying it late.
  uint16_t bytesProcessed = 0;
  while (bytesProcessed < config::kBluetoothRxByteBudgetPerLoop &&
         gBluetooth.available()) {
    const int value = gBluetooth.read();
    if (value < 0) {
      break;
    }
    ++bytesProcessed;

    HapticCommand command{};
    if (!gProtocol.feed(static_cast<uint8_t>(value), command)) {
      continue;
    }

    recordForceTubeUiCommand(command, now);
    processHapticCommand(command, now);
  }
}

void updateConnectionState() {
  if (!gBluetoothStateChanged) {
    return;
  }

  gBluetoothStateChanged = false;
  gDisplayDirty = true;
  if (gBluetoothConnected) {
    TV3_TELEMETRYF("[BT] connected\n");
  } else {
    gProtocol.reset();
    gHaptics.stopAll();
    resetLocalFireCycleState();
    TV3_TELEMETRYF("[BT] disconnected; outputs stopped\n");
  }
}

void requestLocalKick(uint32_t now) {
  const profiles::HapticProfile& profile = currentProfile();
  if (profile.kickIntensity == 0) {
    TV3_TELEMETRYF(
      "[TRIGGER] local kick profile=%s intensity=0 result=NO_OUTPUT\n",
      profile.name
    );
    return;
  }

  recordLocalKickUi(profile.kickIntensity, now);
  const bool accepted = gHaptics.requestKick(profile.kickIntensity, now);
  TV3_TELEMETRYF(
    "[TRIGGER] local kick profile=%s intensity=%u result=%s\n",
    profile.name,
    profile.kickIntensity,
    accepted ? "ACCEPTED" : "INHIBITED"
  );
  if (!accepted && !config::kActuatorsEnabled) {
    TV3_TELEMETRYF("[SAFE] actuator request blocked source=TRIGGER\n");
  }
}

void startLocalProfileRumble(const profiles::HapticProfile& profile, uint32_t now) {
  gLocalProfileRumbleIntensity = profile.manualRumbleIntensity;
  gLocalProfileOwnsRumble = gLocalProfileRumbleIntensity > 0;
  gLocalProfileRumbleStartedAt = now;
  gLastLocalProfileRumbleAt = now;
  if (gLocalProfileOwnsRumble) {
    gHaptics.setRumble(gLocalProfileRumbleIntensity, now);
  }
}

void stopLocalProfileRumble(uint32_t now) {
  if (gLocalProfileOwnsRumble) {
    gHaptics.setRumble(0, now);
  }
  gLocalProfileOwnsRumble = false;
  gLocalProfileRumbleIntensity = 0;
}

void refreshLocalProfileRumble(uint32_t now) {
  if (!gLocalProfileOwnsRumble) {
    return;
  }

  const profiles::HapticProfile& profile = currentProfile();

  // SINGLE: one short rumble impulse only. Holding the trigger must not
  // keep the motor running (e.g. SNIPER).
  if (profile.behavior == profiles::ProfileBehavior::Single) {
    if (static_cast<uint32_t>(now - gLocalProfileRumbleStartedAt) >=
        config::kSingleRumblePulseMs) {
      stopLocalProfileRumble(now);
    }
    return;
  }

  // AUTO: keep rumble alive for as long as the trigger remains pressed.
  if (static_cast<uint32_t>(now - gLastLocalProfileRumbleAt) <
      config::kLocalChargeRumbleRefreshMs) {
    return;
  }

  gHaptics.setRumble(gLocalProfileRumbleIntensity, now);
  gLastLocalProfileRumbleAt = now;
}

uint8_t calculateLocalChargeRumble(
  const profiles::HapticProfile& profile,
  uint32_t now
) {
  const uint32_t elapsed = static_cast<uint32_t>(now - gLocalChargeStartedAt);
  uint32_t completedSteps = static_cast<uint32_t>(
    static_cast<uint64_t>(elapsed) * profile.chargeStepCount /
    profile.chargeDurationMs
  ) + 1;
  if (completedSteps > profile.chargeStepCount) {
    completedSteps = profile.chargeStepCount;
  }

  const uint16_t rumbleRange =
    static_cast<uint16_t>(profile.chargeRumbleMax) - profile.chargeRumbleMin;
  return static_cast<uint8_t>(
    profile.chargeRumbleMin +
    rumbleRange * completedSteps / profile.chargeStepCount
  );
}

void updateLocalCharge(uint32_t now) {
  if (!gLocalChargeActive) {
    return;
  }

  const profiles::HapticProfile& profile = currentProfile();
  if (!profiles::isChargeReleaseConfigured(profile)) {
    resetLocalFireCycleState();
    return;
  }

  const uint8_t rumbleIntensity = calculateLocalChargeRumble(profile, now);
  const bool intensityChanged =
    rumbleIntensity != gLastLocalChargeRumbleIntensity;
  const bool refreshDue =
    static_cast<uint32_t>(now - gLastLocalChargeRumbleAt) >=
      config::kLocalChargeRumbleRefreshMs;
  if (!intensityChanged && !refreshDue) {
    return;
  }

  gHaptics.setRumble(rumbleIntensity, now);
  gLocalChargeOwnsRumble = rumbleIntensity > 0;
  gLastLocalChargeRumbleIntensity = rumbleIntensity;
  gLastLocalChargeRumbleAt = now;
}

void startLocalCharge(uint32_t now) {
  const profiles::HapticProfile& profile = currentProfile();
  if (!profiles::isChargeReleaseConfigured(profile)) {
    resetLocalFireCycleState();
    TV3_TELEMETRYF(
      "[PROFILE] CHARGE_RELEASE profile=%s result=UNCALIBRATED\n",
      profile.name
    );
    return;
  }

  gLocalFireCycleActive = true;
  gLocalChargeActive = true;
  gLocalChargeOwnsRumble = false;
  gLocalChargeStartedAt = now;
  gLastLocalChargeRumbleAt = now;
  gLastLocalChargeRumbleIntensity = 0;
  TV3_TELEMETRYF(
    "[PROFILE] CHARGE_RELEASE profile=%s result=CHARGING\n",
    profile.name
  );
  updateLocalCharge(now);
}

void finishLocalCharge(uint32_t now) {
  if (!gLocalChargeActive) {
    resetLocalFireCycleState();
    return;
  }

  const profiles::HapticProfile& profile = currentProfile();
  if (!profiles::isChargeReleaseConfigured(profile)) {
    resetLocalFireCycleState();
    return;
  }

  switch (profile.chargeReleaseOrder) {
    case profiles::ChargeReleaseOrder::StopRumbleThenKick:
      gHaptics.setRumble(0, now);
      gLocalChargeOwnsRumble = false;
      requestLocalKick(now);
      break;

    case profiles::ChargeReleaseOrder::KickThenStopRumble:
      requestLocalKick(now);
      gHaptics.setRumble(0, now);
      gLocalChargeOwnsRumble = false;
      break;

    case profiles::ChargeReleaseOrder::Uncalibrated:
      break;
  }

  resetLocalFireCycleState();
}

void startLocalFireCycle(uint32_t now) {
  const profiles::HapticProfile& profile = currentProfile();
  switch (profile.behavior) {
    case profiles::ProfileBehavior::Single:
      gLocalFireCycleActive = true;
      startLocalProfileRumble(profile, now);
      requestLocalKick(now);
      break;

    case profiles::ProfileBehavior::Auto:
      if (profile.autoRepeatMs == 0) {
        TV3_TELEMETRYF(
          "[PROFILE] AUTO profile=%s result=INVALID_REPEAT\n",
          profile.name
        );
        return;
      }
      gLocalFireCycleActive = true;
      gLastAutoKickRequestAt = now;
      startLocalProfileRumble(profile, now);
      requestLocalKick(now);
      break;

    case profiles::ProfileBehavior::ChargeRelease:
      startLocalCharge(now);
      break;
  }
}

void finishLocalFireCycle(uint32_t now) {
  if (gLocalChargeActive) {
    finishLocalCharge(now);
  } else {
    stopLocalProfileRumble(now);
    resetLocalFireCycleState();
  }
}

void updateActiveLocalFireCycle(uint32_t now) {
  if (gOperatingMode != OperatingMode::TriggerFallback ||
      !gLocalFireCycleActive ||
      !gTriggerButton.isPressed()) {
    return;
  }

  const profiles::HapticProfile& profile = currentProfile();
  switch (profile.behavior) {
    case profiles::ProfileBehavior::Single:
      refreshLocalProfileRumble(now);
      return;

    case profiles::ProfileBehavior::Auto:
      refreshLocalProfileRumble(now);
      if (profile.autoRepeatMs > 0 &&
          static_cast<uint32_t>(now - gLastAutoKickRequestAt) >=
            profile.autoRepeatMs) {
        gLastAutoKickRequestAt = now;
        requestLocalKick(now);
      }
      return;

    case profiles::ProfileBehavior::ChargeRelease:
      updateLocalCharge(now);
      return;
  }
}

void handleProfileLongPress(uint32_t now) {
  gProfileLongPressHandled = true;
  TV3_TELEMETRYF("[PROFILE] long press -> toggle operating mode\n");
  toggleOperatingMode(now);
}

void updateProfileButton(uint32_t now) {
  const ButtonEvent event = gProfileButton.update(now);

  if (event == ButtonEvent::Pressed) {
    gProfilePressedAt = now;
    gProfileLongPressHandled = false;
  }

  if (gProfileButton.isPressed() &&
      !gProfileLongPressHandled &&
      static_cast<uint32_t>(now - gProfilePressedAt) >=
        config::kProfileLongPressMs) {
    handleProfileLongPress(now);
  }

  if (event != ButtonEvent::Released || gProfileLongPressHandled) {
    return;
  }

  if (static_cast<uint32_t>(now - gProfilePressedAt) >=
      config::kProfileLongPressMs) {
    handleProfileLongPress(now);
  } else {
    cycleProfile(now);
  }
}

void updateTrigger(uint32_t now) {
  const ButtonEvent event = gTriggerButton.update(now);
  if (event == ButtonEvent::Pressed) {
    TV3_TELEMETRYF(
      "[TRIGGER] pressed mode=%s profile=%s behavior=%s\n",
      operatingModeName(gOperatingMode),
      currentProfile().name,
      profileBehaviorName(currentProfile().behavior)
    );
    if (gOperatingMode == OperatingMode::TriggerFallback) {
      startLocalFireCycle(now);
    }
  } else if (event == ButtonEvent::Released) {
    finishLocalFireCycle(now);
    TV3_TELEMETRYF("[TRIGGER] released\n");
  }

  updateActiveLocalFireCycle(now);
}

void updateLocalControls(uint32_t now) {
  updateProfileButton(now);
  updateTrigger(now);
}

bool probeI2cAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission(true) == 0;
}

void updateLeds(uint32_t now) {
  if (static_cast<uint32_t>(now - gLastLedRefreshAt) < config::kLedRefreshMs) {
    return;
  }
  gLastLedRefreshAt = now;

  if (gBootHealth.update(now, gBluetoothConnected)) {
    return;
  }

  gLeds[config::kConnectionLedIndex] = gBluetoothConnected ? CRGB::Blue : CRGB::Red;

  const uint8_t kick = gHaptics.kickIntensity();
  const uint8_t rumble = gHaptics.rumbleIntensity();
  if (kick > 0 && rumble > 0) {
    gLeds[config::kStatusLedIndex] = CRGB(kick, 0, rumble);
  } else if (kick > 0) {
    gLeds[config::kStatusLedIndex] = CRGB(kick, 0, 0);
  } else if (rumble > 0) {
    gLeds[config::kStatusLedIndex] = CRGB(0, 0, rumble);
  } else if (!config::kActuatorsEnabled) {
    gLeds[config::kStatusLedIndex] = CRGB::Orange;
  } else {
    const profiles::HapticProfile& profile = currentProfile();
    gLeds[config::kStatusLedIndex] = CRGB(profile.ledRed, profile.ledGreen, profile.ledBlue);
  }
  FastLED.show();
}

const char* displayOperatingModeName() {
  return gOperatingMode == OperatingMode::HapticOnly ? "HAPT" : "TRIG";
}

uint16_t displayTextWidth(const char* text) {
  uint16_t length = 0;
  while (text[length] != '\0') {
    ++length;
  }
  return length * 6;
}

void drawRightAlignedText(const char* text, int16_t y) {
  const uint16_t width = displayTextWidth(text);
  const int16_t x = width < config::kOledWidth
    ? static_cast<int16_t>(config::kOledWidth - width)
    : 0;
  gDisplay.setCursor(x, y);
  gDisplay.print(text);
}

void drawIntensityBar(
  int16_t x,
  int16_t y,
  int16_t width,
  int16_t height,
  uint8_t value
) {
  gDisplay.drawRect(x, y, width, height, SSD1306_WHITE);
  if (value == 0 || width <= 2 || height <= 2) {
    return;
  }

  const int16_t innerWidth = width - 2;
  const int16_t fillWidth = static_cast<int16_t>(
    static_cast<uint32_t>(value) * innerWidth / 255
  );
  if (fillWidth > 0) {
    gDisplay.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
  }
}

void drawIntensityRow(
  const char* label,
  int16_t y,
  uint8_t barValue,
  uint8_t displayedValue
) {
  char valueText[4];
  snprintf(valueText, sizeof(valueText), "%3u", displayedValue);

  gDisplay.setCursor(0, y);
  gDisplay.print(label);
  drawIntensityBar(29, y, 70, 8, barValue);
  gDisplay.setCursor(110, y);
  gDisplay.print(valueText);
}

uint32_t calculateFtRateRpm() {
  if (gDisplayUi.ftIntervalCount == 0) {
    return 0;
  }

  uint64_t intervalTotal = 0;
  for (uint8_t index = 0; index < gDisplayUi.ftIntervalCount; ++index) {
    intervalTotal += gDisplayUi.ftKickIntervals[index];
  }
  const uint32_t averageIntervalMs = static_cast<uint32_t>(
    intervalTotal / gDisplayUi.ftIntervalCount
  );
  return averageIntervalMs > 0 ? 60000UL / averageIntervalMs : 0;
}

void updateDisplayState(uint32_t now) {
  if (gDisplayUi.localKickLatched &&
      static_cast<uint32_t>(now - gDisplayUi.localKickStartedAt) >=
        config::kKickUiLatchMs) {
    gDisplayUi.localKickLatched = false;
    gDisplayUi.localKickIntensity = 0;
    gDisplayDirty = true;
  }

  if (gDisplayUi.ftKickLatched &&
      static_cast<uint32_t>(now - gDisplayUi.ftKickStartedAt) >=
        config::kKickUiLatchMs) {
    gDisplayUi.ftKickLatched = false;
    gDisplayDirty = true;
  }

  if (gDisplayUi.hasPreviousFtKick &&
      static_cast<uint32_t>(now - gDisplayUi.lastFtKickAt) >=
        config::kFtRateResetMs) {
    const bool rateWasVisible = gDisplayUi.ftIntervalCount > 0;
    resetFtRateEstimate();
    if (rateWasVisible) {
      gDisplayDirty = true;
    }
  }

  if (gDisplayUi.ftLiveActive &&
      static_cast<uint32_t>(now - gDisplayUi.lastFtCommandAt) >=
        config::kFtLiveHoldMs) {
    gDisplayUi.ftLiveActive = false;
    gDisplayDirty = true;
  }

  const uint8_t manualRumbleIntensity = gLocalChargeOwnsRumble
    ? gLastLocalChargeRumbleIntensity
    : (gLocalProfileOwnsRumble ? gLocalProfileRumbleIntensity : 0);
  if (manualRumbleIntensity != gDisplayUi.manualRumbleIntensity) {
    gDisplayUi.manualRumbleIntensity = manualRumbleIntensity;
    gDisplayDirty = true;
  }
}

void drawRateAndReverse(const char* rateText) {
  char reverseText[12];
  snprintf(
    reverseText,
    sizeof(reverseText),
    "%lums %u%%",
    static_cast<unsigned long>(config::kReverseDurationMs),
    config::kReverseKickPercent
  );

  gDisplay.setCursor(0, 33);
  gDisplay.print("RATE");
  drawRightAlignedText(rateText, 33);
  gDisplay.setCursor(0, 43);
  gDisplay.print("REV");
  drawRightAlignedText(reverseText, 43);
}

void drawBottomStatus() {
  gDisplay.setCursor(0, 56);
  gDisplay.print("BT");

  if (!gBluetoothReady) {
    gDisplay.setCursor(16, 56);
    gDisplay.print("ERROR");
  } else {
    if (gBluetoothConnected) {
      gDisplay.fillCircle(17, 59, 2, SSD1306_WHITE);
    } else {
      gDisplay.drawCircle(17, 59, 2, SSD1306_WHITE);
    }
    gDisplay.setCursor(22, 56);
    gDisplay.print(gBluetoothConnected ? "CONNECTED" : "WAITING");
  }

  drawRightAlignedText(displayOperatingModeName(), 56);
}

void drawManualDisplay() {
  const profiles::HapticProfile& profile = currentProfile();
  char rateText[16];

  switch (profile.behavior) {
    case profiles::ProfileBehavior::Single:
      snprintf(rateText, sizeof(rateText), "SINGLE");
      break;
    case profiles::ProfileBehavior::Auto:
      if (profile.autoRepeatMs > 0) {
        snprintf(
          rateText,
          sizeof(rateText),
          "~%lu RPM",
          static_cast<unsigned long>(60000UL / profile.autoRepeatMs)
        );
      } else {
        snprintf(rateText, sizeof(rateText), "--");
      }
      break;
    case profiles::ProfileBehavior::ChargeRelease:
      snprintf(
        rateText,
        sizeof(rateText),
        "%s",
        profiles::isChargeReleaseConfigured(profile) ? "CHARGE" : "UNCAL"
      );
      break;
  }

  gDisplay.setCursor(0, 0);
  gDisplay.print(profile.name);
  drawRightAlignedText(profileBehaviorName(profile.behavior), 0);
  gDisplay.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  drawIntensityRow(
    "KICK",
    12,
    gDisplayUi.localKickLatched ? gDisplayUi.localKickIntensity : 0,
    profile.kickIntensity
  );
  drawIntensityRow(
    "RUMB",
    22,
    gDisplayUi.manualRumbleIntensity,
    gDisplayUi.manualRumbleIntensity
  );
  drawRateAndReverse(rateText);
  gDisplay.drawLine(0, 53, 127, 53, SSD1306_WHITE);
  drawBottomStatus();
}

void drawForceTubeLiveDisplay() {
  char rateText[16];
  const uint32_t rateRpm = calculateFtRateRpm();
  if (gDisplayUi.ftIntervalCount > 0 && rateRpm > 0) {
    snprintf(
      rateText,
      sizeof(rateText),
      "~%lu RPM",
      static_cast<unsigned long>(rateRpm)
    );
  } else {
    snprintf(rateText, sizeof(rateText), "--");
  }

  gDisplay.setCursor(0, 0);
  gDisplay.print("FT LIVE");
  gDisplay.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  drawIntensityRow(
    "KICK",
    12,
    gDisplayUi.ftKickLatched ? gDisplayUi.ftKickIntensity : 0,
    gDisplayUi.ftKickIntensity
  );
  drawIntensityRow(
    "RUMB",
    22,
    gDisplayUi.ftRumbleIntensity,
    gDisplayUi.ftRumbleIntensity
  );
  drawRateAndReverse(rateText);
  gDisplay.drawLine(0, 53, 127, 53, SSD1306_WHITE);
  drawBottomStatus();
}

void showAuthorSplash(uint32_t now) {
  if (!gDisplayReady) {
    return;
  }

  gDisplay.clearDisplay();
  gDisplay.setTextColor(SSD1306_WHITE);
  gDisplay.setTextSize(2);
  gDisplay.setCursor(4, 0);
  gDisplay.print("TRIGGER V3");
  gDisplay.setTextSize(1);
  gDisplay.setCursor(0, 20);
  gDisplay.print("by alfawalidou /");
  gDisplay.setCursor(0, 30);
  gDisplay.print("McWall");
  gDisplay.setCursor(0, 42);
  gDisplay.print("GitHub: alfawalidou");
  gDisplay.setCursor(0, 54);
  gDisplay.print("@McWall07 | .mcwall");
  gDisplay.display();
  gAuthorSplashUntil = now + identity::kAuthorSplashDurationMs;
  gDisplayDirty = true;
}

void updateDisplay(uint32_t now) {
  if (gAuthorSplashUntil != 0) {
    if (static_cast<int32_t>(now - gAuthorSplashUntil) < 0) {
      return;
    }
    gAuthorSplashUntil = 0;
    gDisplayDirty = true;
  }

  if (!gDisplayReady || !gDisplayDirty ||
      static_cast<uint32_t>(now - gLastDisplayRefreshAt) < config::kDisplayRefreshMs) {
    return;
  }
  gLastDisplayRefreshAt = now;
  gDisplayDirty = false;

  gDisplay.clearDisplay();
  gDisplay.setTextSize(1);
  gDisplay.setTextColor(SSD1306_WHITE);
  if (gDisplayUi.ftLiveActive) {
    drawForceTubeLiveDisplay();
  } else {
    drawManualDisplay();
  }
  gDisplay.display();
}

void setOutputsSafeBeforeBoot() {
  pinMode(config::kRumblePin, OUTPUT);
  pinMode(config::kSolenoidRpwmPin, OUTPUT);
  pinMode(config::kSolenoidLpwmPin, OUTPUT);
  pinMode(config::kBuzzerPin, OUTPUT);
  digitalWrite(config::kRumblePin, LOW);
  digitalWrite(config::kSolenoidRpwmPin, LOW);
  digitalWrite(config::kSolenoidLpwmPin, LOW);
  digitalWrite(config::kBuzzerPin, LOW);
}

}  // namespace

void setup() {
  setOutputsSafeBeforeBoot();

  Serial.begin(115200);
  Serial.printf("[BOOT] %s\n", identity::kProjectName);
  Serial.printf("[BOOT] by %s\n", identity::kAuthor);

  FastLED.addLeds<WS2812B, config::kLedDataPin, RGB>(gLeds, config::kLedCount);
  FastLED.setBrightness(config::kLedBrightness);
  gLeds[config::kConnectionLedIndex] = CRGB::Red;
  gLeds[config::kStatusLedIndex] = CRGB::Orange;
  FastLED.show();
  gBootHealth.report(
    "ws2812",
    "WS2812 GPIO16",
    HealthStatus::Configured,
    "2 outputs configured; physical LEDs are not detectable",
    false,
    "UNKNOWN"
  );

  gProfileButton.begin();
  gTriggerButton.begin();
  gBootHealth.report(
    "trigger",
    "Trigger GPIO13",
    HealthStatus::Reserved,
    "INPUT_PULLUP and boot-release guard configured; physical input not detectable",
    true,
    "UNKNOWN"
  );
  gBootHealth.report(
    "profile",
    "Profile GPIO14",
    HealthStatus::Reserved,
    "INPUT_PULLUP and boot-release guard configured; physical input not detectable",
    true,
    "UNKNOWN"
  );
  gBootHealth.report(
    "buzzer",
    "Buzzer GPIO27",
    HealthStatus::Reserved,
    "OUTPUT LOW; physical buzzer is not detectable",
    true,
    "UNKNOWN"
  );

  Wire.begin(config::kOledSdaPin, config::kOledSclPin);
  Wire.setTimeOut(config::kI2cTimeoutMs);
  Wire.setClock(config::kI2cClockHz);
  gBootHealth.report(
    "i2c",
    "I2C GPIO21/22",
    HealthStatus::Configured,
    "bus configured; physical wiring is not detectable",
    true,
    "UNKNOWN"
  );

  const bool oledAddressDetected = probeI2cAddress(config::kOledAddress);
  if (oledAddressDetected) {
    gDisplayReady = gDisplay.begin(
      SSD1306_SWITCHCAPVCC,
      config::kOledAddress,
      true,
      false
    );
    gBootHealth.report(
      "oled",
      "OLED I2C 0x3C",
      gDisplayReady ? HealthStatus::Detected : HealthStatus::Error,
      gDisplayReady ? "I2C ACK; UI initialized" : "I2C ACK; UI init failed, continuing",
      true,
      "I2C_ACK"
    );
    if (gDisplayReady) {
      showAuthorSplash(millis());
    }
  } else {
    gBootHealth.report(
      "oled",
      "OLED I2C 0x3C",
      HealthStatus::NotDetected,
      "no I2C ACK; optional display skipped",
      true,
      "NO_I2C_ACK"
    );
  }

  const bool hapticsReady = gHaptics.begin();
  if (!hapticsReady) {
    Serial.println("[ACTUATORS] PWM initialization failed; outputs remain disabled");
  }
  gHaptics.setArmed(true);

  const HealthStatus actuatorStatus = !config::kActuatorsEnabled
    ? HealthStatus::Reserved
    : (hapticsReady ? HealthStatus::Configured : HealthStatus::Error);
  const char* actuatorDetail = !config::kActuatorsEnabled
    ? "mapping retained; output is gated off by safe build"
    : (hapticsReady ? "output configured; physical hardware is not detectable"
                    : "PWM initialization failed; outputs remain off");
  gBootHealth.report(
    "rumble", "Rumble GPIO17", actuatorStatus, actuatorDetail, false, "UNKNOWN"
  );
  gBootHealth.report(
    "bts7960", "BTS7960 GPIO5/23", actuatorStatus, actuatorDetail, false, "UNKNOWN"
  );
  gBootHealth.report(
    "solenoid", "Solenoid", actuatorStatus, actuatorDetail, false, "UNKNOWN"
  );

  gBluetooth.register_callback(onBluetoothEvent);
  gBluetoothReady = gBluetooth.begin(config::kBluetoothName);
  if (!gBluetoothReady) {
    Serial.println("[BT] initialization failed");
  }
  gBootHealth.report(
    "bluetooth",
    "Bluetooth SPP",
    gBluetoothReady ? HealthStatus::Configured : HealthStatus::Error,
    gBluetoothReady ? "server initialized" : "stack unavailable",
    false,
    "UNKNOWN"
  );
  gBootHealth.report(
    "forcetube",
    "ForceTube protocol",
    HealthStatus::Configured,
    "four-byte streaming parser ready",
    false,
    "UNKNOWN"
  );

  TV3_TELEMETRYF("[BT] ready name=%s\n", config::kBluetoothName);
  TV3_TELEMETRYF("[MODE] %s\n", operatingModeName(gOperatingMode));
  traceCurrentProfile();
  TV3_TELEMETRYF(
    "[ACTUATORS] %s\n",
    config::kActuatorsEnabled ? "COMPAT / ENABLED" : "SAFE / DISABLED"
  );
  gBootHealth.start(millis());
}

void loop() {
  const uint32_t now = millis();
  updateConnectionState();
  processBluetooth(now);
  updateLocalControls(now);
  gHaptics.update(now);
  updateBuzzer(now);
  updateLeds(now);
  updateDisplayState(now);
  updateDisplay(now);
  yield();
}
