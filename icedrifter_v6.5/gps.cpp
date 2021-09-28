
#include <time.h>
#include <TinyGPS++.h>  // NMEA parsing: http://arduiniana.org
#include <PString.h>    // String buffer formatting: http://arduiniana.org

#include "icedrifter.h"
#include "gps.h"

TinyGPSPlus tinygps;

char GPShexchars[] = "0123456789ABCDEF";

void GPSprintHexChar(uint8_t x) {
  Serial.print(GPShexchars[(x >> 4)]);
  Serial.print(GPShexchars[(x & 0x0f)]);
}

int gpsGetFix(icedrifterData * idData) {

  int fixfnd = false;
  unsigned long now;
  tm timeStru;
  struct tm *ptm = &timeStru;

#ifdef  SERIAL_DEBUG_GPS
  char outBuffer[OUTBUFFER_SIZE];
#endif

  GPS_SERIAL.begin(GPS_BAUD);

  // Step 1: Reset TinyGPS++ and begin listening to the GPS
#ifdef SERIAL_DEBUG_GPS
  DEBUG_SERIAL.println(F("Beginning GPS"));
#endif
  tinygps = TinyGPSPlus();

  // Step 2: Look for GPS signal for up to 7 minutes
  for (now = millis(); !fixfnd && ((millis() - now) < (7UL * 60UL * 1000UL));) {

    if (GPS_SERIAL.available()) {
      tinygps.encode(GPS_SERIAL.read());

      fixfnd = tinygps.location.isValid() && tinygps.date.isValid() &&
          tinygps.time.isValid() && tinygps.altitude.isValid();
    }

    if (fixfnd) {
      break;
    }
  }

  if (fixfnd) {
    timeStru.tm_year = tinygps.date.year() - 1900;
    timeStru.tm_mon = tinygps.date.month() - 1;
    timeStru.tm_mday = tinygps.date.day();
    timeStru.tm_hour = tinygps.time.hour();
    timeStru.tm_min = tinygps.time.minute();
    timeStru.tm_sec = tinygps.time.second();
    idData->idGPSTime = mktime(&timeStru);
    idData->idLatitude = tinygps.location.lat();
    idData->idLongitude = tinygps.location.lng();

#ifdef SERIAL_DEBUG_GPS
    *outBuffer = 0;
    PString str(outBuffer, OUTBUFFER_SIZE);
    str.print(F("fix found!\n"));
//    str.print(asctime(&timeStru));
    str.print(tinygps.date.year());
    str.print(F("/"));
    str.print(tinygps.date.month());
    str.print(F("/"));
    str.print(tinygps.date.day());
    str.print(F(" "));//
    str.print(tinygps.time.hour());
    str.print(F(":"));
    str.print(tinygps.time.minute());
    str.print(F(":"));
    str.print(tinygps.time.second());
    str.print(F(" "));
    str.print(tinygps.location.lat(), 6);
    str.print(F(","));
    str.print(tinygps.location.lng(), 6);
    str.print(F("\n"));

    DEBUG_SERIAL.print(outBuffer);
#endif

  }

#ifdef SERIAL_DEBUG_GPS
else {
    DEBUG_SERIAL.print(F("No fix found.\n"));
  }
#endif

  GPS_SERIAL.end();
//  digitalWrite(GPS_POWER_PIN, HIGH);
//   setSerialMuxOff();
  return (fixfnd);
}

int gpsGetMinutes() {
  return(tinygps.time.minute());
}

int gpsGetHour() {
  return(tinygps.time.hour());
}

