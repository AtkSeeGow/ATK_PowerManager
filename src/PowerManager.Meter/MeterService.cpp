#include "MeterService.h"
#include "MeterConfig.h"
#include "SerialLogger.h"

#include <math.h>
#include <stdio.h>

MeterService::MeterService(
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
  int am2320SclPin)
  : meterApiServer(ssid, password, hostname, MeterConfig::Http::PORT),
    pm5350(modbusSerial, slaveId, rxPin, txPin, dePin, rePin),
    am2320(am2320SdaPin, am2320SclPin),
    meterReadTaskHandle(nullptr) {
}

void MeterService::begin(unsigned long setupStartedAtMs) {
  SerialLogger::begin(MeterConfig::Logging::BAUD_RATE);
  SerialLogger::info("SERVICE", "Starting ESP32 PM5350 meter service.");
  SerialLogger::infof("SERVICE", "setup() entered at uptime=%lu ms.", setupStartedAtMs);
  pm5350.begin();
  am2320.begin();

  meterApiServer.begin(setupStartedAtMs);
  BaseType_t taskCreated = xTaskCreate(
    meterReadTaskEntry,
    "MeterReadTask",
    MeterConfig::Service::READ_TASK_STACK_SIZE,
    this,
    MeterConfig::Service::READ_TASK_PRIORITY,
    &meterReadTaskHandle);
  if (taskCreated != pdPASS) {
    meterReadTaskHandle = nullptr;
    SerialLogger::info("SERVICE", "Failed to create the meter read task; cached readings will remain unavailable.");
    return;
  }

  SerialLogger::infof(
    "SERVICE",
    "Meter read task started; first read is scheduled in %lu ms.",
    MeterConfig::Service::READ_INTERVAL_MS);
}

void MeterService::loop() {
  meterApiServer.loop();
  delay(1);
}

void MeterService::meterReadTaskEntry(void* context) {
  MeterService* service = static_cast<MeterService*>(context);
  service->runMeterReadTask();
}

void MeterService::runMeterReadTask() {
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(MeterConfig::Service::READ_INTERVAL_MS));
    readAndPublishMeter();
  }
}

void MeterService::readAndPublishMeter() {
  SerialLogger::info("SERVICE", "Starting meter read cycle.");
  MeterReading reading = pm5350.readMeter();
  Am2320Reading environment = am2320.read();
  reading.temperatureC = environment.temperatureC;
  reading.humidityPercent = environment.humidityPercent;
  reading.am2320Valid = environment.valid;
  reading.am2320Status = static_cast<uint8_t>(environment.status);
  reading.am2320WireStatus = environment.wireStatus;
  reading.am2320Diagnostics = environment.diagnostics;

  meterApiServer.updateMeter(reading);
  logMeterReading(reading);
}

void MeterService::logMeterReading(const MeterReading& reading) {
  if (!reading.modbusValid) {
    SerialLogger::infof("SERVICE", "Meter read failed, status: 0x%02X", static_cast<unsigned int>(reading.modbusStatus));
    return;
  }

  String powerFactor = formatFloat(reading.powerFactorTotal, 3);
  if (isfinite(reading.powerFactorTotal)) {
    powerFactor += reading.powerFactorLeading ? " leading" : " lagging";
  }

  String message = "Vln=";
  message += formatFloat(reading.voltageLN, 3);
  message += " V | Ia=";
  message += formatFloat(reading.currentA, 3);
  message += " A | Ib=";
  message += formatFloat(reading.currentB, 3);
  message += " A | Ic=";
  message += formatFloat(reading.currentC, 3);
  message += " A | Iavg=";
  message += formatFloat(reading.currentAverage, 3);
  message += " A | P=";
  message += formatFloat(reading.activePowerTotalKw, 3);
  message += " kW | PF=";
  message += powerFactor;
  message += " | F=";
  message += formatFloat(reading.frequencyHz, 3);
  message += " Hz | E=";
  message += formatEnergyKwh(reading.activeEnergyDeliveredWh);
  message += " kWh | T=";
  message += reading.am2320Valid ? formatFloat(reading.temperatureC, 1) : "N/A";
  message += " C | RH=";
  message += reading.am2320Valid ? formatFloat(reading.humidityPercent, 1) : "N/A";
  message += " %";
  message += " | status=0x";
  message += String(reading.modbusStatus, HEX);
  SerialLogger::info("METER", message);

  if (!reading.valid) {
    SerialLogger::info("SERVICE", "Warning: one or more measurement values are unavailable.");
  }
  if (!reading.am2320Valid) {
    SerialLogger::infof(
      "SERVICE",
      "AM2320 read failed: status=%u, Wire status=%u",
      static_cast<unsigned int>(reading.am2320Status),
      static_cast<unsigned int>(reading.am2320WireStatus));
  }
}

String MeterService::formatFloat(float value, uint8_t decimals) {
  return isfinite(value) ? String(value, static_cast<unsigned int>(decimals)) : "N/A";
}

String MeterService::formatEnergyKwh(uint64_t energyWh) {
  char valueText[32];
  unsigned long long wholeKwh = static_cast<unsigned long long>(energyWh / 1000);
  unsigned long long remainingWh = static_cast<unsigned long long>(energyWh % 1000);
  snprintf(valueText, sizeof(valueText), "%llu.%03llu", wholeKwh, remainingWh);
  return String(valueText);
}
