//
//
//

#include "ReadDigitalValue.h"

ReadDigitalValue::ReadDigitalValue()
{
  pinMode(this->p1, INPUT);
}

void ReadDigitalValue::Action(WiFiClient client, String value)
{
  if (value.indexOf("/ReadDigitalValue/") == -1)
    return;
    
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:application/json");
  client.println();
  client.println("{\"pin\":" + String(this->p1) + ",\"value\":" + String(digitalRead(this->p1)) + "}");
}
