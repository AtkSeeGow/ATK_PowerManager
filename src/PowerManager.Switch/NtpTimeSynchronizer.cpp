#include "NtpTimeSynchronizer.h"

#include "SerialLogger.h"
#include "SwitchConfiguration.h"

#include <time.h>

namespace {
constexpr time_t MIN_VALID_EPOCH = 1704067200;  // 2024-01-01T00:00:00Z
}

void NtpTimeSynchronizer::begin() {
  configTzTime(
    SwitchConfiguration::TIME_ZONE,
    SwitchConfiguration::NTP_SERVER_PRIMARY,
    SwitchConfiguration::NTP_SERVER_SECONDARY);
  SerialLogger::infof(
    "NTP",
    "Synchronization requested; timezone=%s, servers=%s,%s",
    SwitchConfiguration::TIME_ZONE,
    SwitchConfiguration::NTP_SERVER_PRIMARY,
    SwitchConfiguration::NTP_SERVER_SECONDARY);
}

void NtpTimeSynchronizer::update(bool networkAvailable) {
  if (timeSynchronized || !networkAvailable)
    return;

  time_t currentEpochSeconds = time(nullptr);
  if (currentEpochSeconds < MIN_VALID_EPOCH)
    return;

  timeSynchronized = true;
  SerialLogger::infof(
    "NTP",
    "Time synchronized; epoch=%lld",
    static_cast<long long>(currentEpochSeconds));
}
