#pragma once

#include <Arduino.h>

namespace trigger_v3 {

enum class HapticChannel : uint8_t {
  Kick = 0x00,
  Rumble = 0x01,
};

struct HapticCommand {
  HapticChannel channel;
  uint8_t intensity;
};

class ForceTubeProtocolParser {
 public:
  bool feed(uint8_t value, HapticCommand& command) {
    switch (state_) {
      case State::WaitStart:
        if (value == kCommandStart) {
          state_ = State::WaitMagic;
        }
        return false;

      case State::WaitMagic:
        if (value == kMagic) {
          state_ = State::WaitChannel;
        } else if (value != kCommandStart) {
          reset();
        }
        return false;

      case State::WaitChannel:
        if (value == static_cast<uint8_t>(HapticChannel::Kick) ||
            value == static_cast<uint8_t>(HapticChannel::Rumble)) {
          pendingChannel_ = static_cast<HapticChannel>(value);
          state_ = State::WaitIntensity;
        } else {
          restartFrom(value);
        }
        return false;

      case State::WaitIntensity:
        command.channel = pendingChannel_;
        command.intensity = value;
        reset();
        return true;
    }

    reset();
    return false;
  }

  void reset() { state_ = State::WaitStart; }

 private:
  enum class State : uint8_t {
    WaitStart,
    WaitMagic,
    WaitChannel,
    WaitIntensity,
  };

  static constexpr uint8_t kCommandStart = 0x2A;
  static constexpr uint8_t kMagic = 0xB0;

  void restartFrom(uint8_t value) {
    state_ = value == kCommandStart ? State::WaitMagic : State::WaitStart;
  }

  State state_ = State::WaitStart;
  HapticChannel pendingChannel_ = HapticChannel::Kick;
};

}  // namespace trigger_v3
