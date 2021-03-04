//
//
//

#include "SwitchOpticalCoupler.h"

SwitchOpticalCoupler::SwitchOpticalCoupler()
{
  pinMode(this->p1, OUTPUT);
}

void SwitchOpticalCoupler::Action(WiFiClient client, String value)
{
  unsigned long now = millis();

  if (value.indexOf("/SwitchOpticalCoupler/") == -1)
    return;

  value.replace("GET /SwitchOpticalCoupler/", "");
  value.replace("HTTP/1.1", "");

  this->previousTime = now;
  this->interval = value.toInt();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:application/json");
  client.println();
  client.println("{\"previousTime\":" + String(now) + ",\"interval\":" + String(value) + "}");
}

void SwitchOpticalCoupler::Execution()
{
  unsigned long now = millis();
  if ((this->previousTime + this->interval) > now)
    digitalWrite(this->p1, HIGH);
  else
    digitalWrite(this->p1, LOW);
}
