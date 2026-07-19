#pragma once

#include "Arduino.h"

class GpioReader {
public:
  GpioReader();
  void begin();
  bool read(int pin, int& value) const;
};
