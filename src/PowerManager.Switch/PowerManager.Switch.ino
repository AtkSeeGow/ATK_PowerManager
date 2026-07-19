#include "GpioOutputController.h"
#include "GpioReader.h"
#include "NtpTimeSynchronizer.h"
#include "SerialLogger.h"
#include "SwitchApiServer.h"
#include "SwitchConfiguration.h"
#include "WifiManager.h"

GpioReader gpioReader;
GpioOutputController gpioOutputController;
SwitchApiServer switchApiServer(gpioReader, gpioOutputController);
WifiManager wifiManager(
  SwitchConfiguration::WIFI_SSID,
  SwitchConfiguration::WIFI_PASSWORD,
  SwitchConfiguration::WIFI_HOSTNAME,
  SwitchConfiguration::WIFI_RECONNECT_INTERVAL_MILLISECONDS);
NtpTimeSynchronizer ntpTimeSynchronizer;

void setup() {
  SerialLogger::begin(9600);
  SerialLogger::info("SERVICE", "Initializing power switch service.");

  gpioReader.begin();
  gpioOutputController.begin();
  wifiManager.begin();
  ntpTimeSynchronizer.begin();
}

void loop() {
  wifiManager.update();
  ntpTimeSynchronizer.update(wifiManager.isConnected());
  switchApiServer.setNetworkAvailable(wifiManager.isConnected());
  switchApiServer.update();
  gpioOutputController.update();

  if (millis() > 86400000UL) {
    SerialLogger::info("SERVICE", "Daily restart requested.");
    ESP.restart();
  }
}
