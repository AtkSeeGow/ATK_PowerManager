#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>
#include "MeterReading.h"

class Pm5350Modbus {
public:
  Pm5350Modbus(HardwareSerial& serial, uint8_t slaveId, int rxPin, int txPin, int dePin, int rePin);

  void begin();
  MeterReading readMeter();

private:
  static constexpr uint16_t CURRENT_A_REGISTER = 0x0BB7;
  static constexpr uint16_t CURRENT_B_REGISTER = 0x0BB9;
  static constexpr uint16_t CURRENT_C_REGISTER = 0x0BBB;
  static constexpr uint16_t CURRENT_AVERAGE_REGISTER = 0x0BC1;
  // PM5350 documented register 3028 (zero-based Modbus address 0x0BD3): V1-N.
  static constexpr uint16_t VOLTAGE_LN_REGISTER = 0x0BD3;
  static constexpr uint16_t ACTIVE_POWER_TOTAL_REGISTER = 0x0BF3;
  static constexpr uint16_t POWER_FACTOR_TOTAL_REGISTER = 0x0C0B;
  static constexpr uint16_t FREQUENCY_REGISTER = 0x0C25;
  static constexpr uint16_t ACTIVE_ENERGY_DELIVERED_REGISTER = 0x0C83;
  static constexpr uint8_t FLOAT_REGISTER_COUNT = 2;
  static constexpr uint8_t INT64_REGISTER_COUNT = 4;
  static Pm5350Modbus* activeInstance;

  HardwareSerial& serial;
  uint8_t slaveId;
  int rxPin;
  int txPin;
  int dePin;
  int rePin;
  ModbusMaster node;
  uint64_t powerFactorInvalidCount;

  static void preTransmission();
  static void postTransmission();
  static float registersToFloat(uint16_t registerHigh, uint16_t registerLow);
  static uint64_t registersToUint64(uint16_t register0, uint16_t register1, uint16_t register2, uint16_t register3);
  bool readFloat(uint16_t startRegister, float& value, uint8_t& modbusStatus);
  bool retryNonFiniteFloat(uint16_t startRegister, float& value, uint8_t& modbusStatus, const char* fieldName);
  bool readUint64(uint16_t startRegister, uint64_t& value, uint8_t& modbusStatus);
  uint8_t readHoldingRegisters(uint16_t startRegister, uint8_t registerCount);
  void drainReceiveBuffer();
  static bool isRetryableStatus(uint8_t status);
  static float decodePowerFactor(float rawPowerFactor, bool& leading);
  void setTransmitEnabled(bool enabled);
};
