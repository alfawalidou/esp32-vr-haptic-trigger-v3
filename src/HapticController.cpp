/*
 * ESP32 VR Haptic Trigger V3
 * Original project by alfawalidou / McWall
 * GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
 * Discord: .mcwall | Telegram: @McWall07
 *
 * Copyright 2026 alfawalidou / McWall
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include "HapticController.h"

#include "TriggerV3Config.h"

namespace trigger_v3 {

bool HapticController::begin() {
  pinMode(config::kRumblePin, OUTPUT);
  pinMode(config::kSolenoidRpwmPin, OUTPUT);
  pinMode(config::kSolenoidLpwmPin, OUTPUT);

  digitalWrite(config::kRumblePin, LOW);
  digitalWrite(config::kSolenoidRpwmPin, LOW);
  digitalWrite(config::kSolenoidLpwmPin, LOW);

  // Same Arduino LEDC path as the historical firmware that was physically
  // stable with GPIO17 + IRLZ44N + Xbox rumble motors.
  rumblePwmReady_ = ledcAttach(
    config::kRumblePin,
    config::kRumbleFrequencyHz,
    config::kPwmResolutionBits
  );
  hardwareReady_ = rumblePwmReady_;

  writeRumble(0);
  stopSolenoid();
  return hardwareReady_;
}

void HapticController::setArmed(bool armed) {
  armed_ = armed && config::kActuatorsEnabled && hardwareReady_;
  if (!armed_) {
    stopAll();
  }
}

void HapticController::stopAll() {
  writeRumble(0);
  rumbleIntensity_ = 0;
  rumbleTimedOut_ = false;
  stopSolenoid();
  kickState_ = KickState::Idle;
  kickIntensity_ = 0;
}

bool HapticController::requestKick(uint8_t intensity, uint32_t now) {
  if (intensity == 0) {
    // KICK is an impulse. Do not let a trailing ForceTube KICK=0
    // abort an already running Forward/Reverse sequence.
    if (kickState_ == KickState::Idle) {
      stopSolenoid();
      kickIntensity_ = 0;
    }
    return false;
  }

  if (!armed_ || kickState_ != KickState::Idle) {
    return false;
  }

  kickIntensity_ = intensity;
  kickPwm_ = static_cast<uint8_t>(map(
    intensity,
    0,
    255,
    config::kSolenoidMinPwm,
    config::kSolenoidMaxPwm
  ));
  kickState_ = KickState::Forward;
  kickStateStartedAt_ = now;

  startForward(kickPwm_);

  return true;
}

void HapticController::setRumble(uint8_t intensity, uint32_t now) {
  if (!armed_) {
    rumbleIntensity_ = 0;
    rumbleTimedOut_ = false;
    return;
  }

  // Bluetooth remains real-time: only remember the newest requested duty.
  // update() applies the physical PWM at a bounded rate.
  rumbleIntensity_ = intensity;
  rumbleTimedOut_ = false;
  if (intensity > 0) {
    lastRumbleCommandAt_ = now;
  }
}

void HapticController::update(uint32_t now) {
  if (rumbleIntensity_ > 0 &&
      static_cast<uint32_t>(now - lastRumbleCommandAt_) >= config::kRumbleTimeoutMs) {
    // Safety stop: force hardware OFF immediately.
    rumbleIntensity_ = 0;
    rumbleTimedOut_ = true;
    writeRumble(0);
    lastRumbleHardwareApplyAt_ = now;
  } else if (
    rumbleIntensity_ != rumbleAppliedIntensity_ &&
    static_cast<uint32_t>(now - lastRumbleHardwareApplyAt_) >=
      config::kRumbleHardwareApplyIntervalMs
  ) {
    // Coalesce all RUMBLE packets received since the previous hardware update.
    // 10 ms caps GPIO17/LEDC writes at 100 Hz while Bluetooth is still drained
    // immediately and never deliberately backlogged.
    writeRumble(rumbleIntensity_);
    lastRumbleHardwareApplyAt_ = now;
  }

  if (kickState_ == KickState::Forward &&
      static_cast<uint32_t>(now - kickStateStartedAt_) >= config::kKickDurationMs) {
    stopSolenoid();
    if (config::kReverseDurationMs > 0) {
      const uint8_t reversePwm = static_cast<uint8_t>(
        static_cast<uint16_t>(kickPwm_) * config::kReverseKickPercent / 100
      );
      startReverse(reversePwm);
      kickState_ = KickState::Reverse;
      kickStateStartedAt_ = now;
    } else {
      kickState_ = KickState::Idle;
      kickIntensity_ = 0;
    }
  } else if (kickState_ == KickState::Reverse &&
             static_cast<uint32_t>(now - kickStateStartedAt_) >= config::kReverseDurationMs) {
    stopSolenoid();
    kickState_ = KickState::Idle;
    kickIntensity_ = 0;
  }
}

void HapticController::startForward(uint8_t pwm) {
  if (!armed_) {
    return;
  }
  analogWrite(config::kSolenoidRpwmPin, 0);
  delayMicroseconds(100);
  analogWrite(config::kSolenoidLpwmPin, pwm);
}

void HapticController::startReverse(uint8_t pwm) {
  if (!armed_) {
    return;
  }
  analogWrite(config::kSolenoidLpwmPin, 0);
  delayMicroseconds(100);
  analogWrite(config::kSolenoidRpwmPin, pwm);
}

void HapticController::stopSolenoid() {
  analogWrite(config::kSolenoidLpwmPin, 0);
  analogWrite(config::kSolenoidRpwmPin, 0);
}

void HapticController::writeRumble(uint8_t intensity) {
  const uint8_t duty = armed_ ? intensity : 0;

  if (!rumblePwmReady_) {
    digitalWrite(config::kRumblePin, LOW);
    rumbleAppliedIntensity_ = 0;
    return;
  }

  ledcWrite(config::kRumblePin, duty);
  rumbleAppliedIntensity_ = duty;
}

}  // namespace trigger_v3
