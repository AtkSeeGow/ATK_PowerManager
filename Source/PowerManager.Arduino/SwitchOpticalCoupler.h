// SwitchOpticalCoupler.h

#ifndef _SWITCHOPTICALCOUPLER_h
#define _SWITCHOPTICALCOUPLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "WiFi.h"

class SwitchOpticalCoupler {
  public:
    SwitchOpticalCoupler();
    void Action(WiFiClient client, String value);
    void Execution();
  private:
    int p1 = 25;
    unsigned long previousTime = 0;
    unsigned long interval = 0;
};

#endif
