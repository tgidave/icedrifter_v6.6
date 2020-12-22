/*!                                                                              
 *  @file icedrifter.ino                                                  
 *                                                                               
 *  @mainpage Code to implement the Icedrifter buoy.                     
 *                                                                               
 *  @section intro_sec Introduction                                              
 *  
 *  This code implements functionality that gathers GPS location data,
 *  temperature and air pressure data, and optionally, a remote
 *  temperature probe and light and temperature chain data.  This data
 *  is then sent back to the user using the Iridium system on a daily
 *  or hourly basis.
 *                                                                               
 *  @section author Author                                                       
 *                                                                               
 *  Uncle Dave                                                  
 *                                                                               
 *  @section license License                                                     
 *                                                                               
 *  Unknown (Talk to Cy)                                                        
 *                                                                               
 *  @section HISTORY                                                             
 *                                                                               
 *  v1.0 - First release                                                         
 */

#include <LowPower.h>
//#include <TimeLib.h>
#include <time.h>

//#include <arduino.h>
#include <avr/wdt.h>

#include <TinyGPS++.h> // NMEA parsing: http://arduiniana.org
#include <PString.h> // String buffer formatting: http://arduiniana.org

#include "icedrifter.h"
#include "gps.h"
#include "bmp280.h"
#include "ds18b20.h"
//#include "chain.h"
#include "rockblock.h"
//#include "serialmux.h"

#define CONSOLE_BAUD 115200

// This table is used to determine when to report data through the
// Iridium system.  It is set up as UTC times from midnight to 23:00
// hours.  If an hour is set to false, the system will not report.
// If an hour is set to true, a report will be attempted on the next
// half hour.  I.E. if Noon UTC is set to true, the system will try
// to report at 12:30 UTC.  If all hour entries are set to true the
// system will try to report every hour on the half hour.  If all hour
// entries are set to false, the system will never try to report.
//
// This table is set for standard time and does not account for local
// daylight savings time.

const bool timeToReport[24] = {
  false,  // Midnight UTC
  true,   // 01:00 UTC - 6PM Thistle Grove time.
  false,  // 02:00 UTC
  false,  // 03:00 UTC
  false,  // 04:00 UTC
  false,  // 05:00 UTC
  false,  // 06:00 UTC
  false,  // 07:00 UTC - Midnight Mountain standard time
  false,   // 08:00 UTC
  false,  // 09:00 UTC
  false,  // 10:00 UTC
  false,  // 11:00 UTC
  false,  // Noon UTC
  true,   // 13:00 UTC - 6AM Thistle Grove time.
  false,  // 14:00 UTC
  false,  // 15:00 UTC
  false,  // 16:00 UTC
  false,  // 17:00 UTC
  false,  // 18:00 UTC
  false,  // 19:00 UTC - Noon Mountain standard time
  false,  // 20:00 UTC 
  false,  // 21:00 UTC
  false,  // 22:00 UTC
  false,  // 23:00 UTC
};

//enum period_t { /// Values for setting the watchdog timer.
//  SLEEP_15MS,
//  SLEEP_30MS,
//  SLEEP_60MS,
//  SLEEP_120MS,
//  SLEEP_250MS,
//  SLEEP_500MS,
//  SLEEP_1S,
//  SLEEP_2S,
//  SLEEP_4S,
//  SLEEP_8S,
//  SLEEP_FOREVER
//};

bool firstTime;   /// Set true in the setup function and set false after the first
                  /// time through the loop function.  Used to indicate when to
/// capture the last boot date and time.

bool gotFullFix;  /// Indicates that a Full fix was received.

int noFixFoundCount;  /// Number of times the GPS device could not get a fix.

int fixFound; /// indicates weather the last call to the GPS system returned a fix.

int totalDataLength;

icedrifterData idData;  /// Structure for accumulating and sending sensor data,

time_t lbTime;  /// Time and date of the last boot.

const char hexchars[] = "0123456789ABCDEF";

void printHexChar(uint8_t x) {
  Serial.print(hexchars[(x >> 4)]);
  Serial.print(hexchars[(x & 0x0f)]);
}

/// powerDown - Put processor into low power mode.
///
/// This function first set up the watchdog timer to go of after
/// the maxiuum interval
/// of 8 seconds and then puts the processor into low power sleep node.  After
/// approximately 8 seconds the interval time will expire and wake up the processor
/// and the program continues.
///
/// \param void
/// \return void

//void powerDown(void) {
//  ADCSRA &= ~(1 << ADEN);
//  wdt_enable(SLEEP_8S);
//  WDTCSR |= (1 << WDIE);
//  sleepMode(SLEEP_POWER_SAVE);
//  sleep();
//  noSleep();
//}

/// <summary>
/// ISR - Interrupt Service Routine for handeling the watchdog
/// timer interupt.  This routine disables the WDT interupt and
/// then returns. </summary>
/// <param> WDT_vect </param>
/// <returns>"Nothing"</returns>

//ISR(WDT_vect) {
  /// WDIE & WDIF is cleared in hardware upon entering this ISR
//  wdt_disable();
//}

/// <summary>
/// Accumulate and send data. This function captures the sender
/// data and sends that data to the user.
/// </summary>
/// <param name="void"></param>
/// <returns>"void"</returns>

void accumulateandsendData(void) {

  int i, j;

  int chainRetryCount;
  int recCount;
  uint8_t* wkPtr;


  totalDataLength = BASE_RECORD_LENGTH;
  idData.idSwitches = idData.idTempByteCount = idData.idLightByteCount = idData.idcdError = 0;

#ifdef PROCESS_REMOTE_TEMP_SWITCH
  idData.idSwitches |= PROCESS_REMOTE_TEMP_SWITCH;
#endif // PROCESS_REMOTE_TEMP_SWITCH

#ifdef PROCESS_CHAIN_DATA
  idData.idSwitches |= PROCESS_CHAIN_DATA_SWITCH;
  idData.idTempByteCount = idData.idLightByteCount = 0;
#endif // PROCESS_CHAIN_DATA

  idData.idLastBootTime = lbTime;

  digitalWrite(BMP280_DS18B20_GPS_POWER_PIN, HIGH);
  delay(1000);

  if ((fixFound = gpsGetFix(FIX_FULL, &idData)) == false) {
    idData.idGPSTime = 0;
    idData.idLatitude = 0;
    idData.idLongitude = 0;
  }

  getBMP280Data(&idData);

#ifdef PROCESS_REMOTE_TEMP
  getRemoteTemp(&idData);
#else
  idData.idRemoteTemp = 0;
#endif // PROCESS_REMOTE_TEMP

// Turn off the power to the BMP280, DS18B20, and GPS.
  digitalWrite(BMP280_DS18B20_GPS_POWER_PIN, LOW);

//#ifdef PROCESS_CHAIN_DATA
//  processChainData(&idData);
//  totalDataLength += (idData.idTempByteCount + idData.idLightByteCount);
//#endif  // PROCESS_CHAIN_DATA

  wkPtr = (uint8_t*)&idData;

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Dumping data record\n"));
  DEBUG_SERIAL.print(F("Address = "));
  DEBUG_SERIAL.print((uint32_t)wkPtr, HEX);
  DEBUG_SERIAL.print(F(" size = "));
  DEBUG_SERIAL.print(totalDataLength);
  DEBUG_SERIAL.print(F("\n"));

  for (i = 0; i < totalDataLength; i++) {
    printHexChar((uint8_t)*wkPtr);
    ++wkPtr;
  }

  DEBUG_SERIAL.print(F("\n"));
#endif // SERIAL_DEBUG

#ifdef HUMAN_READABLE_DISPLAY
  rbTransmitIcedrifterData(&idData, 0);
#else
  rbTransmitIcedrifterData(&idData, totalDataLength);
#endif // HUMAN_READABLE_DISPLAY
}

//! setup - This is an arduino defined routine that is called only once after the processor is booted.
//!
void setup() {

//  pinMode(muxINHPort, OUTPUT);
//  pinMode(muxAPort, OUTPUT);
//  pinMode(muxBPort, OUTPUT);

//  setSerialMuxInit();

  pinMode(BMP280_DS18B20_GPS_POWER_PIN, OUTPUT);
  digitalWrite(BMP280_DS18B20_GPS_POWER_PIN, LOW);

//  pinMode(GPS_POWER_PIN, OUTPUT);
//  digitalWrite(GPS_POWER_PIN, HIGH);

//  pinMode(BMP280_POWER_PIN, OUTPUT);
//  digitalWrite(BMP280_POWER_PIN, LOW);

  pinMode(ROCKBLOCK_POWER_PIN, OUTPUT);
  digitalWrite(ROCKBLOCK_POWER_PIN, LOW);

//#ifdef PROCESS_REMOTR_TEMP
//  pinMode(DS18B20_POWER_PIN, OUTPUT);
//  digitalWrite(DS18B20_POWER_PIN, LOW);
//#endif // PROCESS_REMOTR_TEMP

//#ifdef PROCESS_CHAIN_DATA
//  pinMode(CHAIN_POWER_PIN, OUTPUT);
//  digitalWrite(CHAIN_POWER_PIN, LOW);
//#endif // PROCESS_CHAIN_DATA

#ifdef SERIAL_DEBUG
  //! Start the serial ports
  DEBUG_SERIAL.begin(CONSOLE_BAUD);
  
  delay(5000);  // Wait for the serial port to connect.

  DEBUG_SERIAL.print(F("Icedrifter\nHardware version "));
  DEBUG_SERIAL.print(HARDWARE_VERSION);
  DEBUG_SERIAL.print(F("\nSoftware version "));
  DEBUG_SERIAL.print(SOFTWARE_VERSION);
  DEBUG_SERIAL.print(F("\n"));
  DEBUG_SERIAL.flush(); // Make sure the above message is displayed before continuing.
#endif // SERIAL_DEBUG

  gotFullFix = false; //! Clear the GPS full fix switch so the first call to the loop function requests a full fix.
  firstTime = true;

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Setup done\n")); //! Let the user know we are done with the setup function.
#endif // SERIAL_DEBUG
}

//! loop - This is the main processing function for the arduino system.
//!
//! The first time through this function a full GPS fix is requested.  If no fix is
//! received the processor is put to sleep for 60 minutes and then a full GPS fix
//! will be requested again.  This continues until a full fix is received.
//!
//! Upon receiving a full GPS fix. the minutes are calculated to wake up the processor
//! at the next half hour and the processor is put to sleep.
//!
//! Once a full GPS fix is received, only the current time is requested from the GPS.
//! That's all that is needed to calculate the minutes to the next wake up time.
void loop() {

  int sleepSecs;  //! Number of seconds to sleep before the processor is woken up.
  int sleepMins;  //! Number of minutes to sleep before the processor is woken up.

  noFixFoundCount = 0;  //! clear the no fix found count.

  digitalWrite(BMP280_DS18B20_GPS_POWER_PIN, HIGH);
  delay(1000);

  //! Check to see if a full fix was received.  If not, try to get a full fix.
  //! If so, just get a time fix.
  if (gotFullFix) {
    fixFound = gpsGetFix(FIX_TIME, &idData);
  } else {
    fixFound = gpsGetFix(FIX_FULL, &idData);
  }

  //! If a GPS fix was received, set the gotFullFix switch and clear the noFixFound count.
  //! Otherwise add one to the noFixFoundCount.
  if (fixFound) {
    gotFullFix = true;
    noFixFoundCount = 0;
    if (firstTime) {
      lbTime = idData.idLastBootTime = idData.idGPSTime;
    }
  } else {
    ++noFixFoundCount;
  }

#ifdef TEST_ALL
  accumulateandsendData();
#elif defined(TRANSMIT_AT_BOOT)
  if (firstTime ||
      ((fixFound && timeToReport[gpsGetHour()] == true) ||
       noFixFoundCount >= 24)) {
    noFixFoundCount = 0;
    accumulateandsendData();
  }
#else
  if (!firstTime &&
      ((fixFound && timeToReport[gpsGetHour()] == true) ||
       noFixFoundCount >= 24)) {
    noFixFoundCount = 0;
    accumulateandsendData();
  }
#endif // TEST_ALL

  digitalWrite(BMP280_DS18B20_GPS_POWER_PIN, HIGH);
  delay(1000);

  // Accumulating and sending the data can take a while so update the time again.
  fixFound = gpsGetFix(FIX_FULL, &idData);
  firstTime = false;

  digitalWrite(BMP280_DS18B20_GPS_POWER_PIN, LOW);
  
  //! If a GPS fix was found
  if (fixFound) {
    //! Calculate the minutes until the next half hour,
    sleepMins = 90 - gpsGetMinutes();
    //! If it less than 15 minutes until the nex half hour,
    if (sleepMins >= 75) {
      sleepMins -= 60;
    }

#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("Fix found - sleep "));
    DEBUG_SERIAL.print(sleepMins);
    DEBUG_SERIAL.print(F(" minutes\n"));
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.end();
#endif // SERIAL_DEBUG
    sleepSecs = sleepMins * 60;
  } else {
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("Fix not found - sleep 60 minutes\n"));
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.end();
#endif // SERIAL_DEBUG
    sleepSecs = 3600;
  }

  do {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    sleepSecs -= 8;
  } while (sleepSecs > 0);

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.begin(CONSOLE_BAUD);
  DEBUG_SERIAL.print(F("woke up!\n"));
  DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG
}

