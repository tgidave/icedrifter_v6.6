#ifndef _ROCKBLOCK_H
  #define _ROCKBLOCK_H

#include "icedrifter.h"

#define ROCKBLOCK_RX_PIN 12 // Pin marked RX on RockBlock
#define ROCKBLOCK_TX_PIN 13 // Pin marked TX on RockBlock
#define ROCKBLOCK_SLEEP_PIN 4
#define ROCKBLOCK_BAUD 19200
#define ROCKBLOCK_POWER_PIN 15

#define MAX_CHUNK_LENGTH  340
#define CHUNK_HEADER_SIZE 8
#define MAX_CHUNK_DATA_LENGTH (MAX_CHUNK_LENGTH - CHUNK_HEADER_SIZE)

typedef struct iceDrifterChunk {
#ifdef ARDUINO
  time_t idcSendTime;
#else
  uint32_t idcSendTime;
#endif
  char idcRecordType[2];
  uint16_t idcRecordNumber;
  uint8_t idcBuffer[MAX_CHUNK_LENGTH];
} iceDrifterChunk; 

void rbTransmitIcedrifterData(icedrifterData *, int);

#endif  //_ROCKBLOCK_H
