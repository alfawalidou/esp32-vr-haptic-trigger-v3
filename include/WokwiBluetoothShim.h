/*
 * ESP32 VR Haptic Trigger V3
 * Original project by alfawalidou / McWall
 * GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
 * Discord: .mcwall | Telegram: @McWall07
 *
 * Copyright 2026 alfawalidou / McWall
 * SPDX-License-Identifier: Apache-2.0
 *
 * Wokwi-only BluetoothSerial compatibility shim.
 * This file is force-included only by the trigger-v3-wokwi PlatformIO env.
 * Production SAFE/COMPAT builds never compile against this shim.
 */

#pragma once

#ifndef TRIGGER_V3_WOKWI
#error "WokwiBluetoothShim.h must only be used by the trigger-v3-wokwi environment"
#endif

#include <Arduino.h>
#include <cstdio>
#include <cstring>

// Prevent the real BluetoothSerial header from being compiled in the Wokwi
// environment. Wokwi currently does not simulate ESP32 Bluetooth hardware.
#ifndef _BLUETOOTH_SERIAL_H_
#define _BLUETOOTH_SERIAL_H_
#endif

enum esp_spp_cb_event_t : uint8_t {
  ESP_SPP_SRV_OPEN_EVT = 1,
  ESP_SPP_CLOSE_EVT = 2,
};

struct esp_spp_cb_param_t {};

using WokwiSppCallback = void (*)(esp_spp_cb_event_t, esp_spp_cb_param_t*);

class BluetoothSerial {
 public:
  int register_callback(WokwiSppCallback callback) {
    callback_ = callback;
    return 0;
  }

  bool begin(const char* localName) {
    localName_ = localName;
    connected_ = true;
    if (callback_ != nullptr) {
      callback_(ESP_SPP_SRV_OPEN_EVT, nullptr);
    }

    Serial.println();
    Serial.println("[WOKWI] Bluetooth hardware is not simulated.");
    Serial.println("[WOKWI] Serial terminal is acting as a ForceTube transport shim.");
    Serial.printf("[WOKWI] Simulated device: %s\n", localName_ != nullptr ? localName_ : "Trigger V3");
    printHelp();
    return true;
  }

  int available() {
    pumpSerial();
    return static_cast<int>(queueCount_);
  }

  int read() {
    pumpSerial();
    if (queueCount_ == 0) {
      return -1;
    }

    const uint8_t value = queue_[queueRead_];
    queueRead_ = (queueRead_ + 1) % kQueueSize;
    --queueCount_;
    return value;
  }

 private:
  static constexpr size_t kQueueSize = 64;
  static constexpr size_t kLineSize = 48;

  void pumpSerial() {
    while (Serial.available() > 0) {
      const int raw = Serial.read();
      if (raw < 0) {
        return;
      }

      const char ch = static_cast<char>(raw);
      if (ch == '\r') {
        continue;
      }
      if (ch == '\n') {
        line_[lineLength_] = '\0';
        if (lineLength_ > 0) {
          handleLine(line_);
        }
        lineLength_ = 0;
        continue;
      }

      if (lineLength_ + 1 < kLineSize) {
        line_[lineLength_++] = ch;
      }
    }
  }

  void handleLine(const char* line) {
    unsigned int intensity = 0;

    if (std::sscanf(line, "KICK %u", &intensity) == 1 && intensity <= 255) {
      enqueueForceTubePacket(0x00, static_cast<uint8_t>(intensity));
      return;
    }

    if (std::sscanf(line, "RUMBLE %u", &intensity) == 1 && intensity <= 255) {
      enqueueForceTubePacket(0x01, static_cast<uint8_t>(intensity));
      return;
    }

    if (std::strcmp(line, "BT OFF") == 0) {
      if (connected_) {
        connected_ = false;
        clearQueue();
        if (callback_ != nullptr) {
          callback_(ESP_SPP_CLOSE_EVT, nullptr);
        }
        Serial.println("[WOKWI] Simulated Bluetooth disconnected.");
      }
      return;
    }

    if (std::strcmp(line, "BT ON") == 0) {
      if (!connected_) {
        connected_ = true;
        if (callback_ != nullptr) {
          callback_(ESP_SPP_SRV_OPEN_EVT, nullptr);
        }
        Serial.println("[WOKWI] Simulated Bluetooth connected.");
      }
      return;
    }

    if (std::strcmp(line, "HELP") == 0) {
      printHelp();
      return;
    }

    Serial.printf("[WOKWI] Unknown command: %s\n", line);
    printHelp();
  }

  void enqueueForceTubePacket(uint8_t channel, uint8_t intensity) {
    if (!connected_) {
      Serial.println("[WOKWI] Command ignored while simulated Bluetooth is OFF.");
      return;
    }

    if (kQueueSize - queueCount_ < 4) {
      Serial.println("[WOKWI] Input queue full; command dropped.");
      return;
    }

    enqueueByte(0x2A);
    enqueueByte(0xB0);
    enqueueByte(channel);
    enqueueByte(intensity);

    Serial.printf(
      "[WOKWI] Injected ForceTube %s %u\n",
      channel == 0x00 ? "KICK" : "RUMBLE",
      intensity
    );
  }

  void enqueueByte(uint8_t value) {
    queue_[queueWrite_] = value;
    queueWrite_ = (queueWrite_ + 1) % kQueueSize;
    ++queueCount_;
  }

  void clearQueue() {
    queueRead_ = 0;
    queueWrite_ = 0;
    queueCount_ = 0;
  }

  static void printHelp() {
    Serial.println("[WOKWI] Terminal commands:");
    Serial.println("         KICK 255");
    Serial.println("         RUMBLE 128");
    Serial.println("         RUMBLE 0");
    Serial.println("         BT OFF");
    Serial.println("         BT ON");
    Serial.println("         HELP");
  }

  WokwiSppCallback callback_ = nullptr;
  const char* localName_ = nullptr;
  bool connected_ = false;

  uint8_t queue_[kQueueSize]{};
  size_t queueRead_ = 0;
  size_t queueWrite_ = 0;
  size_t queueCount_ = 0;

  char line_[kLineSize]{};
  size_t lineLength_ = 0;
};
