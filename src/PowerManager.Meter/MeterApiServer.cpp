#include "MeterApiServer.h"
#include "MeterConfig.h"
#include "SerialLogger.h"

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <WiFi.h>

MeterApiServer::MeterApiServer(const char* ssid, const char* password, const char* hostname, uint16_t port)
  : ssid(ssid),
    password(password),
    hostname(hostname),
    server(port),
    updatedAtMs(0),
    lastWifiReconnectAtMs(0),
    bootStartedAtMs(0),
    bootEpochSeconds(0),
    wasWifiConnected(false),
    ntpSynchronized(false)
{
  reading = {};
  reading.modbusValid = false;
  reading.valid = false;
  reading.modbusStatus = 0xFF;
  reading.am2320Valid = false;
  reading.am2320Status = 0xFF;
  reading.am2320WireStatus = 0xFF;
}

void MeterApiServer::begin(unsigned long setupStartedAtMs)
{
  bootStartedAtMs = setupStartedAtMs;
  SerialLogger::info("WIFI", String("Starting station mode for SSID: ") + ssid);
  WiFi.mode(WIFI_STA);
  // Power-save mode can add noticeable latency to short, infrequent LAN requests.
  WiFi.setSleep(MeterConfig::Wifi::SLEEP_ENABLED);
  bool hostnameConfigured = WiFi.setHostname(hostname);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  lastWifiReconnectAtMs = millis();
  SerialLogger::infof(
    "WIFI",
    "Connecting; hostname=%s, hostname_applied=%s",
    hostname,
    hostnameConfigured ? "true" : "false");
  configTzTime(
    MeterConfig::Ntp::TIMEZONE,
    MeterConfig::Ntp::PRIMARY_SERVER,
    MeterConfig::Ntp::SECONDARY_SERVER);
  SerialLogger::info("NTP", "Time synchronization requested.");

  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/api/meter", HTTP_GET, [this]() { handleMeterApi(); });
  server.onNotFound([this]() {
    SerialLogger::info("HTTP", String("404 Not Found: ") + server.uri());
    sendJson(404, "{\"error\":\"not_found\"}");
  });
  server.begin();
  SerialLogger::info("HTTP", "Server started; routes: GET /, GET /api/meter");
}

void MeterApiServer::loop()
{
  maintainWifiConnection();
  updateNtpTime();
  server.handleClient();
}

void MeterApiServer::updateMeter(const MeterReading& newReading)
{
  portENTER_CRITICAL(&readingMux);
  reading = newReading;
  updatedAtMs = millis();
  portEXIT_CRITICAL(&readingMux);
}

MeterReading MeterApiServer::getMeterSnapshot(unsigned long& snapshotUpdatedAtMs)
{
  MeterReading snapshot;
  portENTER_CRITICAL(&readingMux);
  snapshot = reading;
  snapshotUpdatedAtMs = updatedAtMs;
  portEXIT_CRITICAL(&readingMux);
  return snapshot;
}

void MeterApiServer::maintainWifiConnection()
{
  if (WiFi.status() == WL_CONNECTED) {
    if (!wasWifiConnected) {
      wasWifiConnected = true;
      SerialLogger::info("WIFI", String("Connected. API URL: http://") + WiFi.localIP().toString() + "/api/meter");
    }
    return;
  }

  if (wasWifiConnected) {
    SerialLogger::info("WIFI", "Connection lost.");
    wasWifiConnected = false;
  }
  if (millis() - lastWifiReconnectAtMs < MeterConfig::Wifi::RECONNECT_INTERVAL_MS) {
    return;
  }

  lastWifiReconnectAtMs = millis();
  SerialLogger::info("WIFI", "Reconnecting...");
  WiFi.begin(ssid, password);
}

void MeterApiServer::updateNtpTime()
{
  if (ntpSynchronized || WiFi.status() != WL_CONNECTED) {
    return;
  }

  time_t currentEpochSeconds = time(nullptr);
  if (currentEpochSeconds < MeterConfig::Ntp::MIN_VALID_EPOCH) {
    return;
  }

  unsigned long elapsedSinceSetupMs = millis() - bootStartedAtMs;
  bootEpochSeconds = currentEpochSeconds - static_cast<time_t>(elapsedSinceSetupMs / 1000);
  ntpSynchronized = true;
  SerialLogger::infof(
    "NTP",
    "Synchronized. Boot time epoch=%lld, current epoch=%lld",
    static_cast<long long>(bootEpochSeconds),
    static_cast<long long>(currentEpochSeconds));
}

void MeterApiServer::sendJson(int statusCode, const String& body)
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(statusCode, "application/json; charset=utf-8", body);
  SerialLogger::infof("HTTP", "Response %d, %u bytes", statusCode, static_cast<unsigned int>(body.length()));
}

void MeterApiServer::handleRoot()
{
  SerialLogger::info("HTTP", "GET /");
  sendJson(200, "{\"api\":\"/api/meter\",\"method\":\"GET\"}");
}

void MeterApiServer::handleMeterApi()
{
  unsigned long snapshotUpdatedAtMs;
  MeterReading snapshot = getMeterSnapshot(snapshotUpdatedAtMs);
  SerialLogger::infof(
    "HTTP",
    "GET /api/meter (valid=%s, modbus_valid=%s, status=0x%02X)",
    snapshot.valid ? "true" : "false",
    snapshot.modbusValid ? "true" : "false",
    static_cast<unsigned int>(snapshot.modbusStatus));
  String response = "{\"voltage_ln\":";
  response.reserve(768);
  appendJsonFloat(response, snapshot.voltageLN, snapshot.modbusValid);

  response += ",\"current_a\":";
  appendJsonFloat(response, snapshot.currentA, snapshot.modbusValid);
  response += ",\"current_b\":";
  appendJsonFloat(response, snapshot.currentB, snapshot.modbusValid);
  response += ",\"current_c\":";
  appendJsonFloat(response, snapshot.currentC, snapshot.modbusValid);
  response += ",\"current_average\":";
  appendJsonFloat(response, snapshot.currentAverage, snapshot.modbusValid);
  response += ",\"active_power_total_kw\":";
  appendJsonFloat(response, snapshot.activePowerTotalKw, snapshot.modbusValid);
  response += ",\"power_factor_total\":";
  appendJsonFloat(response, snapshot.powerFactorTotal, snapshot.modbusValid);
  response += ",\"power_factor_leading\":";
  response += snapshot.modbusValid && isfinite(snapshot.powerFactorTotal)
                ? (snapshot.powerFactorLeading ? "true" : "false")
                : "null";
  response += ",\"frequency_hz\":";
  appendJsonFloat(response, snapshot.frequencyHz, snapshot.modbusValid);
  response += ",\"active_energy_delivered_wh\":";
  appendJsonUint64(response, snapshot.activeEnergyDeliveredWh, snapshot.modbusValid);
  response += ",\"active_energy_delivered_kwh\":";
  appendJsonKwhFromWh(response, snapshot.activeEnergyDeliveredWh, snapshot.modbusValid);
  response += ",\"temperature_c\":";
  appendJsonFloat(response, snapshot.temperatureC, snapshot.am2320Valid);
  response += ",\"humidity_percent\":";
  appendJsonFloat(response, snapshot.humidityPercent, snapshot.am2320Valid);
  response += ",\"am2320_valid\":";
  response += snapshot.am2320Valid ? "true" : "false";
  response += ",\"am2320_status\":";
  response += String(snapshot.am2320Status);
  response += ",\"am2320_wire_status\":";
  response += String(snapshot.am2320WireStatus);
  response += ",\"am2320_failure_counts\":{\"not_initialized\":";
  appendJsonUint64(response, snapshot.am2320Diagnostics.failureCount(Am2320Status::NotInitialized), true);
  response += ",\"command_failed\":";
  appendJsonUint64(response, snapshot.am2320Diagnostics.failureCount(Am2320Status::CommandFailed), true);
  response += ",\"response_too_short\":";
  appendJsonUint64(response, snapshot.am2320Diagnostics.failureCount(Am2320Status::ResponseTooShort), true);
  response += ",\"invalid_frame\":";
  appendJsonUint64(response, snapshot.am2320Diagnostics.failureCount(Am2320Status::InvalidFrame), true);
  response += ",\"crc_mismatch\":";
  appendJsonUint64(response, snapshot.am2320Diagnostics.failureCount(Am2320Status::CrcMismatch), true);
  response += "}";
  response += ",\"am2320_retry_recovery_count\":";
  appendJsonUint64(response, snapshot.am2320Diagnostics.retryRecoveryCount, true);

  time_t currentEpochSeconds = ntpSynchronized ? time(nullptr) : 0;
  response += ",\"ntp_synchronized\":";
  response += ntpSynchronized ? "true" : "false";
  response += ",\"current_time_utc\":";
  appendJsonUtcTime(response, currentEpochSeconds, ntpSynchronized);
  response += ",\"boot_started_at_utc\":";
  appendJsonUtcTime(response, bootEpochSeconds, ntpSynchronized);
  response += ",\"boot_started_at_epoch\":";
  appendJsonEpoch(response, bootEpochSeconds, ntpSynchronized);
  response += ",\"uptime_ms\":";
  response += String(millis());

  response += ",\"units\":{\"voltage\":\"V\",\"current\":\"A\",\"active_power\":\"kW\",\"frequency\":\"Hz\",\"active_energy\":\"Wh\"}";
  response += ",\"valid\":";
  response += snapshot.valid ? "true" : "false";
  response += ",\"modbus_valid\":";
  response += snapshot.modbusValid ? "true" : "false";
  response += ",\"updated_at_ms\":";
  response += String(snapshotUpdatedAtMs);
  response += ",\"modbus_status\":";
  response += String(snapshot.modbusStatus);
  response += ",\"pm5350_power_factor_invalid_count\":";
  appendJsonUint64(response, snapshot.powerFactorInvalidCount, true);
  response += "}";

  sendJson(200, response);
}

void MeterApiServer::appendJsonFloat(String& response, float value, bool valid)
{
  response += valid && isfinite(value) ? String(value, 3) : "null";
}

void MeterApiServer::appendJsonUint64(String& response, uint64_t value, bool valid)
{
  if (!valid) {
    response += "null";
    return;
  }

  char valueText[24];
  snprintf(valueText, sizeof(valueText), "%llu", static_cast<unsigned long long>(value));
  response += valueText;
}

void MeterApiServer::appendJsonKwhFromWh(String& response, uint64_t valueWh, bool valid)
{
  if (!valid) {
    response += "null";
    return;
  }

  char valueText[32];
  unsigned long long wholeKwh = static_cast<unsigned long long>(valueWh / 1000);
  unsigned long long remainingWh = static_cast<unsigned long long>(valueWh % 1000);
  snprintf(valueText, sizeof(valueText), "%llu.%03llu", wholeKwh, remainingWh);
  response += valueText;
}

void MeterApiServer::appendJsonUtcTime(String& response, time_t value, bool valid)
{
  if (!valid) {
    response += "null";
    return;
  }

  struct tm timeInfo;
  gmtime_r(&value, &timeInfo);
  char valueText[25];
  strftime(valueText, sizeof(valueText), "%Y-%m-%dT%H:%M:%SZ", &timeInfo);
  response += '"';
  response += valueText;
  response += '"';
}

void MeterApiServer::appendJsonEpoch(String& response, time_t value, bool valid)
{
  if (!valid) {
    response += "null";
    return;
  }

  char valueText[24];
  snprintf(valueText, sizeof(valueText), "%lld", static_cast<long long>(value));
  response += valueText;
}
