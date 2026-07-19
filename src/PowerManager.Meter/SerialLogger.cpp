#include "SerialLogger.h"
#include "MeterConfig.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

SemaphoreHandle_t SerialLogger::logMutex = nullptr;

void SerialLogger::begin(unsigned long baudRate)
{
  Serial.begin(baudRate);
  logMutex = xSemaphoreCreateMutex();
  Serial.println();
  info("LOGGER", "Serial logger initialized.");
}

void SerialLogger::info(const char* category, const char* message)
{
  if (logMutex != nullptr) {
    xSemaphoreTake(logMutex, portMAX_DELAY);
  }
  writePrefix(category);
  Serial.println(message);
  if (logMutex != nullptr) {
    xSemaphoreGive(logMutex);
  }
}

void SerialLogger::info(const char* category, const String& message)
{
  info(category, message.c_str());
}

void SerialLogger::infof(const char* category, const char* format, ...)
{
  char message[256];
  va_list arguments;
  va_start(arguments, format);
  vsnprintf(message, sizeof(message), format, arguments);
  va_end(arguments);
  info(category, message);
}

void SerialLogger::writePrefix(const char* category)
{
  time_t currentEpochSeconds = time(nullptr);
  if (currentEpochSeconds >= MeterConfig::Ntp::MIN_VALID_EPOCH) {
    struct tm timeInfo;
    gmtime_r(&currentEpochSeconds, &timeInfo);
    char utcTime[25];
    strftime(utcTime, sizeof(utcTime), "%Y-%m-%dT%H:%M:%SZ", &timeInfo);
    Serial.print("[utc=");
    Serial.print(utcTime);
    Serial.print(']');
  }

  Serial.print("[uptime_ms=");
  Serial.print(millis());
  Serial.print("][");
  Serial.print(category);
  Serial.print("] ");
}
