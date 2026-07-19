#pragma once

// Copy SwitchSecrets.example.h to SwitchSecrets.h and fill in the local
// credentials. SwitchSecrets.h is intentionally excluded from source control.
#if defined(__has_include)
#if __has_include("SwitchSecrets.h")
#include "SwitchSecrets.h"
#endif
#endif

#ifndef POWER_MANAGER_WIFI_SSID
#define POWER_MANAGER_WIFI_SSID "TP-Link IoT"
#endif

#ifndef POWER_MANAGER_WIFI_PASSWORD
#define POWER_MANAGER_WIFI_PASSWORD "0986230207"
#endif

#ifndef POWER_MANAGER_WIFI_HOSTNAME
#define POWER_MANAGER_WIFI_HOSTNAME "ESP32-SWITCH"
#endif

namespace SwitchConfiguration {
  constexpr const char* WIFI_SSID = POWER_MANAGER_WIFI_SSID;
  constexpr const char* WIFI_PASSWORD = POWER_MANAGER_WIFI_PASSWORD;
  constexpr const char* WIFI_HOSTNAME = POWER_MANAGER_WIFI_HOSTNAME;
  constexpr unsigned long WIFI_RECONNECT_INTERVAL_MILLISECONDS = 10000UL;

  // POSIX timezone: Taiwan Standard Time (UTC+8, no daylight saving time).
  constexpr const char* TIME_ZONE = "CST-8";
  constexpr const char* NTP_SERVER_PRIMARY = "pool.ntp.org";
  constexpr const char* NTP_SERVER_SECONDARY = "time.nist.gov";

  constexpr int GPIO_INPUT_PINS[] = {35};
  constexpr unsigned int GPIO_INPUT_PIN_COUNT =
    sizeof(GPIO_INPUT_PINS) / sizeof(GPIO_INPUT_PINS[0]);

  constexpr int GPIO_OUTPUT_PINS[] = {25};
  constexpr unsigned int GPIO_OUTPUT_PIN_COUNT =
    sizeof(GPIO_OUTPUT_PINS) / sizeof(GPIO_OUTPUT_PINS[0]);
}
