#pragma once

#include <Arduino.h>

class SerialLogger {
public:
  static void begin(unsigned long baudRate);
  static void info(const char* category, const char* message);
  static void info(const char* category, const String& message);
  static void infof(const char* category, const char* format, ...);

private:
  static SemaphoreHandle_t logMutex;
  static void writePrefix(const char* category);
};
