#include "Pm5350Modbus.h"
#include "MeterConfig.h"
#include "SerialLogger.h"

#include <math.h>
#include <string.h>

Pm5350Modbus* Pm5350Modbus::activeInstance = nullptr;

Pm5350Modbus::Pm5350Modbus(
  HardwareSerial& serial,
  uint8_t slaveId,
  int rxPin,
  int txPin,
  int dePin,
  int rePin)
  : serial(serial),
    slaveId(slaveId),
    rxPin(rxPin),
    txPin(txPin),
    dePin(dePin),
    rePin(rePin),
    powerFactorInvalidCount(0)
{
}

void Pm5350Modbus::begin()
{
  SerialLogger::infof(
    "MODBUS",
    "Initializing PM5350: slave=%u, baud=%lu, format=%s, RX=%d, TX=%d, DE=%d, RE=%d",
    static_cast<unsigned int>(slaveId),
    MeterConfig::Pm5350::BAUD_RATE,
    MeterConfig::Pm5350::SERIAL_FORMAT_NAME,
    rxPin,
    txPin,
    dePin,
    rePin);
  pinMode(rePin, OUTPUT);
  pinMode(dePin, OUTPUT);
  setTransmitEnabled(false);

  serial.begin(
    MeterConfig::Pm5350::BAUD_RATE,
    MeterConfig::Pm5350::SERIAL_FORMAT,
    rxPin,
    txPin);
  node.begin(slaveId, serial);

  activeInstance = this;
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  SerialLogger::info("MODBUS", "PM5350 transport ready.");
}

MeterReading Pm5350Modbus::readMeter()
{
  MeterReading reading = {};
  reading.modbusValid = false;
  reading.valid = false;
  reading.modbusStatus = 0xFF;
  reading.powerFactorInvalidCount = powerFactorInvalidCount;

  if (!readFloat(VOLTAGE_LN_REGISTER, reading.voltageLN, reading.modbusStatus) ||
      !readFloat(CURRENT_A_REGISTER, reading.currentA, reading.modbusStatus) ||
      !readFloat(CURRENT_B_REGISTER, reading.currentB, reading.modbusStatus) ||
      !readFloat(CURRENT_C_REGISTER, reading.currentC, reading.modbusStatus) ||
      !readFloat(CURRENT_AVERAGE_REGISTER, reading.currentAverage, reading.modbusStatus) ||
      !readFloat(ACTIVE_POWER_TOTAL_REGISTER, reading.activePowerTotalKw, reading.modbusStatus) ||
      !readFloat(POWER_FACTOR_TOTAL_REGISTER, reading.powerFactorTotal, reading.modbusStatus) ||
      !readFloat(FREQUENCY_REGISTER, reading.frequencyHz, reading.modbusStatus) ||
      !readUint64(ACTIVE_ENERGY_DELIVERED_REGISTER, reading.activeEnergyDeliveredWh, reading.modbusStatus) ||
      !retryNonFiniteFloat(VOLTAGE_LN_REGISTER, reading.voltageLN, reading.modbusStatus, "voltage_ln") ||
      !retryNonFiniteFloat(CURRENT_A_REGISTER, reading.currentA, reading.modbusStatus, "current_a") ||
      !retryNonFiniteFloat(CURRENT_AVERAGE_REGISTER, reading.currentAverage, reading.modbusStatus, "current_average") ||
      !retryNonFiniteFloat(ACTIVE_POWER_TOTAL_REGISTER, reading.activePowerTotalKw, reading.modbusStatus, "active_power_total_kw") ||
      !retryNonFiniteFloat(FREQUENCY_REGISTER, reading.frequencyHz, reading.modbusStatus, "frequency_hz")) {
    return reading;
  }

  reading.powerFactorTotal = decodePowerFactor(reading.powerFactorTotal, reading.powerFactorLeading);
  if (!isfinite(reading.powerFactorTotal)) {
    ++powerFactorInvalidCount;
  }
  reading.powerFactorInvalidCount = powerFactorInvalidCount;
  reading.modbusValid = true;
  // 1PH2W LN only requires phase A measurements. B and C are not expected.
  // Power factor is unstable or undefined at very low real power, so the
  // PM5350 may legitimately return NaN even while current is non-zero.
  bool powerFactorApplicable = isfinite(reading.activePowerTotalKw) &&
                               fabsf(reading.activePowerTotalKw) >=
                                 MeterConfig::Pm5350::MIN_POWER_FACTOR_ACTIVE_POWER_KW;
  reading.valid = isfinite(reading.voltageLN) &&
                  isfinite(reading.currentA) &&
                  isfinite(reading.currentAverage) &&
                  isfinite(reading.activePowerTotalKw) &&
                  (!powerFactorApplicable || isfinite(reading.powerFactorTotal)) &&
                  isfinite(reading.frequencyHz);
  if (!reading.valid) {
    String fields = "Invalid measurement field(s):";
    if (!isfinite(reading.voltageLN)) fields += " voltage_ln";
    if (!isfinite(reading.currentA)) fields += " current_a";
    if (!isfinite(reading.currentAverage)) fields += " current_average";
    if (!isfinite(reading.activePowerTotalKw)) fields += " active_power_total_kw";
    if (powerFactorApplicable && !isfinite(reading.powerFactorTotal)) fields += " power_factor_total";
    if (!isfinite(reading.frequencyHz)) fields += " frequency_hz";
    SerialLogger::info("MODBUS", fields);
  }
  return reading;
}

void Pm5350Modbus::preTransmission()
{
  if (activeInstance != nullptr) {
    activeInstance->setTransmitEnabled(true);
  }
}

void Pm5350Modbus::postTransmission()
{
  if (activeInstance != nullptr) {
    activeInstance->setTransmitEnabled(false);
  }
}

float Pm5350Modbus::registersToFloat(uint16_t registerHigh, uint16_t registerLow)
{
  uint32_t combined = ((uint32_t)registerHigh << 16) | registerLow;
  float value;
  memcpy(&value, &combined, sizeof(value));
  return value;
}

uint64_t Pm5350Modbus::registersToUint64(
  uint16_t register0,
  uint16_t register1,
  uint16_t register2,
  uint16_t register3)
{
  return (static_cast<uint64_t>(register0) << 48) |
         (static_cast<uint64_t>(register1) << 32) |
         (static_cast<uint64_t>(register2) << 16) |
         static_cast<uint64_t>(register3);
}

bool Pm5350Modbus::readFloat(uint16_t startRegister, float& value, uint8_t& modbusStatus)
{
  modbusStatus = readHoldingRegisters(startRegister, FLOAT_REGISTER_COUNT);
  if (modbusStatus != node.ku8MBSuccess) {
    SerialLogger::infof(
      "MODBUS",
      "FC03 failed: register=0x%04X, count=%u, status=0x%02X",
      static_cast<unsigned int>(startRegister),
      static_cast<unsigned int>(FLOAT_REGISTER_COUNT),
      static_cast<unsigned int>(modbusStatus));
    return false;
  }

  value = registersToFloat(node.getResponseBuffer(0), node.getResponseBuffer(1));
  return true;
}

bool Pm5350Modbus::retryNonFiniteFloat(
  uint16_t startRegister,
  float& value,
  uint8_t& modbusStatus,
  const char* fieldName)
{
  if (isfinite(value)) {
    return true;
  }

  SerialLogger::infof(
    "MODBUS",
    "Non-finite measurement; field=%s, register=0x%04X, retry=1/%u",
    fieldName,
    static_cast<unsigned int>(startRegister),
    static_cast<unsigned int>(MeterConfig::Pm5350::MAX_VALUE_READ_ATTEMPTS - 1));
  delay(MeterConfig::Pm5350::VALUE_RETRY_DELAY_MS);
  return readFloat(startRegister, value, modbusStatus);
}

bool Pm5350Modbus::readUint64(uint16_t startRegister, uint64_t& value, uint8_t& modbusStatus)
{
  modbusStatus = readHoldingRegisters(startRegister, INT64_REGISTER_COUNT);
  if (modbusStatus != node.ku8MBSuccess) {
    SerialLogger::infof(
      "MODBUS",
      "FC03 failed: register=0x%04X, count=%u, status=0x%02X",
      static_cast<unsigned int>(startRegister),
      static_cast<unsigned int>(INT64_REGISTER_COUNT),
      static_cast<unsigned int>(modbusStatus));
    return false;
  }

  value = registersToUint64(
    node.getResponseBuffer(0),
    node.getResponseBuffer(1),
    node.getResponseBuffer(2),
    node.getResponseBuffer(3));
  return true;
}

uint8_t Pm5350Modbus::readHoldingRegisters(uint16_t startRegister, uint8_t registerCount)
{
  uint8_t status = 0xFF;
  for (uint8_t attempt = 1; attempt <= MeterConfig::Pm5350::MAX_READ_ATTEMPTS; ++attempt) {
    drainReceiveBuffer();
    delay(MeterConfig::Pm5350::RTU_SILENT_INTERVAL_MS);
    status = node.readHoldingRegisters(startRegister, registerCount);
    if (status == node.ku8MBSuccess ||
        !isRetryableStatus(status) ||
        attempt == MeterConfig::Pm5350::MAX_READ_ATTEMPTS) {
      return status;
    }

    SerialLogger::infof(
      "MODBUS",
      "Transient FC03 error; register=0x%04X, status=0x%02X, retry=%u/%u",
      static_cast<unsigned int>(startRegister),
      static_cast<unsigned int>(status),
      static_cast<unsigned int>(attempt),
      static_cast<unsigned int>(MeterConfig::Pm5350::MAX_READ_ATTEMPTS - 1));
    delay(MeterConfig::Pm5350::RETRY_DELAY_MS);
  }

  return status;
}

void Pm5350Modbus::drainReceiveBuffer()
{
  while (serial.available() > 0) {
    serial.read();
  }
}

bool Pm5350Modbus::isRetryableStatus(uint8_t status)
{
  return status == ModbusMaster::ku8MBInvalidSlaveID ||
         status == ModbusMaster::ku8MBInvalidFunction ||
         status == ModbusMaster::ku8MBResponseTimedOut ||
         status == ModbusMaster::ku8MBInvalidCRC;
}

float Pm5350Modbus::decodePowerFactor(float rawPowerFactor, bool& leading)
{
  leading = rawPowerFactor > 1.0f || rawPowerFactor < -1.0f;
  if (rawPowerFactor > 1.0f) {
    return 2.0f - rawPowerFactor;
  }
  if (rawPowerFactor < -1.0f) {
    return -2.0f - rawPowerFactor;
  }
  return rawPowerFactor;
}

void Pm5350Modbus::setTransmitEnabled(bool enabled)
{
  digitalWrite(rePin, enabled ? HIGH : LOW);
  digitalWrite(dePin, enabled ? HIGH : LOW);
}
