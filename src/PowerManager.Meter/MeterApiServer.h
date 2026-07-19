#pragma once

#include <Arduino.h>
#include <time.h>
#include <WebServer.h>
#include "MeterReading.h"

class MeterApiServer {
public:
  MeterApiServer(const char* ssid, const char* password, const char* hostname, uint16_t port);

  void begin(unsigned long bootStartedAtMs);
  void loop();
  void updateMeter(const MeterReading& reading);

private:
  const char* ssid;
  const char* password;
  const char* hostname;
  WebServer server;
  portMUX_TYPE readingMux = portMUX_INITIALIZER_UNLOCKED;
  MeterReading reading;
  unsigned long updatedAtMs;
  unsigned long lastWifiReconnectAtMs;
  unsigned long bootStartedAtMs;
  time_t bootEpochSeconds;
  bool wasWifiConnected;
  bool ntpSynchronized;

  void maintainWifiConnection();
  void updateNtpTime();
  MeterReading getMeterSnapshot(unsigned long& snapshotUpdatedAtMs);
  void sendJson(int statusCode, const String& body);
  static void appendJsonFloat(String& response, float value, bool valid);
  static void appendJsonUint64(String& response, uint64_t value, bool valid);
  static void appendJsonKwhFromWh(String& response, uint64_t valueWh, bool valid);
  static void appendJsonUtcTime(String& response, time_t value, bool valid);
  static void appendJsonEpoch(String& response, time_t value, bool valid);
  void handleRoot();
  void handleMeterApi();
};
