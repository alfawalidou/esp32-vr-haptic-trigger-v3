#include "BootHealth.h"

#include <cstring>

#include "TriggerV3Config.h"

namespace trigger_v3 {

BootHealthReporter::BootHealthReporter(
  CRGB* leds,
  uint8_t connectionLedIndex,
  uint8_t statusLedIndex
) :
  leds_(leds),
  connectionLedIndex_(connectionLedIndex),
  statusLedIndex_(statusLedIndex) {}

void BootHealthReporter::report(
  const char* id,
  const char* component,
  HealthStatus status,
  const char* detail,
  bool optional,
  const char* physicalPresence
) {
  Serial.printf("[HEALTH] %-24s %-12s %s\n", component, statusName(status), detail);
  if (entryCount_ < kMaximumEntries) {
    entries_[entryCount_++] = {
      id,
      component,
      status,
      detail,
      optional,
      physicalPresence,
    };
  }
}

bool BootHealthReporter::setStatus(
  const char* id,
  HealthStatus status,
  const char* detail
) {
  for (size_t index = 0; index < entryCount_; ++index) {
    if (strcmp(entries_[index].id, id) == 0) {
      entries_[index].status = status;
      entries_[index].detail = detail;
      Serial.printf(
        "[HEALTH] %-24s %-12s %s\n",
        entries_[index].component,
        statusName(status),
        detail
      );
      return true;
    }
  }
  return false;
}

const HealthEntry& BootHealthReporter::entry(size_t index) const {
  static const HealthEntry kEmptyEntry{
    "unknown", "Unknown", HealthStatus::Reserved, "not registered", true, "UNKNOWN"
  };
  return index < entryCount_ ? entries_[index] : kEmptyEntry;
}

const HealthEntry* BootHealthReporter::find(const char* id) const {
  for (size_t index = 0; index < entryCount_; ++index) {
    if (strcmp(entries_[index].id, id) == 0) {
      return &entries_[index];
    }
  }
  return nullptr;
}

bool BootHealthReporter::systemReady() const {
  for (size_t index = 0; index < entryCount_; ++index) {
    if (!entries_[index].optional && entries_[index].status == HealthStatus::Error) {
      return false;
    }
  }
  return true;
}

void BootHealthReporter::start(uint32_t now) {
  currentVisualStep_ = 0;
  visualStepStartedAt_ = now;
  visualActive_ = entryCount_ > 0;
  currentVisualShown_ = false;
}

bool BootHealthReporter::update(uint32_t now, bool bluetoothConnected) {
  if (!visualActive_) {
    return false;
  }

  if (!currentVisualShown_) {
    showCurrent(bluetoothConnected);
    visualStepStartedAt_ = now;
    currentVisualShown_ = true;
    return true;
  }

  if (static_cast<uint32_t>(now - visualStepStartedAt_) < config::kBootHealthLedStepMs) {
    return true;
  }

  ++currentVisualStep_;
  if (currentVisualStep_ >= entryCount_) {
    visualActive_ = false;
    return false;
  }

  showCurrent(bluetoothConnected);
  visualStepStartedAt_ = now;
  return true;
}

const char* BootHealthReporter::statusName(HealthStatus status) {
  switch (status) {
    case HealthStatus::Detected:
      return "DETECTED";
    case HealthStatus::Configured:
      return "CONFIGURED";
    case HealthStatus::Reserved:
      return "RESERVED";
    case HealthStatus::NotDetected:
      return "NOT DETECTED";
    case HealthStatus::Error:
      return "ERROR";
  }
  return "RESERVED";
}

const char* BootHealthReporter::statusToken(HealthStatus status) {
  return status == HealthStatus::NotDetected ? "NOT_DETECTED" : statusName(status);
}

CRGB BootHealthReporter::statusColor(HealthStatus status) {
  switch (status) {
    case HealthStatus::Detected:
      return CRGB::Green;
    case HealthStatus::Configured:
      return CRGB::Blue;
    case HealthStatus::Reserved:
      return CRGB::Orange;
    case HealthStatus::NotDetected:
      return CRGB::Purple;
    case HealthStatus::Error:
      return CRGB::Red;
  }
  return CRGB::Orange;
}

void BootHealthReporter::showCurrent(bool bluetoothConnected) {
  leds_[connectionLedIndex_] = bluetoothConnected ? CRGB::Blue : CRGB::Red;
  leds_[statusLedIndex_] = statusColor(entries_[currentVisualStep_].status);
  FastLED.show();
}

}  // namespace trigger_v3
