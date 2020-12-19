/*!                                                                              
 *  @file serialmux.h                                               
 *                                                                               
 *  @mainpage Code to handle the serial multiplexer
 *                                                                                 
 *  @section intro_sec Introduction                                              
 *  
 *  This code implements functionality that controls the serial
 *  multiplexer which is used to allow a single uart to communicate
 *  with multiple devices.  In this case the devices are the GPS,
 *  the RockBlock, and the temperature and light sensors.
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

#ifndef _SERIAL_MUX_H
#define _SERIAL_MUX_H

const enum muxCmd {
  muxOff,        
  muxGPS,        
  muxRockBlock,  
  muxChain,
  muxCmdMax,      
};

const enum muxIx {
  muxINH,
  muxA,
  muxB,
};

const uint8_t muxTable[4][3] = {
  { HIGH, LOW, LOW },
  { LOW, LOW, HIGH },
  { LOW, HIGH, LOW },
  { LOW, HIGH, HIGH },
};
  
#define muxINHPort  19
#define muxAPort    13
#define muxBPort    12
  
void SetSerialMuxOff(void);
void SetSerialMuxToGPS(void);
void SetSerialMuxToRockBlock(void);
void SetSerialMuxToChain(void);

#endif // _SERIAL_MUX_H


