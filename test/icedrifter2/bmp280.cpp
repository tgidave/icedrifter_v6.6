
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "icedrifter.h"
#include "bmp280.h"

Adafruit_BMP280 bmp; // I2C

void getBMP280Data(icedrifterData* idData) {

  bool deviceReady; 

#ifdef SERIAL_DEBUG_BMP280
  DEBUG_SERIAL.println(F("Powering on BMP280."));
#endif

  digitalWrite(BMP280_POWER_PIN, HIGH);
  delay(1000);

  deviceReady = bmp.begin(0x76);
  delay(1000);

#ifdef SERIAL_DEBUG_BMP280
  if (deviceReady) {
    DEBUG_SERIAL.print(F("BMP280 ready.\r\n"));
  } else {
    DEBUG_SERIAL.print(F("BMP280 did not become ready!!!\r\n"));
  }
#endif

  if (deviceReady) {

#ifdef SERIAL_DEBUG_BMP280
    DEBUG_SERIAL.println(F("Reading temperature."));
#endif
    idData->idTemperature = bmp.readTemperature();

#ifdef SERIAL_DEBUG_BMP280
    DEBUG_SERIAL.println(F("Reading pressure."));
#endif
    idData->idPressure = bmp.readPressure();

#ifdef SERIAL_DEBUG_BMP280
    DEBUG_SERIAL.print(F("BMP280 temperature = "));
    DEBUG_SERIAL.print(idData->idTemperature);
    DEBUG_SERIAL.print(F(" pressure = "));
    DEBUG_SERIAL.print(idData->idPressure);
    DEBUG_SERIAL.print(F(" Pa\r\n"));
#endif

  } else {

#ifdef SERIAL_DEBUG_BMP280
    DEBUG_SERIAL.print(F("BMP280 did not become ready!!!\r\n"));
#endif

    idData->idTemperature = 0;
    idData->idPressure = 0;
  }

  Wire.end();
  digitalWrite(BMP280_POWER_PIN, LOW);
}

