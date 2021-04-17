/*
  SerialPassthrough sketch

  Some boards, like the Arduino 101, the MKR1000, Zero, or the Micro, have one
  hardware serial port attached to Digital pins 0-1, and a separate USB serial
  port attached to the IDE Serial Monitor. This means that the "serial
  passthrough" which is possible with the Arduino UNO (commonly used to interact
  with devices/shields that require configuration via serial AT commands) will
  not work by default.

  This sketch allows you to emulate the serial passthrough behaviour. Any text
  you type in the IDE Serial monitor will be written out to the serial port on
  Digital pins 0 and 1, and vice-versa.

  On the 101, MKR1000, Zero, and Micro, "Serial" refers to the USB Serial port
  attached to the Serial Monitor, and "Serial1" refers to the hardware serial
  port attached to pins 0 and 1. This sketch will emulate Serial passthrough
  using those two Serial ports on the boards mentioned above, but you can change
  these names to connect any two serial ports on a board that has multiple ports.

  created 23 May 2016
  by Erik Nyquist
*/

#include <SoftwareSerial.h>

#define CHAIN_POWER_PIN 1
#define CHAIN_RX 3
#define CHAIN_TX 2 //swapped

SoftwareSerial schain(CHAIN_RX, CHAIN_TX);

int firstTime;

const char hexchars[] = "0123456789ABCDEF";

void printHexChar(uint8_t x) {
  Serial.print(hexchars[(x >> 4)]);
  Serial.print(hexchars[(x & 0x0f)]);
}

void printHelp() {
  Serial.print("\n*** Valid commands ***\n");
  Serial.print("0 = +1::chain\n");
  Serial.print("1 = +1::light\n");
  Serial.print("2 = +1::getall\n");
  Serial.print("3 = +1::measure\n");
  Serial.print("4 = +1::debug=0\n");
  Serial.print("5 = +1::debug=1\n\n");
//  Serial.print("Do not press enter, just type a number.\n");
}

void setup() {

  pinMode(CHAIN_POWER_PIN, OUTPUT);
  digitalWrite(CHAIN_POWER_PIN, HIGH);

  Serial.begin(115200);
  schain.begin(9600);

  schain.listen();

  firstTime = true;
}

void loop() {

  int i;
  char cmd;
  char byteRead;

  if (firstTime == true) {

    delay(5000);  // Wait for the serial port to connect.

    Serial.print("Powering on the chain.\n");

    // 15 second delay for the chain hardware to initialize.
    for(i = 0; i < 15; ++i) {
      delay(1000);
    }

    Serial.print("Passthrough ready.\n");
    printHelp();
    firstTime = false;
  }

  if (Serial.available()) {      // If anything comes in Serial (USB),
    cmd = Serial.read();
    Serial.print("\nSerial Received 0x");
    printHexChar(cmd);
    Serial.print("\n");

    switch (cmd) {
      case '0':
        Serial.print("Sending +1::chain\\n\n");
        schain.print("+1::chain\n");
        break;

      case '1':
        Serial.print("Sending +1::light\\n\n");
        schain.print("+1::light\n");
        break;

      case '2':
        Serial.print("Sending +1::getall\\n\n");
        schain.print("+1::getall\n");
        break;

      case '3':
        Serial.print("Sending +1::measure\\n\n");
        schain.print("+1::measure\n");
        break;

      case '4':
        Serial.print("Sending +1::debug=0\\n\n");
        schain.print("+1::debug=0\n");
        break;

      case '5':
        Serial.print("Sending +1::debug=1\\n\n");
        schain.print("+1::debug=1\n");
        break;

      // ignore new line;
      case 0x0a:
      case 0x0d:
      case 0xe1:
        break;

      default:
        Serial.print("\nInvalid command = 0x");
        printHexChar(cmd);
        Serial.print("\n");
        printHelp();
    }
    schain.flush();
  }

  if (schain.available()) {     // If anything comes in from schain
//    printHexChar(schain.read());   // read it and send it out Serial (USB)
    Serial.write(schain.read());
    Serial.flush();
  }
}
