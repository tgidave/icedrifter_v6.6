#include <Arduino.h>
#include <IridiumSBD.h>
#include <SoftwareSerial.h>

#include "icedrifter.h"
#include "rockblock.h"

SoftwareSerial isbdss(ROCKBLOCK_RX_PIN, ROCKBLOCK_TX_PIN);

IridiumSBD isbd(isbdss, ROCKBLOCK_SLEEP_PIN);

iceDrifterChunk idcChunk;

char rbhexchars[] = "0123456789ABCDEF";

void rbprintHexChar(uint8_t x) {
  Serial.print(rbhexchars[(x >> 4)]);
  Serial.print(rbhexchars[(x & 0x0f)]);
}

#ifdef SERIAL_DEBUG_ROCKBLOCK
void ISBDConsoleCallback(IridiumSBD *device, char c) {
  DEBUG_SERIAL.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c) {
  DEBUG_SERIAL.write(c);
}
#endif

void rbTransmitIcedrifterData(icedrifterData *idPtr, int idLen) {

#ifdef NEVER_TRANSMIT

#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.print(F("Transmission disabled by NEVER_TRANSMIT switch.\n"));
#endif

#else // NEVER_TRANSMIT

  int rc;
  int recCount;
  int dataLen;
  int chunkLen;
  int i;
  uint8_t *dataPtr;
  uint8_t *chunkPtr;
  uint8_t *wkPtr;
  struct tm* timeInfo;
  char *buffPtr;
  char buff[128];
  char oBuff[340];

  // Setup the RockBLOCK
  isbd.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);

#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.flush();
  DEBUG_SERIAL.println(F("Powering up RockBLOCK\n"));
  DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK

  digitalWrite(ROCKBLOCK_POWER_PIN, HIGH);
  delay(1000);

  isbdss.begin(ROCKBLOCK_BAUD);

  // Step 3: Start talking to the RockBLOCK and power it up
#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.flush();
  DEBUG_SERIAL.println(F("RockBLOCK begin\n"));
  DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK
  isbdss.listen();

  if ((rc = isbd.begin()) == ISBD_SUCCESS) {
#ifdef SERIAL_DEBUG_ROCKBLOCK
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.print(F("Transmitting address="));
    DEBUG_SERIAL.print((long)idPtr, HEX);
    DEBUG_SERIAL.print(F(" length="));
    DEBUG_SERIAL.print(idLen);
    DEBUG_SERIAL.print(F("\n"));
    DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK

    recCount = 0;
    dataPtr = (uint8_t *)idPtr;
    chunkPtr = (uint8_t *)&idcChunk.idcBuffer; 
    dataLen = idLen;

    if (dataLen == 0) {

      oBuff[0] = 0;
//      strcat(oBuff, "idGPSTime=0x");
//      sprintf(buff, "%x", idPtr->idGPSTime);
//      strcat(oBuff, buff);
//      strcat(oBuff, "\nidLastBootTime=0x");
//      sprintf(buff, "%x", idPtr->idLastBootTime);
//      strcat(oBuff, buff);
      strcat(oBuff, "\nGMT=");
      timeInfo = gmtime(&idPtr->idGPSTime);
      buffPtr = asctime(timeInfo);
      strcat(oBuff, buffPtr);
      strcat(oBuff, "\nLBT=");
      timeInfo = gmtime(&idPtr->idLastBootTime);
      buffPtr = asctime(timeInfo);
      strcat(oBuff, buffPtr);
      strcat(oBuff, "\nLat=");
      buffPtr = dtostrf(idPtr->idLatitude, 4, 6, buff);
      strcat(oBuff, buffPtr);
      strcat(oBuff, "\nLon=");
      buffPtr = dtostrf(idPtr->idLongitude, 4, 6, buff);
      strcat(oBuff, buffPtr);
//    strcat(oBuff, "\nTmp=");
//    buffPtr = dtostrf(idPtr->idTemperature, 4, 2, buff);
//    strcat(oBuff, buffPtr);
      strcat(oBuff, "\nBP=");
      buffPtr = dtostrf(idPtr->idPressure, 6, 2, buff);
      strcat(oBuff, buffPtr);
      strcat(oBuff, " Pa\nTs=");
      buffPtr = dtostrf(idPtr->idRemoteTemp, 4, 2, buff);
      strcat(oBuff, buffPtr);
      strcat(oBuff, " C = ");
      buffPtr = dtostrf(((idPtr->idRemoteTemp * 1.8) + 32), 4, 2, buff);
      strcat(oBuff, buffPtr);
      strcat(oBuff, " F\n");

#ifdef SERIAL_DEBUG_ROCKBLOCK
      DEBUG_SERIAL.print(oBuff);
      delay(1000);
//      DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK

      dataLen = strlen(oBuff) + 1;
      rc = isbd.sendSBDBinary((uint8_t *)oBuff, dataLen);

    } else {

      while (dataLen > 0) {
        idcChunk.idcSendTime = idPtr->idGPSTime;
        idcChunk.idcRecordType[0] = 'I';
        idcChunk.idcRecordType[1] = 'D';
        idcChunk.idcRecordNumber = recCount;

        if (dataLen > MAX_CHUNK_DATA_LENGTH) {
          chunkLen = MAX_CHUNK_LENGTH;
          dataLen -= MAX_CHUNK_DATA_LENGTH;
        } else {
          chunkLen = (dataLen + CHUNK_HEADER_SIZE);
          dataLen = 0;
        }

        memmove(chunkPtr, dataPtr, chunkLen);
        dataPtr += MAX_CHUNK_DATA_LENGTH;
        ++recCount;

#ifdef SERIAL_DEBUG_ROCKBLOCK
        DEBUG_SERIAL.flush();
        DEBUG_SERIAL.print(F("Chunk address="));
        DEBUG_SERIAL.print((long)chunkPtr, HEX);
        DEBUG_SERIAL.print(F(" Chunk length="));
        DEBUG_SERIAL.print(chunkLen);
        DEBUG_SERIAL.print(F("\n"));
        wkPtr = (uint8_t *)&idcChunk;

        for (i = 0; i < chunkLen; i++) {
          rbprintHexChar((uint8_t)*wkPtr);
          ++wkPtr;
        }

        DEBUG_SERIAL.print(F("\n"));
        DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK

        rc = isbd.sendSBDBinary((uint8_t *)&idcChunk, chunkLen);
      }
    }
#ifdef SERIAL_DEBUG_ROCKBLOCK
    DEBUG_SERIAL.flush();
    if (rc == 0) {
      DEBUG_SERIAL.print(F("Good return code from send!\n"));
      DEBUG_SERIAL.flush();
    } else {
      DEBUG_SERIAL.print(F("Bad return code from send = "));
      DEBUG_SERIAL.print(rc);
      DEBUG_SERIAL.print(F("\n"));
      DEBUG_SERIAL.flush();
    }
#endif // SERIAL_DEBUG_ROCKBLOCK

#ifdef SERIAL_DEBUG_ROCKBLOCK
  } else {
    DEBUG_SERIAL.print("Bad return code from begin = ");
    DEBUG_SERIAL.print(rc);
    DEBUG_SERIAL.print("\n");
    DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK
  }

  isbd.sleep();
  isbdss.end();
  digitalWrite(ROCKBLOCK_POWER_PIN, LOW);

#endif // NEVER_TRANSMIT
}


