

#include <Wire.h>
#include <MS5837.h>
#include "icedrifter.h"
#include "ms5837_02ba.h"


MS5837 sensor;  //I2C

void getMs5837Data(icedrifterData* idData) {

  Wire.begin();

  while (!sensor.init()) {
#ifdef SERIAL_DEBUG_MS5837
    DEBUG_SERIAL.println("Init failed!");
#endif
    idData->idTemperature = 0.0;
    idData->idPressure = 0.0;
    return;
  }

  sensor.setModel(MS5837::MS5837_02BA);
  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)

  sensor.read();

#ifdef SERIAL_DEBUG_MS5837
    DEBUG_SERIAL.println(F("Reading pressure."));
#endif

  idData->idPressure = sensor.pressure();
  
#ifdef SERIAL_DEBUG_MS5837
    DEBUG_SERIAL.println(F("Reading temperature."));
#endif

  idData->idTemperature = idData->idRemoteTemp = sensor.temperature(); 

#ifdef SERIAL_DEBUG_MS5837
    DEBUG_SERIAL.print(F("MS5837 temperature = "));
    DEBUG_SERIAL.print(idData->idTemperature);
    DEBUG_SERIAL.print(F(" C\npressure = "));
    DEBUG_SERIAL.print(idData->idPressure);
    DEBUG_SERIAL.print(F(" hPa\n"));
#endif

  Wire.end();
}

