#include "SwitchApiServer.h"

#include "SerialLogger.h"

#include <limits.h>

SwitchApiServer::SwitchApiServer(
  GpioReader& gpioReader,
  GpioOutputController& gpioOutputController)
  : server(80),
    gpioReader(gpioReader),
    gpioOutputController(gpioOutputController) {
}

void SwitchApiServer::setNetworkAvailable(bool available) {
  if (available && !started) {
    if (!routesRegistered)
      registerRoutes();

    server.begin();
    started = true;
    SerialLogger::info("HTTP", "Server started on port 80.");
    return;
  }

  if (!available && started) {
    server.stop();
    started = false;
    SerialLogger::info("HTTP", "Server stopped because network is unavailable.");
  }
}

void SwitchApiServer::update() {
  if (started)
    server.handleClient();
}

void SwitchApiServer::registerRoutes() {
  server.on("/api/heartbeat", HTTP_GET, [this]() {
    handleHeartbeat();
  });
  server.on("/api/gpio", HTTP_GET, [this]() {
    handleGetDigitalValue();
  });
  server.on("/api/gpio", HTTP_POST, [this]() {
    handleSetDigitalValue();
  });
  server.on("/api/gpio/pulse", HTTP_POST, [this]() {
    handlePulseDigitalValue();
  });
  server.onNotFound([this]() {
    SerialLogger::info("HTTP", String("404 Not Found: ") + server.uri());
    sendJson(404, "{\"error\":\"not_found\"}");
  });
  routesRegistered = true;
}

void SwitchApiServer::handleHeartbeat() {
  SerialLogger::info("HTTP", "GET /api/heartbeat");
  sendJson(200, "{\"status\":\"ok\"}");
}

void SwitchApiServer::handleGetDigitalValue() {
  SerialLogger::info("HTTP", "GET /api/gpio");
  int pin;
  int value;
  if (!tryGetIntArgument("pin", pin)) {
    sendJson(400, "{\"error\":\"invalid_pin\"}");
    return;
  }

  if (!gpioReader.read(pin, value)) {
    sendJson(400, "{\"error\":\"unsupported_pin\"}");
    return;
  }

  sendJson(200, "{\"pin\":" + String(pin) + ",\"value\":" + String(value) + "}");
}

void SwitchApiServer::handleSetDigitalValue() {
  SerialLogger::info("HTTP", "POST /api/gpio");
  int pin;
  int value;
  if (!tryGetIntArgument("pin", pin) ||
      !tryGetIntArgument("value", value) ||
      (value != 0 && value != 1)) {
    sendJson(400, "{\"error\":\"invalid_command\"}");
    return;
  }

  if (!gpioOutputController.setDigitalValue(pin, value == 1)) {
    sendJson(400, "{\"error\":\"unsupported_pin\"}");
    return;
  }

  sendJson(200, "{\"pin\":" + String(pin) + ",\"value\":" + String(value) + "}");
}

void SwitchApiServer::handlePulseDigitalValue() {
  SerialLogger::info("HTTP", "POST /api/gpio/pulse");
  int pin;
  int value;
  int durationMilliseconds;
  if (!tryGetIntArgument("pin", pin) ||
      !tryGetIntArgument("value", value) ||
      !tryGetIntArgument("durationMs", durationMilliseconds) ||
      (value != 0 && value != 1) ||
      durationMilliseconds <= 0 ||
      durationMilliseconds > MAXIMUM_PULSE_DURATION_MILLISECONDS) {
    sendJson(400, "{\"error\":\"invalid_command\"}");
    return;
  }

  if (!gpioOutputController.pulseDigitalValue(
        pin,
        value == 1,
        static_cast<unsigned long>(durationMilliseconds))) {
    sendJson(400, "{\"error\":\"unsupported_pin\"}");
    return;
  }

  sendJson(
    200,
    "{\"pin\":" + String(pin) +
      ",\"value\":" + String(value) +
      ",\"durationMs\":" + String(durationMilliseconds) + "}");
}

bool SwitchApiServer::tryGetIntArgument(const char* name, int& value) const {
  if (!server.hasArg(name))
    return false;

  String text = server.arg(name);
  if (text.length() == 0)
    return false;

  unsigned long parsedValue = 0;
  for (unsigned int index = 0; index < text.length(); index++) {
    if (!isDigit(text[index]))
      return false;

    unsigned long digit = static_cast<unsigned long>(text[index] - '0');
    if (parsedValue > (static_cast<unsigned long>(INT_MAX) - digit) / 10UL)
      return false;

    parsedValue = (parsedValue * 10UL) + digit;
  }

  value = static_cast<int>(parsedValue);
  return true;
}

void SwitchApiServer::sendJson(int statusCode, const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(statusCode, "application/json; charset=utf-8", body);
  SerialLogger::infof(
    "HTTP",
    "Response; status=%d, bytes=%u",
    statusCode,
    static_cast<unsigned int>(body.length()));
}
