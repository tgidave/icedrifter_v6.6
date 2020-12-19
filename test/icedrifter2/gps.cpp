#include <TimeLib.h>
#include <Time.h>
#include <TinyGPS++.h>  // NMEA parsing: http://arduiniana.org
#include <PString.h>    // String buffer formatting: http://arduiniana.org

#include "icedrifter.h"
#include "gps.h"
#include "serialmux.h"

TinyGPSPlus tinygps;

int gpsGetFix(fixType typeFix, icedrifterData * idData) {

  int fixfnd = false;
  unsigned long now;
//  TimeElements timeStru;
  tm timeStru;

#ifdef  SERIAL_DEBUG_GPS
  char outBuffer[OUTBUFFER_SIZE];
#endif

  setSerialMuxToGPS();

  digitalWrite(GPS_POWER_PIN, LOW);
  GPS_SERIAL.begin(GPS_BAUD);

  // Step 1: Reset TinyGPS++ and begin listening to the GPS
#ifdef SERIAL_DEBUG_GPS
  DEBUG_SERIAL.println(F("Beginning GPS"));
#endif
  tinygps = TinyGPSPlus();

  // Step 2: Look for GPS signal for up to 7 minutes
  for (now = millis(); !fixfnd && ((millis() - now) < (5UL * 60UL * 1000UL));) {

    if (GPS_SERIAL.available()) {
      tinygps.encode(GPS_SERIAL.read());

      if (typeFix == FIX_FULL) {
        fixfnd = tinygps.location.isValid() && tinygps.date.isValid() &&
            tinygps.time.isValid() && tinygps.altitude.isValid();

      } else {
        fixfnd = tinygps.date.isValid() && tinygps.time.isValid();
      }
    }
  }


  if (fixfnd) {
    timeStru.tm_year = tinygps.date.year() - 1970;
    timeStru.tm_mon = tinygps.date.month();
    timeStru.tm_mday = tinygps.date.day();
    timeStru.tm_hour = tinygps.time.hour();
    timeStru.tm_min = tinygps.time.minute();
    timeStru.tm_sec = tinygps.time.second();
    idData->idGPSTime = mktime(timeStru);

#ifdef SERIAL_DEBUG_GPS
    *outBuffer = 0;
    PString str(outBuffer, OUTBUFFER_SIZE);
    str.print(F("fix found!\n"));
    str.print(tinygps.date.year());
    str.print(F("/"));
    str.print(tinygps.date.month());
    str.print(F("/"));
    str.print(tinygps.date.day());
    str.print(F(" "));
    str.print(tinygps.time.hour());
    str.print(F(":"));
    str.print(tinygps.time.minute());
    str.print(F(":"));
    str.print(tinygps.time.second());
#endif // SERIAL_DEBUG_GPS

    if (typeFix == FIX_FULL) {
      idData->idLatitude = tinygps.location.lat();
      idData->idLongitude = tinygps.location.lng();

#ifdef SERIAL_DEBUG_GPS
      str.print(F(" "));
      str.print(tinygps.location.lat(), 6);
      str.print(F(","));
      str.print(tinygps.location.lng(), 6);
#endif // SERIAL_DEBUG_GPS

    }

#ifdef SERIAL_DEBUG_GPS
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
  digitalWrite(GPS_POWER_PIN, HIGH);
  setSerialMuxOff();
  return (fixfnd);
}
