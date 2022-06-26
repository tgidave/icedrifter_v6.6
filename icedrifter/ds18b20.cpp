#include <OneWire.h>
#include <DallasTemperature.h>

#include "icedrifter.h"
#include "ds18b20.h"

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(& oneWire);

float getRemoteTemp(icedrifterData* idData) { 

// pinMode(DS18B20_POWER_PIN, OUTPUT);
//  digitalWrite(DS18B20_POWER_PIN, HIGH);
//  delay(1000);

  //Start the Library.
  sensors.begin();

#ifdef SERIAL_DEBUG_DS18B20
  DEBUG_SERIAL.print(F("Requesting DS18B20 temperatures...\n"));
#endif

  sensors.requestTemperatures(); // Send the command to get temperature readings
  delay(1000);


  if ((idData->idRemoteTemp = sensors.getTempCByIndex(0)) == -127) {

#ifdef SERIAL_DEBUG_DS18B20
    DEBUG_SERIAL.print(F("Remote temperature is "));
    DEBUG_SERIAL.print(idData->idRemoteTemp);
    DEBUG_SERIAL.print(F(" C\n"));

    if (idData->idRemoteTemp == -127) {
      DEBUG_SERIAL.print(F("Error: Disconnected!!!\n"));
    }
#endif

    idData->idRemoteTemp = idData->idTemperature;
  }

#ifdef SERIAL_DEBUG_DS18B20
  DEBUG_SERIAL.print(F("Sending back "));
  DEBUG_SERIAL.print(idData->idRemoteTemp);
  DEBUG_SERIAL.print(F(" C\n"));
#endif

  return (idData->idRemoteTemp);
}
