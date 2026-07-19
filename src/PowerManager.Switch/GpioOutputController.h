#pragma once

#include "Arduino.h"
#include "SwitchConfiguration.h"

class GpioOutputController {
public:
  GpioOutputController();
  void begin();
  bool setDigitalValue(int pin, bool value);
  bool pulseDigitalValue(int pin, bool value, unsigned long durationMilliseconds);
  void update();

private:
  struct OutputState {
    bool currentValue;
    bool pulseActive;
    bool restoreValue;
    unsigned long pulseStartedAtMilliseconds;
    unsigned long pulseDurationMilliseconds;
  };

  OutputState* findOutputState(int pin);

  OutputState outputStates[SwitchConfiguration::GPIO_OUTPUT_PIN_COUNT] = {};
};
