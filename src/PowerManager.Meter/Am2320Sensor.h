#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "MeterDiagnostics.h"

struct Am2320Reading {
  float temperatureC;
  float humidityPercent;
  bool valid;
  Am2320Status status;
  uint8_t wireStatus;
  Am2320Diagnostics diagnostics;
};

class Am2320Sensor {
public:
  Am2320Sensor(int sdaPin, int sclPin, TwoWire& wire = Wire);

  bool begin();
  Am2320Reading read();

private:
  static constexpr uint8_t I2C_ADDRESS = 0x5C;
  static constexpr uint8_t READ_FUNCTION = 0x03;
  static constexpr uint8_t REGISTER_START = 0x00;
  static constexpr uint8_t REGISTER_COUNT = 0x04;
  static constexpr uint8_t RESPONSE_SIZE = 8;

  TwoWire& wire;
  int sdaPin;
  int sclPin;
  bool initialized;
  Am2320Diagnostics diagnostics;

  void runStartupDiagnostics();
  Am2320Reading readOnce();
  void recordFailure(Am2320Status status);
  void attachDiagnostics(Am2320Reading& reading) const;
  uint8_t probeAddress(uint8_t address);
  static uint16_t calculateCrc(const uint8_t* data, uint8_t length);
  static const char* statusToString(Am2320Status status);
};
