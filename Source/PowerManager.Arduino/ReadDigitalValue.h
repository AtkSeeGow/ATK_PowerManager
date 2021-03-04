// ReadDigitalValue.h

#ifndef _READDIGITALVALUE_h
#define _READDIGITALVALUE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "WiFi.h"

class ReadDigitalValue {
  public:
    ReadDigitalValue();
    void Action(WiFiClient client, String value);
  private:
    int p1 = 35;
};

#endif
