#include "WifiManager.h"

#include "SerialLogger.h"

namespace {
const char* wifiStatusName(int status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "idle";
    case WL_NO_SSID_AVAIL:
      return "ssid_unavailable";
    case WL_SCAN_COMPLETED:
      return "scan_completed";
    case WL_CONNECTED:
      return "connected";
    case WL_CONNECT_FAILED:
      return "connect_failed";
    case WL_CONNECTION_LOST:
      return "connection_lost";
    case WL_DISCONNECTED:
      return "disconnected";
    default:
      return "unknown";
  }
}
}

WifiManager::WifiManager(
  const char* ssid,
  const char* password,
  const char* hostname,
  unsigned long reconnectIntervalMilliseconds)
  : ssid(ssid),
    password(password),
    hostname(hostname),
    reconnectIntervalMilliseconds(reconnectIntervalMilliseconds) {
}

void WifiManager::begin() {
  WiFi.mode(WIFI_STA);
  bool dhcpConfigured = WiFi.config(
    INADDR_NONE,
    INADDR_NONE,
    INADDR_NONE,
    INADDR_NONE);
  bool hostnameConfigured = WiFi.setHostname(hostname);
  String macAddress = WiFi.macAddress();
  SerialLogger::infof(
    "WIFI",
    "Station initialized; MAC=%s, hostname=%s, hostname_applied=%s, DHCP=%s, reconnect_interval_ms=%lu",
    macAddress.c_str(),
    hostname,
    hostnameConfigured ? "true" : "false",
    dhcpConfigured ? "true" : "false",
    reconnectIntervalMilliseconds);
  connect();
}

void WifiManager::update() {
  bool connected = isConnected();
  if (connected) {
    if (!wasConnected)
      logNetworkInformation();

    wasConnected = true;
    return;
  }

  if (wasConnected) {
    int status = static_cast<int>(WiFi.status());
    SerialLogger::infof(
      "WIFI",
      "Connection lost; status=%s(%d)",
      wifiStatusName(status),
      status);
  }

  wasConnected = false;
  unsigned long now = millis();
  if (!connectionAttempted ||
      now - lastConnectionAttemptAtMilliseconds >=
        reconnectIntervalMilliseconds) {
    connect();
  }
}

bool WifiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

void WifiManager::connect() {
  if (ssid == nullptr || ssid[0] == '\0') {
    if (!missingConfigurationReported) {
      SerialLogger::info(
        "WIFI",
        "Wi-Fi is not configured; create SwitchSecrets.h.");
      missingConfigurationReported = true;
    }
    lastConnectionAttemptAtMilliseconds = millis();
    connectionAttempted = true;
    return;
  }

  int previousStatus = static_cast<int>(WiFi.status());
  SerialLogger::infof(
    "WIFI",
    "Connecting; SSID=%s, hostname=%s, previous_status=%s(%d)",
    ssid,
    hostname,
    wifiStatusName(previousStatus),
    previousStatus);
  WiFi.begin(ssid, password);
  lastConnectionAttemptAtMilliseconds = millis();
  connectionAttempted = true;
}

void WifiManager::logNetworkInformation() const {
  String connectedSsid = WiFi.SSID();
  String bssid = WiFi.BSSIDstr();
  String macAddress = WiFi.macAddress();
  String localIp = WiFi.localIP().toString();
  String gatewayIp = WiFi.gatewayIP().toString();
  String subnetMask = WiFi.subnetMask().toString();
  String primaryDns = WiFi.dnsIP(0).toString();
  String secondaryDns = WiFi.dnsIP(1).toString();

  SerialLogger::infof(
    "WIFI",
    "Connected; SSID=%s, BSSID=%s, channel=%d, RSSI=%d dBm, MAC=%s",
    connectedSsid.c_str(),
    bssid.c_str(),
    static_cast<int>(WiFi.channel()),
    static_cast<int>(WiFi.RSSI()),
    macAddress.c_str());
  SerialLogger::infof(
    "WIFI",
    "IPv4; address=%s, gateway=%s, subnet=%s, DNS=%s,%s",
    localIp.c_str(),
    gatewayIp.c_str(),
    subnetMask.c_str(),
    primaryDns.c_str(),
    secondaryDns.c_str());
}
