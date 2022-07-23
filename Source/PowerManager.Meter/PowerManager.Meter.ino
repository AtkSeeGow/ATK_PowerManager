#include <ModbusMaster.h>

#define DE 19
#define RE 18

ModbusMaster node;

void preTransmission()
{
  digitalWrite(RE, 1);
  digitalWrite(DE, 1);
}

void postTransmission()
{
  digitalWrite(RE, 0);
  digitalWrite(DE, 0);
}

void setup()
{
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  
  postTransmission();

  Serial.begin(9600);
  Serial2.begin(9600);

  node.begin(2, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

void loop()
{
  uint8_t result = node.readHoldingRegisters(0x0bd3, 4);
  if (result == node.ku8MBSuccess)
  {
    Serial.println("SlaveValue:"); 
    Serial.println(node.getResponseBuffer(0x00));
    Serial.println(node.getResponseBuffer(0x01));
    Serial.println(node.getResponseBuffer(0x02));
    Serial.println(node.getResponseBuffer(0x03));
  }

  delay(5000);
}

