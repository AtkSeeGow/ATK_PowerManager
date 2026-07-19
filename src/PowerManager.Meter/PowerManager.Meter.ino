#include "MeterConfig.h"
#include "MeterService.h"

MeterService meterService(
  MeterConfig::Wifi::SSID,
  MeterConfig::Wifi::PASSWORD,
  MeterConfig::Wifi::HOSTNAME,
  Serial2,
  MeterConfig::Pm5350::SLAVE_ID,
  MeterConfig::Pm5350::RX_PIN,
  MeterConfig::Pm5350::TX_PIN,
  MeterConfig::Pm5350::DE_PIN,
  MeterConfig::Pm5350::RE_PIN,
  MeterConfig::Am2320::SDA_PIN,
  MeterConfig::Am2320::SCL_PIN);

void setup()
{
  const unsigned long setupStartedAtMs = millis();
  meterService.begin(setupStartedAtMs);
}

void loop()
{
  meterService.loop();
}
