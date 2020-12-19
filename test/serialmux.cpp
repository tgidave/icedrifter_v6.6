
/*!                                                                              
 *  @file serialmux.cpp                                                  
 *                                                                               
 *  @mainpage Code to switch the serial mux between devices.                     
 *                                                                               
 *  @section intro_sec Introduction                                              
 *  
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

#include <Wire.h>

#include "icedrifter.h"
#include "serialmux.h"

void SetSerialMux( int muxCommand ) {
  if (!((muxCommand >= 0) && (muxCommand < muxCmdMax))) {
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("Mux command out of range = "));
    DEBUG_SERIAL.print( muxCommand ); 
    DEBUG_SERIAL.print(F("\n"));
#endif
    return;
  }

  digitalWrite(muxINHPort, HIGH);
  digitalWrite(muxAPort, muxTable[muxCommand][muxA]);
  digitalWrite(muxBPort, muxTable[muxCommand][muxB]);
  digitalWrite(muxINHPort, muxTable[muxCommand][muxINH]);

}

void SetSerialMuxOff(void) {
  SetSerialMux(muxOff);
}

void SetSerialMuxToGPS(void) {
  SetSerialMux(muxGPS);
}

void SetSerialMuxToRockblock(void) {
  SetSerialMux(muxRockblock);
}

void SetSerialMuxToChain(void) {
  SetSerialMux(muxChain);
}
