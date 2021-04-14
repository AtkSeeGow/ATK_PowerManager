#include "WiFi.h"
#include "SwitchOpticalCoupler.h"
#include "ReadDigitalValue.h"

#include <WebServer.h>

WiFiServer server(80);

SwitchOpticalCoupler switchOpticalCoupler = SwitchOpticalCoupler();
ReadDigitalValue readDigitalValue = ReadDigitalValue();

void setup() {
  Serial.begin(9600);
  Serial.println("Initialize...");

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("ESP32-PowerManager");

  int status = WL_IDLE_STATUS ;
  while (status != WL_CONNECTED) {
    Serial.println("Connecting...");
    status = WiFi.begin("TP-Link A608", "**********");
    delay(5000);
  }

  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    if (client.connected()) {
      String value = client.readStringUntil('\r');
      switchOpticalCoupler.Action(client, value);
      readDigitalValue.Action(client, value);
    }
    client.stop();
  }

  switchOpticalCoupler.Execution();
  
  if(millis() > 86400000)
    ESP.restart();
}
