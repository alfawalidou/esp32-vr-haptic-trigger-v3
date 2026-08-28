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

namespace trigger_v3 {
namespace profiles {

enum class ProfileBehavior : uint8_t {
  Single,
  Auto,
  ChargeRelease,
};

enum class ChargeReleaseOrder : uint8_t {
  Uncalibrated,
  StopRumbleThenKick,
  KickThenStopRumble,
};

struct HapticProfile {
  const char* name;
  ProfileBehavior behavior;

  uint8_t kickIntensity;
  uint8_t manualRumbleIntensity;

  uint16_t autoRepeatMs;

  uint8_t chargeRumbleMin;
  uint8_t chargeRumbleMax;
  uint8_t chargeStepCount;
  uint16_t chargeDurationMs;
  ChargeReleaseOrder chargeReleaseOrder;

  uint8_t ledRed;
  uint8_t ledGreen;
  uint8_t ledBlue;
};

constexpr HapticProfile makeSingleProfile(
  const char* name,
  uint8_t kickIntensity,
  uint8_t manualRumbleIntensity,
  uint8_t ledRed,
  uint8_t ledGreen,
  uint8_t ledBlue
) {
  return {
    name,
    ProfileBehavior::Single,
    kickIntensity,
    manualRumbleIntensity,
    0,
    0,
    0,
    0,
    0,
    ChargeReleaseOrder::Uncalibrated,
    ledRed,
    ledGreen,
    ledBlue,
  };
}

constexpr HapticProfile makeAutoProfile(
  const char* name,
  uint8_t kickIntensity,
  uint8_t manualRumbleIntensity,
  uint16_t autoRepeatMs,
  uint8_t ledRed,
  uint8_t ledGreen,
  uint8_t ledBlue
) {
  return {
    name,
    ProfileBehavior::Auto,
    kickIntensity,
    manualRumbleIntensity,
    autoRepeatMs,
    0,
    0,
    0,
    0,
    ChargeReleaseOrder::Uncalibrated,
    ledRed,
    ledGreen,
    ledBlue,
  };
}

constexpr HapticProfile makeChargeReleaseProfile(
  const char* name,
  uint8_t kickIntensity,
  uint8_t chargeRumbleMin,
  uint8_t chargeRumbleMax,
  uint8_t chargeStepCount,
  uint16_t chargeDurationMs,
  ChargeReleaseOrder chargeReleaseOrder,
  uint8_t ledRed,
  uint8_t ledGreen,
  uint8_t ledBlue
) {
  return {
    name,
    ProfileBehavior::ChargeRelease,
    kickIntensity,
    0,
    0,
    chargeRumbleMin,
    chargeRumbleMax,
    chargeStepCount,
    chargeDurationMs,
    chargeReleaseOrder,
    ledRed,
    ledGreen,
    ledBlue,
  };
}

// TEMPORARY INHERITED TUNING VALUE.
// Do not interpret 150 ms as the measured real weapon cadence.
static constexpr uint16_t kInheritedAutoRepeatMs = 150;

static constexpr HapticProfile kHapticProfiles[] = {
  makeSingleProfile("PISTOL", 120, 0, 0, 0, 255),
  makeSingleProfile("SNIPER", 255, 128, 0, 128, 0),
  makeAutoProfile("M16", 240, 125, kInheritedAutoRepeatMs, 255, 0, 0),
  makeAutoProfile("P90", 220, 159, kInheritedAutoRepeatMs, 255, 255, 0),
  makeAutoProfile("PKM", 129, 255, kInheritedAutoRepeatMs, 255, 0, 255),
  makeChargeReleaseProfile(
    "LASER",
    255,
    0,
    255,
    20,
    2600,
    ChargeReleaseOrder::KickThenStopRumble,
    255,
    255,
    255
  ),
};
static constexpr size_t kHapticProfileCount =
  sizeof(kHapticProfiles) / sizeof(kHapticProfiles[0]);
static constexpr size_t kDefaultProfileIndex = 0;

constexpr bool isChargeReleaseConfigured(const HapticProfile& profile) {
  return profile.behavior == ProfileBehavior::ChargeRelease &&
    profile.chargeDurationMs > 0 &&
    profile.chargeReleaseOrder != ChargeReleaseOrder::Uncalibrated;
}

constexpr bool isProfileStructurallyValid(const HapticProfile& profile) {
  switch (profile.behavior) {
    case ProfileBehavior::Single:
      return profile.autoRepeatMs == 0;
    case ProfileBehavior::Auto:
      return profile.autoRepeatMs > 0;
    case ProfileBehavior::ChargeRelease:
      return profile.chargeStepCount > 0 &&
        profile.chargeRumbleMax >= profile.chargeRumbleMin;
  }
  return false;
}

constexpr bool areHapticProfilesStructurallyValid() {
  for (size_t index = 0; index < kHapticProfileCount; ++index) {
    if (!isProfileStructurallyValid(kHapticProfiles[index])) {
      return false;
    }
  }
  return true;
}

static_assert(kHapticProfileCount > 0, "At least one haptic profile is required");
static_assert(kDefaultProfileIndex < kHapticProfileCount, "Default profile is out of range");
static_assert(
  areHapticProfilesStructurallyValid(),
  "Haptic profile behavior configuration is invalid"
);

}  // namespace profiles
}  // namespace trigger_v3
