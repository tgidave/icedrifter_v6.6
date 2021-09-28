#ifndef _GPS_H
#define _GPS_H

#include "icedrifter.h"

#ifdef SERIAL_DEBUG_GPS
  #define OUTBUFFER_SIZE  80
#endif

#define GPS_SERIAL Serial1
#define GPS_BAUD 9600

int gpsGetFix(icedrifterData* idData);

int gpsGetMinutes();

int gpsGetHour();

#endif



