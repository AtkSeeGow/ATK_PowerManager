#pragma once

#include <Arduino.h>
#include "Am2320Sensor.h"
#include "MeterApiServer.h"
#include "Pm5350Modbus.h"

class MeterService {
public:
  MeterService(
    const char* ssid,
    const char* password,
    const char* hostname,
    HardwareSerial& modbusSerial,
    uint8_t slaveId,
    int rxPin,
    int txPin,
    int dePin,
    int rePin,
    int am2320SdaPin,
    int am2320SclPin);

  void begin(unsigned long setupStartedAtMs);
  void loop();

private:
  MeterApiServer meterApiServer;
  Pm5350Modbus pm5350;
  Am2320Sensor am2320;
  TaskHandle_t meterReadTaskHandle;

  static void meterReadTaskEntry(void* context);
  void runMeterReadTask();
  void readAndPublishMeter();
  void logMeterReading(const MeterReading& reading);
  static String formatFloat(float value, uint8_t decimals);
  static String formatEnergyKwh(uint64_t energyWh);
};
