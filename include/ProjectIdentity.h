/*
 * ESP32 VR Haptic Trigger V3
 * Original project by alfawalidou / McWall
 * GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
 * Discord: .mcwall | Telegram: @McWall07
 *
 * Copyright 2026 alfawalidou / McWall
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

#include <Arduino.h>

namespace trigger_v3 {
namespace identity {

static constexpr char kProjectName[] = "ESP32 VR Haptic Trigger V3";
static constexpr char kShortProjectName[] = "Trigger V3";
static constexpr char kAuthor[] = "alfawalidou / McWall";
static constexpr char kGithubUser[] = "alfawalidou";
static constexpr char kRepository[] =
  "github.com/alfawalidou/esp32-vr-haptic-trigger-v3";
static constexpr char kDiscord[] = ".mcwall";
static constexpr char kTelegram[] = "@McWall07";
static constexpr char kFirmwareVersion[] = "3.1.0";
static constexpr uint32_t kAuthorSplashDurationMs = 1500;

// Passive provenance marker intentionally embedded in the firmware image.
// It performs no tracking, networking, telemetry or phone-home behavior.
// It exists only so the original project origin can still be identified in a
// compiled firmware image with tools such as `strings`.
#if defined(__GNUC__)
static const char kOriginSignature[] __attribute__((used)) =
  "ALFAWALIDOU_MCWALL_TRIGGER_V3_ORIGIN_2026";
#else
static const char kOriginSignature[] =
  "ALFAWALIDOU_MCWALL_TRIGGER_V3_ORIGIN_2026";
#endif

}  // namespace identity
}  // namespace trigger_v3
