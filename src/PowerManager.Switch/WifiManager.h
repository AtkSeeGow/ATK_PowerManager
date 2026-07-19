#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WifiManager {
public:
  WifiManager(
    const char* ssid,
    const char* password,
    const char* hostname,
    unsigned long reconnectIntervalMilliseconds);

  void begin();
  void update();
  bool isConnected() const;

private:
  void connect();
  void logNetworkInformation() const;

  const char* ssid;
  const char* password;
  const char* hostname;
  unsigned long reconnectIntervalMilliseconds;
  unsigned long lastConnectionAttemptAtMilliseconds = 0;
  bool connectionAttempted = false;
  bool wasConnected = false;
  bool missingConfigurationReported = false;
};
