#pragma once

#include <Arduino.h>

namespace trigger_v3 {

class HapticController {
 public:
  bool begin();
  void update(uint32_t now);
  void setArmed(bool armed);
  void stopAll();

  bool requestKick(uint8_t intensity, uint32_t now);
  void setRumble(uint8_t intensity, uint32_t now);

  bool isArmed() const { return armed_; }
  bool isKickActive() const { return kickState_ != KickState::Idle; }
  uint8_t kickIntensity() const { return kickIntensity_; }
  uint8_t rumbleIntensity() const { return rumbleIntensity_; }
  bool rumbleTimedOut() const { return rumbleTimedOut_; }

 private:
  enum class KickState : uint8_t {
    Idle,
    Forward,
    Reverse,
  };

  void startForward(uint8_t pwm);
  void startReverse(uint8_t pwm);
  void stopSolenoid();
  void writeRumble(uint8_t intensity);

  bool armed_ = false;
  bool hardwareReady_ = false;
  bool rumblePwmReady_ = false;
  bool rumbleTimedOut_ = false;
  KickState kickState_ = KickState::Idle;
  uint8_t kickIntensity_ = 0;
  uint8_t kickPwm_ = 0;
  uint8_t rumbleIntensity_ = 0;
  uint8_t rumbleAppliedIntensity_ = 0xFF;
  uint32_t kickStateStartedAt_ = 0;
  uint32_t lastRumbleCommandAt_ = 0;
  uint32_t lastRumbleHardwareApplyAt_ = 0;
};

}  // namespace trigger_v3
