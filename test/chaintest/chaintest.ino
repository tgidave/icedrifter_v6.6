#include <SoftwareSerial.h>

//#define TEMP_DATA_COUNT   160
//#define LIGHT_DATA_COUNT  64
#define TEMP_DATA_COUNT   16
#define LIGHT_DATA_COUNT  6
#define LIGHT_DATA_FIELDS 4

#define RGB_DATA_FIELDS   3

#define TEMP_DATA_SIZE (TEMP_DATA_COUNT * sizeof(uint16_t))
#define LIGHT_DATA_SIZE (LIGHT_DATA_COUNT * LIGHT_DATA_FIELDS * sizeof(uint16_t))

#define CHAIN_POWER 1
#define CHAIN_RX 3
#define CHAIN_TX 2

struct rawLightData {
  uint16_t rldClear;
  uint16_t rldRed;
  uint16_t rldGreen;
  uint16_t rldBlue;
};

uint8_t rgbBuff[LIGHT_DATA_COUNT * RGB_DATA_FIELDS];

struct tlBuffer {
  uint8_t tempData[TEMP_DATA_SIZE];
  uint8_t lightData[LIGHT_DATA_SIZE];
} tlBuffer;

struct ddBuffer {
  uint16_t ddTemp[TEMP_DATA_COUNT];
  uint16_t ddLight[LIGHT_DATA_COUNT][LIGHT_DATA_FIELDS];
} ddBuffer;

union dataBuffer {
  uint8_t dbuff[TEMP_DATA_SIZE + LIGHT_DATA_SIZE];
  struct tlBuffer tlBuff;
  struct ddBuffer ddBuff;
};

union dataBuffer dataBuff;

SoftwareSerial schain(CHAIN_RX, CHAIN_TX);

char hexchars[] = "0123456789ABCDEF";

void printHexChar(uint8_t x) {
  Serial.print(hexchars[(x >> 4)]);
  Serial.print(hexchars[(x & 0x0f)]);
} 

void setup() {

  Serial.begin(115200);
  schain.begin(9600);

  schain.listen();

  pinMode(CHAIN_POWER, OUTPUT);
  digitalWrite(CHAIN_POWER, HIGH);
  delay(1000);


  Serial.print("Chain test.\r\n");
}

void loop() { 

  int i = 0; 

  uint32_t startTime;
  uint32_t tempTime;

  uint16_t waitSeconds;

  bool timeoutError;
  
  uint8_t tmp;

  float ltClear;

  uint8_t rgbRed;
  uint8_t rgbGreen;
  uint8_t rgbBlue;

  Serial.print("Loop start\r\n");
  Serial.flush();

  schain.print("+1::measure\n");
  startTime = millis();
  timeoutError = false;
  waitSeconds = 0;

//  while (1) {
//    delay(1000);
//  }

  while (i < (TEMP_DATA_SIZE + LIGHT_DATA_SIZE)) {
    if (schain.available()) {
      dataBuff.dbuff[i] = schain.read();
      ++i;
    }

    if ((millis() - startTime) > (2UL * 60UL * 1000UL)) {
      timeoutError = true;
      break;
    }
  }

  if (timeoutError == false) {

    Serial.print("Raw temperature data converted to little endian\r\n");

    for (i = 0; i < TEMP_DATA_SIZE; i += 2) {
      tmp = dataBuff.tlBuff.tempData[i];
      dataBuff.tlBuff.tempData[i] = dataBuff.tlBuff.tempData[i + 1];
      dataBuff.tlBuff.tempData[i + 1] = tmp;
      printHexChar(dataBuff.tlBuff.tempData[i]);
      printHexChar(dataBuff.tlBuff.tempData[i + 1]);
    }

    Serial.print("\r\nTemperature data in Centigrade\r\n"); 

    for (i = 0; i < TEMP_DATA_COUNT; ++i) {
      if (dataBuff.ddBuff.ddTemp[i] & 0x8000) {
        Serial.print(double((dataBuff.ddBuff.ddTemp[i] & 0x7fff) - 0x8000) / 128.0 ,8);
      } else {
        Serial.print(double(dataBuff.ddBuff.ddTemp[i]) / 128.0 ,8);
      }
      Serial.print("\r\n");
    }

    Serial.print("\r\nRaw light data converted to little endian\r\n");

    for (i = 0; i < LIGHT_DATA_SIZE; i += 2) {
      tmp = dataBuff.tlBuff.lightData[i];
      dataBuff.tlBuff.lightData[i] = dataBuff.tlBuff.lightData[i + 1];
      dataBuff.tlBuff.lightData[i + 1] = tmp;

      printHexChar(dataBuff.tlBuff.lightData[i]);
      printHexChar(dataBuff.tlBuff.lightData[i + 1]);
    }

    Serial.print("\r\nLight data\r\n"); 

    for (i = 0; i < LIGHT_DATA_COUNT; i++) {
      Serial.print(" c = "); 
      Serial.print(dataBuff.ddBuff.ddLight[i][0]);
      Serial.print(" red = "); 
      Serial.print(dataBuff.ddBuff.ddLight[i][1]);
      Serial.print(" green = ");
      Serial.print(dataBuff.ddBuff.ddLight[i][2]);
      Serial.print(" blue = ");
      Serial.print(dataBuff.ddBuff.ddLight[i][3]);

      if (dataBuff.ddBuff.ddLight[i][0] == 0) {
        rgbRed = rgbGreen = rgbBlue = 0;
      } else {
        ltClear = (float)dataBuff.ddBuff.ddLight[i][0];
        rgbRed = (float)dataBuff.ddBuff.ddLight[i][1] / ltClear * 255.0;
        rgbGreen = (float)dataBuff.ddBuff.ddLight[i][2] / ltClear * 255.0;
        rgbBlue = (float)dataBuff.ddBuff.ddLight[i][3] / ltClear * 255.0;
      }

      Serial.print(" RGB = ");
      Serial.print(rgbRed);
      Serial.print(" ");
      Serial.print(rgbGreen);
      Serial.print(" ");
      Serial.print(rgbBlue);
      Serial.print("\r\n");
    }

  } else {
    Serial.print("\r\nSerial timeout on chain!!!\r\n");
  }

  if (schain.available()) {
    Serial.print("\r\nToo much chain data received!!!\r\n");
  } 

  while (1) {
    delay(1000);
  }
}
