
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

#ifndef _ICEDRIFTER_H
#define _ICEDRIFTER_H

#ifdef ARDUINO
  #include <TimeLib.h>
  #include <Time.h>
#endif // ARDUINO

// ****************************************************************************
// ***                                                                      ***
// *** These defines are for debugging only and should be commented out for ***
// *** normal processing.                                                   ***
// ***                                                                      ***
// ****************************************************************************
//
// The TEST_ALL switch will collect and send data at bootup
// and then every hour on the half hour after that.  Comment out
// the next line to run normally.

#define TEST_ALL  // test as much code a possible at bootup.

// The NEVER_TRANSMIT switch does all processing determined by
// other switches including turning on and initializing the
// Rockblock hardware and software but the rockblock send function
// is never called

//#define NEVER_TRANSMIT  // Do everything except transmit data.

//To turn off the debugging messages, comment out the next line.

#define SERIAL_DEBUG

//The following defines are used to control what data is transmitted during debugging.
//If "SERIAL_DEBUG" is not defined they have no effect.

#ifdef SERIAL_DEBUG
#define DEBUG_SERIAL Serial
#define DEBUG_BAUD 115200

#define SERIAL_DEBUG_GPS
#define SERIAL_DEBUG_BMP280
#define SERIAL_DEBUG_DS18B20
#define SERIAL_DEBUG_CHAIN
#define SERIAL_DEBUG_ROCKBLOCK
#endif // SERIAL_DEBUG

// ****************************************************************************
// *** End debugging defines                                                ***
// ****************************************************************************

// If the next define is uncommented, the device will try to transmit data
// when the device if first powered up.  This can be usefull for making sure
// the device is working properly in the field before leaving the area.

#define TRANSMIT_AT_BOOT

// The next define controls whether or not data from the remote temperature sensor
// are collected and reported.  If the remote temperature sensor is not
// present, comment out the next line.

#define PROCESS_REMOTE_TEMP

#ifdef ARDUINO

// The next define controls whether or not data from the temperature and light
// sensors are collected and reported.  If the temperature and light
// chain sensor is not present, comment out the next line.
//#define PROCESS_CHAIN_DATA

//#define DROP_CHAIN_RX_TX

// These defines are used to determine how many sensors are on the temperature and
// light chain.  They are only used if PROCESS_CHAIN_DATA is defined so you do not
// need to change them if no chain hardware is attached.
#define TEMP_SENSOR_COUNT   160
#define LIGHT_SENSOR_COUNT  64

// Minutes to wait for data during chain reads.
#define TEMP_CHAIN_TIMEOUT_MINUTES 3UL
#define LIGHT_CHAIN_TIMEOUT_MINUTES 3UL

#else // ARDUINO

// ****************************************************************************************
// These defines are only used during the compilation of the decoder program.  They are
// set to the maximum values defined by the data chain specification.  The decoder program
// will only display the amount of data sent from the icedrifter even though these values
// may be greater than the actual hardware attached, or if no chain hardware is attached.
// *****************************************************************************************
// ***                                                                                   ***   
// *** IF YOU CHANGE ANY OF THESE VALUES YOU WILL NEED TO RECOMPILE THE DECODER PROGRAM. ***
// ***                                                                                   ***
// *****************************************************************************************
#define PROCESS_CHAIN_DATA
#define TEMP_SENSOR_COUNT   160
#define LIGHT_SENSOR_COUNT  64
#define MAX_RECORD_LENGTH   340
// *****************************************************************************************
#endif // ARDUINO

// Chain retries disabled.
#define MAX_CHAIN_RETRIES 0

#define LIGHT_SENSOR_FIELDS 4
#define TEMP_DATA_SIZE (TEMP_SENSOR_COUNT * sizeof(uint16_t))
#define LIGHT_DATA_SIZE ((LIGHT_SENSOR_COUNT * LIGHT_SENSOR_FIELDS) * sizeof(uint16_t))

// Chain data definition.
typedef struct chainData {
  uint16_t cdTempData[TEMP_SENSOR_COUNT];
  uint16_t cdLightData[LIGHT_SENSOR_COUNT][LIGHT_SENSOR_FIELDS];
} chainData;

//icedrifter data record definition.
typedef struct icedrifterData {
  uint8_t idSwitches;

#define PROCESS_REMOTE_TEMP_SWITCH  0x01
#define PROCESS_CHAIN_DATA_SWITCH   0x02
  
  uint8_t idcdError; 

#define TEMP_CHAIN_TIMEOUT_ERROR  0x01
#define TEMP_CHAIN_OVERRUN_ERROR  0x02
#define LIGHT_CHAIN_TIMEOUT_ERROR 0x04
#define LIGHT_CHAIN_OVERRUN_ERROR 0x08

  uint16_t idTempByteCount; 
  uint16_t idLightByteCount;

  uint16_t idSpare;

#ifdef ARDUINO
  time_t idLastBootTime;
  time_t idGPSTime;
#else
  uint32_t idLastBootTime;
  uint32_t idGPSTime; 
#endif // ARDUINO

  float idLatitude;
  float idLongitude;
  float idTemperature;
  float idPressure;
  float idRemoteTemp;

  #define BASE_RECORD_LENGTH  36

#ifdef PROCESS_CHAIN_DATA
  chainData idChainData;
#endif // PROCESS_CHAIN_DATA

void printHexChar(uint8_t x);

} icedrifterData;

#endif // _ICEDRIFTER_H
