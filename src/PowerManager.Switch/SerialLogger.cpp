#include "SerialLogger.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

namespace {
constexpr time_t MIN_VALID_EPOCH = 1704067200;  // 2024-01-01T00:00:00Z
}

void SerialLogger::begin(unsigned long baudRate) {
  Serial.begin(baudRate);
  Serial.println();
  info("LOGGER", "Serial logger initialized.");
}

void SerialLogger::info(const char* category, const char* message) {
  writePrefix(category);
  Serial.println(message);
}

void SerialLogger::info(const char* category, const String& message) {
  info(category, message.c_str());
}

void SerialLogger::infof(const char* category, const char* format, ...) {
  char message[256];
  va_list arguments;
  va_start(arguments, format);
  vsnprintf(message, sizeof(message), format, arguments);
  va_end(arguments);
  info(category, message);
}

void SerialLogger::writePrefix(const char* category) {
  time_t currentEpochSeconds = time(nullptr);
  if (currentEpochSeconds >= MIN_VALID_EPOCH) {
    struct tm timeInfo;
    localtime_r(&currentEpochSeconds, &timeInfo);
    char localTime[32];
    strftime(localTime, sizeof(localTime), "%Y-%m-%dT%H:%M:%S%z", &timeInfo);
    Serial.print("[time=");
    Serial.print(localTime);
    Serial.print(']');
  }

  Serial.print("[uptime_ms=");
  Serial.print(millis());
  Serial.print("][");
  Serial.print(category);
  Serial.print("] ");
}
