#include "GpioOutputController.h"

#include "SerialLogger.h"

GpioOutputController::GpioOutputController() {
}

void GpioOutputController::begin() {
  for (unsigned int index = 0;
       index < SwitchConfiguration::GPIO_OUTPUT_PIN_COUNT;
       index++) {
    int pin = SwitchConfiguration::GPIO_OUTPUT_PINS[index];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    outputStates[index] = {};
    SerialLogger::infof("GPIO", "Output initialized; pin=%d, value=0", pin);
  }
}

bool GpioOutputController::setDigitalValue(int pin, bool value) {
  OutputState* state = findOutputState(pin);
  if (state == nullptr) {
    SerialLogger::infof("GPIO", "Write rejected; unsupported pin=%d", pin);
    return false;
  }

  state->pulseActive = false;
  digitalWrite(pin, value ? HIGH : LOW);
  state->currentValue = value;
  SerialLogger::infof(
    "GPIO",
    "Write; pin=%d, value=%d",
    pin,
    value ? 1 : 0);
  return true;
}

bool GpioOutputController::pulseDigitalValue(
  int pin,
  bool value,
  unsigned long durationMilliseconds) {
  OutputState* state = findOutputState(pin);
  if (state == nullptr || durationMilliseconds == 0) {
    SerialLogger::infof(
      "GPIO",
      "Pulse rejected; pin=%d, duration_ms=%lu",
      pin,
      durationMilliseconds);
    return false;
  }

  state->restoreValue = state->pulseActive
    ? state->restoreValue
    : state->currentValue;
  digitalWrite(pin, value ? HIGH : LOW);
  state->currentValue = value;
  state->pulseStartedAtMilliseconds = millis();
  state->pulseDurationMilliseconds = durationMilliseconds;
  state->pulseActive = true;
  SerialLogger::infof(
    "GPIO",
    "Pulse started; pin=%d, value=%d, duration_ms=%lu, restore_value=%d",
    pin,
    value ? 1 : 0,
    durationMilliseconds,
    state->restoreValue ? 1 : 0);
  return true;
}

void GpioOutputController::update() {
  unsigned long currentTimeMilliseconds = millis();

  for (unsigned int index = 0;
       index < SwitchConfiguration::GPIO_OUTPUT_PIN_COUNT;
       index++) {
    OutputState& state = outputStates[index];
    if (!state.pulseActive)
      continue;

    if (currentTimeMilliseconds - state.pulseStartedAtMilliseconds <
        state.pulseDurationMilliseconds)
      continue;

    digitalWrite(
      SwitchConfiguration::GPIO_OUTPUT_PINS[index],
      state.restoreValue ? HIGH : LOW);
    state.currentValue = state.restoreValue;
    state.pulseActive = false;
    SerialLogger::infof(
      "GPIO",
      "Pulse completed; pin=%d, value=%d",
      SwitchConfiguration::GPIO_OUTPUT_PINS[index],
      state.currentValue ? 1 : 0);
  }
}

GpioOutputController::OutputState*
GpioOutputController::findOutputState(int pin) {
  for (unsigned int index = 0;
       index < SwitchConfiguration::GPIO_OUTPUT_PIN_COUNT;
       index++) {
    if (pin == SwitchConfiguration::GPIO_OUTPUT_PINS[index])
      return &outputStates[index];
  }

  return nullptr;
}
