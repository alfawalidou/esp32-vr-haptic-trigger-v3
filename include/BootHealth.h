#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace trigger_v3 {

enum class HealthStatus : uint8_t {
  Detected,
  Configured,
  Reserved,
  NotDetected,
  Error,
};

struct HealthEntry {
  const char* id;
  const char* component;
  HealthStatus status;
  const char* detail;
  bool optional;
  const char* physicalPresence;
};

class BootHealthReporter {
 public:
  BootHealthReporter(CRGB* leds, uint8_t connectionLedIndex, uint8_t statusLedIndex);

  void report(
    const char* id,
    const char* component,
    HealthStatus status,
    const char* detail,
    bool optional = false,
    const char* physicalPresence = "UNKNOWN"
  );
  bool setStatus(const char* id, HealthStatus status, const char* detail);
  size_t count() const { return entryCount_; }
  const HealthEntry& entry(size_t index) const;
  const HealthEntry* find(const char* id) const;
  bool systemReady() const;

  void start(uint32_t now);
  bool update(uint32_t now, bool bluetoothConnected);

  static const char* statusName(HealthStatus status);
  static const char* statusToken(HealthStatus status);

 private:
  static constexpr size_t kMaximumEntries = 16;

  static CRGB statusColor(HealthStatus status);
  void showCurrent(bool bluetoothConnected);

  CRGB* leds_;
  uint8_t connectionLedIndex_;
  uint8_t statusLedIndex_;
  HealthEntry entries_[kMaximumEntries]{};
  size_t entryCount_ = 0;
  size_t currentVisualStep_ = 0;
  uint32_t visualStepStartedAt_ = 0;
  bool visualActive_ = false;
  bool currentVisualShown_ = false;
};

}  // namespace trigger_v3
