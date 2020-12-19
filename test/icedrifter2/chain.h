
#ifndef CHAIN_H
#define CHAIN_H

#define CHAIN_POWER_PIN 23

// Number of minutes to wait while reading chain data before it times out.
#define CHAIN_READ_TIMEOUT  3UL

int processChainData(uint8_t* tempDataPtr, uint8_t* lightDataPtr);

#endif 
