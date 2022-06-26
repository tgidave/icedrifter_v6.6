
#include <time.h>
#include <TinyGPS++.h>  // NMEA parsing: http://arduiniana.org
#include <PString.h>    // String buffer formatting: http://arduiniana.org

#include "icedrifter.h"
#include "gps.h"

#define GET_FIX_COUNT_MAX  2
#define FIX_FND_COUNT_MAX  10

TinyGPSPlus tinygps;

char GPShexchars[] = "0123456789ABCDEF";

void GPSprintHexChar(uint8_t x) {
  Serial.print(GPShexchars[(x >> 4)]);
  Serial.print(GPShexchars[(x & 0x0f)]);
}

int gpsGetFix(icedrifterData *idData) {

  unsigned long tempTimeout;
  unsigned long now;
  int gpsFixCount;
  int fixfnd = false;
  int fixfndCount = 0;
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
  gpsFixCount = 0;

  while (1) {
    for (now = millis(); !fixfnd && ((tempTimeout = (millis() - now)) < (7UL * 60UL * 1000UL));) {
      if (GPS_SERIAL.available()) {
        tinygps.encode(GPS_SERIAL.read());

        fixfnd = tinygps.location.isValid() && tinygps.location.isUpdated() &&
            tinygps.date.isValid() && tinygps.date.isUpdated() &&
            tinygps.time.isValid() && tinygps.time.isUpdated() &&
            tinygps.altitude.isValid() && tinygps.altitude.isUpdated();

        if (fixfnd) {
          fixfndCount++;
#ifdef SERIAL_DEBUG_GPS
          DEBUG_SERIAL.print(F("Got fixfnd and fixfndCount = "));
          DEBUG_SERIAL.print(fixfndCount);
          DEBUG_SERIAL.print(F("\n"));
#endif
        }

#ifdef SERIAL_DEBUG_GPS
        else if (fixfndCount > 0) {
          DEBUG_SERIAL.print(F("No fix found and fixfndCount = "));
          DEBUG_SERIAL.print(fixfndCount);
          DEBUG_SERIAL.print(F("\n"));
        }
#endif
      }

      if (fixfnd) {
        break;
      }
    }

    if (++gpsFixCount >= GET_FIX_COUNT_MAX) {
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
    idData->idGPSTime = mk_gmtime(&timeStru);
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
    str.print(F(" ")); //
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
  return (tinygps.time.minute());
}

int gpsGetHour() {
  return (tinygps.time.hour());
}

