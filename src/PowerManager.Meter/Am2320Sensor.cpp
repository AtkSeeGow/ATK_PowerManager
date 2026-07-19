#include "Am2320Sensor.h"
#include "MeterConfig.h"
#include "SerialLogger.h"

#include <math.h>

Am2320Sensor::Am2320Sensor(int sdaPin, int sclPin, TwoWire& wire)
  : wire(wire),
    sdaPin(sdaPin),
    sclPin(sclPin),
    initialized(false),
    diagnostics{}
{
}

bool Am2320Sensor::begin()
{
  SerialLogger::infof("AM2320", "Initializing I2C: SDA=%d, SCL=%d, address=0x%02X", sdaPin, sclPin, I2C_ADDRESS);
  initialized = wire.begin(sdaPin, sclPin, MeterConfig::Am2320::I2C_CLOCK_HZ);
  if (!initialized) {
    SerialLogger::info("AM2320", "I2C initialization failed.");
    return false;
  }

  SerialLogger::info("AM2320", "I2C transport ready.");
  runStartupDiagnostics();
  return true;
}

Am2320Reading Am2320Sensor::read()
{
  Am2320Reading reading;
  for (uint8_t attempt = 1; attempt <= MeterConfig::Am2320::MAX_READ_ATTEMPTS; ++attempt) {
    reading = readOnce();
    if (reading.valid) {
      if (attempt > 1) {
        ++diagnostics.retryRecoveryCount;
        SerialLogger::infof(
          "AM2320",
          "Read recovered on retry %u/%u.",
          static_cast<unsigned int>(attempt - 1),
          static_cast<unsigned int>(MeterConfig::Am2320::MAX_READ_ATTEMPTS - 1));
      }
      attachDiagnostics(reading);
      return reading;
    }

    recordFailure(reading.status);
    if (attempt < MeterConfig::Am2320::MAX_READ_ATTEMPTS) {
      SerialLogger::infof(
        "AM2320",
        "Read failed: status=%s, Wire status=%u, retry=%u/%u",
        statusToString(reading.status),
        static_cast<unsigned int>(reading.wireStatus),
        static_cast<unsigned int>(attempt),
        static_cast<unsigned int>(MeterConfig::Am2320::MAX_READ_ATTEMPTS - 1));
      delay(MeterConfig::Am2320::RETRY_DELAY_MS);
    }
  }

  attachDiagnostics(reading);
  return reading;
}

Am2320Reading Am2320Sensor::readOnce()
{
  Am2320Reading reading = {};
  reading.temperatureC = NAN;
  reading.humidityPercent = NAN;
  reading.valid = false;
  reading.status = Am2320Status::NotInitialized;
  reading.wireStatus = 0xFF;
  if (!initialized) {
    SerialLogger::info("AM2320", "Read skipped: sensor is not initialized.");
    return reading;
  }

  // AM2320 sleeps between reads. Addressing it first wakes it; a NACK is expected here.
  wire.beginTransmission(I2C_ADDRESS);
  wire.endTransmission();
  delay(2);

  wire.beginTransmission(I2C_ADDRESS);
  wire.write(READ_FUNCTION);
  wire.write(REGISTER_START);
  wire.write(REGISTER_COUNT);
  reading.wireStatus = wire.endTransmission();
  if (reading.wireStatus != 0) {
    reading.status = Am2320Status::CommandFailed;
    SerialLogger::infof("AM2320", "Read command failed: Wire status=%u", static_cast<unsigned int>(reading.wireStatus));
    return reading;
  }

  delay(2);
  uint8_t response[RESPONSE_SIZE];
  uint8_t responseLength = 0;
  uint8_t bytesReceived = wire.requestFrom(I2C_ADDRESS, RESPONSE_SIZE);
  while (wire.available() && responseLength < RESPONSE_SIZE) {
    response[responseLength++] = static_cast<uint8_t>(wire.read());
  }

  if (bytesReceived != RESPONSE_SIZE || responseLength != RESPONSE_SIZE) {
    reading.status = Am2320Status::ResponseTooShort;
    SerialLogger::infof(
      "AM2320",
      "Invalid response length: requested=%u, received=%u, read=%u",
      static_cast<unsigned int>(RESPONSE_SIZE),
      static_cast<unsigned int>(bytesReceived),
      static_cast<unsigned int>(responseLength));
    return reading;
  }

  if (response[0] != READ_FUNCTION || response[1] != REGISTER_COUNT) {
    reading.status = Am2320Status::InvalidFrame;
    SerialLogger::infof("AM2320", "Invalid response frame: function=0x%02X, length=%u", response[0], response[1]);
    return reading;
  }

  uint16_t expectedCrc = calculateCrc(response, 6);
  uint16_t receivedCrc = static_cast<uint16_t>(response[6]) |
                         (static_cast<uint16_t>(response[7]) << 8);
  if (receivedCrc != expectedCrc) {
    reading.status = Am2320Status::CrcMismatch;
    SerialLogger::infof("AM2320", "CRC mismatch: expected=0x%04X, received=0x%04X", expectedCrc, receivedCrc);
    return reading;
  }

  uint16_t rawHumidity = (static_cast<uint16_t>(response[2]) << 8) | response[3];
  uint16_t rawTemperature = (static_cast<uint16_t>(response[4]) << 8) | response[5];
  reading.humidityPercent = rawHumidity / 10.0f;
  reading.temperatureC = (rawTemperature & 0x8000)
                           ? -static_cast<float>(rawTemperature & 0x7FFF) / 10.0f
                           : static_cast<float>(rawTemperature) / 10.0f;
  reading.valid = true;
  reading.status = Am2320Status::Success;
  reading.wireStatus = 0;
  return reading;
}

void Am2320Sensor::recordFailure(Am2320Status status)
{
  uint8_t index = static_cast<uint8_t>(status);
  if (index > static_cast<uint8_t>(Am2320Status::Success) &&
      index <= static_cast<uint8_t>(Am2320Status::CrcMismatch)) {
    ++diagnostics.failureCounts[index];
  }
}

void Am2320Sensor::attachDiagnostics(Am2320Reading& reading) const
{
  reading.diagnostics = diagnostics;
}

uint16_t Am2320Sensor::calculateCrc(const uint8_t* data, uint8_t length)
{
  uint16_t crc = 0xFFFF;
  for (uint8_t index = 0; index < length; ++index) {
    crc ^= data[index];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
  }
  return crc;
}

const char* Am2320Sensor::statusToString(Am2320Status status)
{
  switch (status) {
    case Am2320Status::Success: return "success";
    case Am2320Status::NotInitialized: return "not_initialized";
    case Am2320Status::CommandFailed: return "command_failed";
    case Am2320Status::ResponseTooShort: return "response_too_short";
    case Am2320Status::InvalidFrame: return "invalid_frame";
    case Am2320Status::CrcMismatch: return "crc_mismatch";
  }
  return "unknown";
}

void Am2320Sensor::runStartupDiagnostics()
{
  int sdaLevel = digitalRead(sdaPin);
  int sclLevel = digitalRead(sclPin);
  SerialLogger::infof("AM2320", "I2C idle pin level: SDA=%d, SCL=%d (both should be 1)", sdaLevel, sclLevel);

  uint8_t wakeStatus = probeAddress(I2C_ADDRESS);
  delay(2);
  uint8_t readyStatus = probeAddress(I2C_ADDRESS);
  SerialLogger::infof(
    "AM2320",
    "Address 0x%02X probe: wake status=%u, after-wake status=%u (expected after-wake=0)",
    I2C_ADDRESS,
    static_cast<unsigned int>(wakeStatus),
    static_cast<unsigned int>(readyStatus));

  uint8_t deviceCount = 0;
  bool busErrorDetected = false;
  SerialLogger::info("AM2320", "Starting I2C address scan.");
  for (uint8_t address = 0x03; address < 0x78; ++address) {
    uint8_t status = probeAddress(address);
    if (status == 0) {
      ++deviceCount;
      SerialLogger::infof("AM2320", "I2C device detected at 0x%02X", static_cast<unsigned int>(address));
    } else if (status == 4) {
      busErrorDetected = true;
    }
  }

  if (deviceCount == 0) {
    SerialLogger::info("AM2320", "I2C scan found no responding devices.");
  } else {
    SerialLogger::infof("AM2320", "I2C scan completed: %u device(s) found.", static_cast<unsigned int>(deviceCount));
  }
  if (busErrorDetected) {
    SerialLogger::info("AM2320", "I2C scan encountered Wire status=4; check pull-ups, power, GND, and SDA/SCL shorts.");
  }
}

uint8_t Am2320Sensor::probeAddress(uint8_t address)
{
  wire.beginTransmission(address);
  return wire.endTransmission();
}
