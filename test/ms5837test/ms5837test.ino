//-----------------------------------------------------------------------------
// Code to test the MS5837_02BA hardware of the icedrifter version 6.6  board.
//-----------------------------------------------------------------------------

#include <Wire.h>
#include "MS5837.h"

#define MS5837_DS18B20_GPS_POWER_PIN 14

MS5837 sensor;

void setup() {
  
  Serial.begin(115200);
  
  Serial.println("Starting");
  
  pinMode(MS5837_DS18B20_GPS_POWER_PIN, OUTPUT);
  digitalWrite(MS5837_DS18B20_GPS_POWER_PIN, HIGH);
  delay(1000);

  Serial.println("Powered on");

  Wire.begin();

  Serial.println("Wire.begin done");

  // Initialize pressure sensor
  // Returns true if initialization was successful
  // We can't continue with the rest of the program unless we can initialize the sensor
  while (!sensor.init()) {
    Serial.println("Init failed!");
    delay(5000);
  }
  
  Serial.println("sensor.init done");
  
  sensor.setModel(MS5837::MS5837_02BA);
  sensor.setFluidDensity(1029); // kg/m^3 (997 for freshwater, 1029 for seawater)
}

void loop() {
  // Update pressure and temperature readings
  sensor.read();

  Serial.print("Pressure: "); 
  Serial.print(sensor.pressure()); 
  Serial.println(" mbar");
  
  Serial.print("Temperature: "); 
  Serial.print(sensor.temperature()); 
  Serial.println(" deg C");
  
  Serial.print("Depth: "); 
  Serial.print(sensor.depth()); 
  Serial.println(" m");
  
  Serial.print("Altitude: "); 
  Serial.print(sensor.altitude()); 
  Serial.println(" m above mean sea level");

  delay(1000);
}