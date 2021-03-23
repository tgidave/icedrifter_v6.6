#include <SoftwareSerial.h>

#include "icedrifter.h"
#include "chain.h"

#ifdef PROCESS_CHAIN_DATA

SoftwareSerial schain(CHAIN_RX, CHAIN_TX); 

void processChainData(icedrifterData* idPtr) {

  int i = 0;

  int chainByteCount;
  int lightByteCount;
//  int main(int argc, char *argv[]) {
//    ,nn
//  }
  uint8_t* buffPtr;
  uint8_t* wkPtr;
  uint16_t* wordPtr;
  uint32_t startTime;
  uint32_t tempTime;

  int chainError;

  uint8_t tmp;

  float ltClear;

  uint8_t rgbRed;
  uint8_t rgbGreen;
  uint8_t rgbBlue;

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("\nPowering up chain.\n"));
#endif // SERIAL_DEBUG

  digitalWrite(CHAIN_POWER_PIN, HIGH);

  // 15 second delay for the chain hardware to initialize.
  for(i = 0; i < 15; ++i) {
    delay(1000);
  }

  wkPtr = (uint8_t*)&idPtr->idChainData;

  for (i = 0; i < TEMP_DATA_SIZE + LIGHT_DATA_SIZE; ++i) {
    *wkPtr = 0;
  }

  schain = SoftwareSerial(CHAIN_RX, CHAIN_TX); 

  schain.begin(9600);
  schain.flush();
  schain.listen();

  buffPtr = (uint8_t *)&idPtr->idChainData;
  idPtr->idcdError = 0;

  schain.print(F("+1::chain\n"));
  startTime = millis();
  idPtr->idTempByteCount = 0;

  while (idPtr->idTempByteCount < (TEMP_DATA_SIZE)) {
    if (schain.available()) {
      *buffPtr = schain.read();
      ++buffPtr;
      ++idPtr->idTempByteCount;
    }

    if ((millis() - startTime) > (TEMP_CHAIN_TIMEOUT_MINUTES * 60UL * 1000UL)) {
      idPtr->idcdError |= TEMP_CHAIN_TIMEOUT_ERROR;
      break;
    }
  }

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Received "));
  DEBUG_SERIAL.print(idPtr->idTempByteCount);
  DEBUG_SERIAL.print(F(" bytes of temp chain data.\n"));

  if (idPtr->idcdError & TEMP_CHAIN_TIMEOUT_ERROR) {
    DEBUG_SERIAL.print(F("\nTimeout on temp chain!!!\n"));
    DEBUG_SERIAL.print((TEMP_DATA_SIZE));
    DEBUG_SERIAL.print(F(" bytes requested but only "));
    DEBUG_SERIAL.print(idPtr->idTempByteCount);
    DEBUG_SERIAL.print(F(" bytes received\n"));
  }
#endif // SERIAL_DEBUG

  if (schain.available()) {
    idPtr->idcdError |= TEMP_CHAIN_OVERRUN_ERROR;

#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("\nToo much chain data received!!!\n"));
    DEBUG_SERIAL.print(F("\nReturning with chainError = "));
    DEBUG_SERIAL.print(idPtr->idcdError);
    DEBUG_SERIAL.print(F("\n"));
#endif // SERIAL_DEBUG

    schain.flush();
    schain.end();
    digitalWrite(CHAIN_POWER_PIN, LOW);
#ifdef DROP_CHAIN_RX_TX
    digitalWrite(CHAIN_RX, LOW);
    digitalWrite(CHAIN_TX, LOW);
#endif // DROP_CHAIN_RX_TX
    delay(1000);
    return;
  }

  schain.print(F("+1::light\n"));
  startTime = millis();
  idPtr->idLightByteCount = 0;


  while (idPtr->idLightByteCount < (LIGHT_DATA_SIZE)) {
    if (schain.available()) {
      *buffPtr = schain.read();
      ++idPtr->idLightByteCount;
      ++buffPtr;
    }

    if ((millis() - startTime) > (LIGHT_CHAIN_TIMEOUT_MINUTES * 60UL * 1000UL)) {
      idPtr->idcdError |= LIGHT_CHAIN_TIMEOUT_ERROR;
      break;
    }
  }

//  totalByteCount += idPtr->idLightByteCount;

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Received "));
  DEBUG_SERIAL.print(idPtr->idLightByteCount);
  DEBUG_SERIAL.print(F(" bytes of light data.\n"));

  if (chainError != 0) {
    DEBUG_SERIAL.print(F("\nTimeout on light chain!!!\n"));
    DEBUG_SERIAL.print(LIGHT_DATA_SIZE);
    DEBUG_SERIAL.print(F(" bytes requested but only "));
    DEBUG_SERIAL.print(idPtr->idLightByteCount);
    DEBUG_SERIAL.print(F(" bytes received\n"));
  }
#endif // SERIAL_DEBUG

  if (schain.available()) {
    idPtr->idcdError |= LIGHT_CHAIN_OVERRUN_ERROR;
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("\nToo much light data received!!!\n"));
#endif // SERIAL_DEBUG
  }

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("\nReturning with idData.idcdError = "));
  DEBUG_SERIAL.print(idPtr->idcdError);
  DEBUG_SERIAL.print(F("\n"));
#endif // SERIAL_DEBUG

  schain.flush();
  schain.end();
  digitalWrite(CHAIN_POWER_PIN, LOW);
#ifdef DROP_CHAIN_RX_TX
  digitalWrite(CHAIN_RX, LOW);
  digitalWrite(CHAIN_TX, LOW);
#endif // DROP_CHAIN_RX_TX
  delay(1000);
}

#endif // PROCESS_CHAIN_DATA

