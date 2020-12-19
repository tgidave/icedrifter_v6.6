
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
#include <Arduino.h>
#include "serialmux.h"

const uint8_t muxTable[5][3] = { 
// This table is set up the same as the function table in the SN74LV4052A 
// datasheet on page 12.  The columns are:
//{ INH,   B,   A  } which corrisponds to muxINHPort, muxBPort, muxAPort.
  { LOW, LOW, LOW },
  { LOW, LOW, HIGH },
  { LOW, HIGH, LOW },
  { LOW, HIGH, HIGH },
  ( HIGH, LOW, LOW ),
};

enum muxCmd {
  muxUnused,
  muxGPS,        
  muxRockBlock,  
  muxChain,
  muxOff,        
  muxCmdMax,      
};

enum muxIx {
  muxINH,
  muxB,
  muxA,
};

void setSerialMuxInit() {
  pinMode(muxINHPort, OUTPUT);
  pinMode(muxAPort, OUTPUT);
  pinMode(muxBPort, OUTPUT);
  digitalWrite(muxINHPort, HIGH);
  digitalWrite(muxAPort, LOW);
  digitalWrite(muxBPort, LOW);
}

void setSerialMux( int muxCommand ) {
  if (!((muxCommand >= 0) && (muxCommand < muxCmdMax))) {
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("Mux command out of range = "));
    DEBUG_SERIAL.print( muxCommand ); 
    DEBUG_SERIAL.print(F("\r\n"));
#endif
    return;
  }

  digitalWrite(muxINHPort, HIGH);
  digitalWrite(muxAPort, muxTable[muxCommand][muxA]);
  digitalWrite(muxBPort, muxTable[muxCommand][muxB]);
  digitalWrite(muxINHPort, muxTable[muxCommand][muxINH]);

}

void setSerialMuxOff(void) {
  setSerialMux(muxOff);
}

void setSerialMuxToGPS(void) {
  setSerialMux(muxGPS);
}

void setSerialMuxToRockBlock(void) {
  setSerialMux(muxRockBlock);
}

void setSerialMuxToChain(void) {
  setSerialMux(muxChain);
}
