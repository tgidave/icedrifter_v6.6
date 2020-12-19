#ifndef _GPS_H
#define _GPS_H

#include "icedrifter.h"

#ifdef SERIAL_DEBUG_GPS
  #define OUTBUFFER_SIZE  80
#endif

#define GPS_SERIAL Serial1
#define GPS_POWER_PIN 3
#define GPS_BAUD 9600


typedef enum ft {
  FIX_FULL,
  FIX_TIME
} fixType;

int gpsGetFix(fixType typeFix, icedrifterData* idData);

#endif



