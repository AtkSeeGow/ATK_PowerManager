#include "GpioReader.h"
#include "SerialLogger.h"
#include "SwitchConfiguration.h"

GpioReader::GpioReader() {
}

void GpioReader::begin() {
  for (unsigned int index = 0;
       index < SwitchConfiguration::GPIO_INPUT_PIN_COUNT;
       index++) {
    pinMode(SwitchConfiguration::GPIO_INPUT_PINS[index], INPUT);
    SerialLogger::infof(
      "GPIO",
      "Input initialized; pin=%d",
      SwitchConfiguration::GPIO_INPUT_PINS[index]);
  }
}

bool GpioReader::read(int pin, int& value) const {
  bool supported = false;

  for (unsigned int index = 0;
       index < SwitchConfiguration::GPIO_INPUT_PIN_COUNT;
       index++) {
    if (pin == SwitchConfiguration::GPIO_INPUT_PINS[index]) {
      supported = true;
      break;
    }
  }

  for (unsigned int index = 0;
       !supported && index < SwitchConfiguration::GPIO_OUTPUT_PIN_COUNT;
       index++) {
    if (pin == SwitchConfiguration::GPIO_OUTPUT_PINS[index])
      supported = true;
  }

  if (!supported) {
    SerialLogger::infof("GPIO", "Read rejected; unsupported pin=%d", pin);
    return false;
  }

  value = digitalRead(pin);
  SerialLogger::infof("GPIO", "Read; pin=%d, value=%d", pin, value);
  return true;
}
