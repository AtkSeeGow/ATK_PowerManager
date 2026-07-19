#pragma once

#include <WebServer.h>

#include "GpioOutputController.h"
#include "GpioReader.h"

class SwitchApiServer {
public:
  SwitchApiServer(
    GpioReader& gpioReader,
    GpioOutputController& gpioOutputController);

  void setNetworkAvailable(bool available);
  void update();

private:
  static const int MAXIMUM_PULSE_DURATION_MILLISECONDS = 3600000;

  void registerRoutes();
  void handleHeartbeat();
  void handleGetDigitalValue();
  void handleSetDigitalValue();
  void handlePulseDigitalValue();
  bool tryGetIntArgument(const char* name, int& value) const;
  void sendJson(int statusCode, const String& body);

  WebServer server;
  GpioReader& gpioReader;
  GpioOutputController& gpioOutputController;
  bool routesRegistered = false;
  bool started = false;
};
