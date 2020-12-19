#ifndef _BMP280_H
#define _BMP280_H

#define BMP_SCK 16
#define BMP_MISO 12
#define BMP_MOSI 17
#define BMP_CS 10

#define BMP280_POWER_PIN 30

void getBMP280Data(icedrifterData* idData);

#endif
