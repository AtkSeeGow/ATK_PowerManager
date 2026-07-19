#pragma once

#include <Arduino.h>
#include "MeterDiagnostics.h"

struct MeterReading {
  float voltageLN;
  float currentA;
  float currentB;
  float currentC;
  float currentAverage;
  float activePowerTotalKw;
  float powerFactorTotal;
  bool powerFactorLeading;
  float frequencyHz;
  uint64_t activeEnergyDeliveredWh;
  float temperatureC;
  float humidityPercent;
  bool am2320Valid;
  uint8_t am2320Status;
  uint8_t am2320WireStatus;
  Am2320Diagnostics am2320Diagnostics;
  bool modbusValid;
  bool valid;
  uint8_t modbusStatus;
  uint64_t powerFactorInvalidCount;
};
