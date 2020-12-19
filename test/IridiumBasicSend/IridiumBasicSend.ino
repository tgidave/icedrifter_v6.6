#include <IridiumSBD.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
//#include "serialmux.h"

#define CONSOLE_BAUD 115200

//#define BODTEST

SoftwareSerial isbdss(12, 13);
IridiumSBD isbd(isbdss);

static const int ROCKBLOCK_POWER = 15;
static const int OTHER_POWER = 14;

void ISBDConsoleCallback(IridiumSBD *device, char c) {
  Serial.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c) {
  Serial.write(c);
}

void setup() {

#ifdef BODTEST
  Serial.begin(CONSOLE_BAUD);
  
  Serial.print("Starting rockblock test\r\n"); 
  Serial.flush();

  delay(1000);
#endif

  pinMode(OTHER_POWER, OUTPUT);
  digitalWrite(OTHER_POWER, LOW);

  pinMode(ROCKBLOCK_POWER, OUTPUT);
//  digitalWrite(ROCKBLOCK_POWER, LOW);
//  delay(5000);

  digitalWrite(ROCKBLOCK_POWER, HIGH);
  delay(1000);

//  serialMuxInit();
//  serialMuxSetToRockBlock();
  isbdss.begin(19200);

#ifndef BODTEST
  Serial.begin(CONSOLE_BAUD);
  
  Serial.print("Starting rockblock test\r\n"); 
  Serial.flush();

  delay(1000);
#endif
}

void loop() {
/*
  Serial1.println("Hello World.");
  delay(1000);
}
*/
  int rc;
  int signalQuality = -1;

//  isbd.attachConsole(Serial);
  isbd.setPowerProfile(isbd.DEFAULT_POWER_PROFILE);

  Serial.print("isbd.begin()\r\n"); 
  delay(1000);

  if ((rc = isbd.begin()) != 0) {
    Serial.print("begin failed: error ");
    Serial.println(rc);
    return;
  }

  Serial.print("Getting signal quality\r\n"); 
  delay(1000);

  int err = isbd.getSignalQuality(signalQuality);
  if (err != 0) {
    Serial.print("SignalQuality failed: error ");
    Serial.println(err);
    return;
  }

  Serial.print("Signal quality is ");
  Serial.println(signalQuality);

  err = isbd.sendSBDText("Hello, world!");
  if (err != 0) {
    Serial.print("sendSBDText failed: error ");
    Serial.println(err);
    return;
  }

  Serial.println("Hey, it worked!");
  Serial.print("Messages left: ");
  Serial.println(isbd.getWaitingMessageCount());
  Serial.print("Test done.\n");

  digitalWrite(ROCKBLOCK_POWER, LOW);
//  serialMuxSetToOff(); 

  while (1) {
    delay(1000);
  }

}

