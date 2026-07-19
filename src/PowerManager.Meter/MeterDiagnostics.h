#pragma once

#include <Arduino.h>

enum class Am2320Status : uint8_t {
  Success = 0,
  NotInitialized = 1,
  CommandFailed = 2,
  ResponseTooShort = 3,
  InvalidFrame = 4,
  CrcMismatch = 5
};

struct Am2320Diagnostics {
  static constexpr uint8_t STATUS_COUNT = 6;

  uint64_t failureCounts[STATUS_COUNT];
  uint64_t retryRecoveryCount;

  uint64_t failureCount(Am2320Status status) const {
    uint8_t index = static_cast<uint8_t>(status);
    return index < STATUS_COUNT ? failureCounts[index] : 0;
  }
};
