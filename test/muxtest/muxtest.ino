/*
 Muxtest

*/

#define GPS_POWER_PIN 3

enum muxCmd {
  muxOff,
  muxGPS,
  muxRockBlock,
  muxChain,
  muxCmdMax,
};

enum muxIx {
  muxINH,
  muxB,
  muxA,
};

const uint8_t muxTable[4][3] = {
  { HIGH, LOW, LOW },
  { LOW, LOW, HIGH },
  { LOW, HIGH, LOW },
  { LOW, HIGH, HIGH },
};

#define RXD1        10
#define TXD1        11

#define muxINHPort  19
#define muxAPort    13
#define muxBPort    12

void SetSerialMux(int muxCommand) {
  if (!((muxCommand >= 0) && (muxCommand < muxCmdMax))) {
    Serial.print(F("Mux command out of range = "));
    Serial.print(muxCommand);
    Serial.print(F("\n"));
    return;
  }

  digitalWrite(muxINHPort, HIGH);
  digitalWrite(muxAPort, muxTable[muxCommand][muxA]);
  digitalWrite(muxBPort, muxTable[muxCommand][muxB]);
  digitalWrite(muxINHPort, muxTable[muxCommand][muxINH]);

}

void SetSerialMuxOff(void) {
  SetSerialMux(muxOff);
}

void SetSerialMuxToGPS(void) {
  SetSerialMux(muxGPS);
}

void SetSerialMuxToRockblock(void) {
  SetSerialMux(muxRockBlock);
}

void SetSerialMuxToChain(void) {
  SetSerialMux(muxChain);
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.

  Serial.begin(115200);

  pinMode(GPS_POWER_PIN, OUTPUT);
  digitalWrite(GPS_POWER_PIN, HIGH);

  pinMode(muxINHPort, OUTPUT);
  pinMode(muxAPort, OUTPUT);
  pinMode(muxBPort, OUTPUT);
  pinMode(RXD1, OUTPUT);
  pinMode(TXD1, OUTPUT);
  SetSerialMuxOff();
}

// the loop function runs over and over again forever
void loop() {
  Serial.print(F("Mux test...\n"));
  delay(1000);

  SetSerialMuxToRockblock();

  while (1) {
    digitalWrite(RXD1, HIGH);
    digitalWrite(TXD1, HIGH);
    delay(500);
    digitalWrite(RXD1, LOW);
    delay(500);
    digitalWrite(RXD1, HIGH);
    digitalWrite(TXD1, LOW);
    delay(500);
    digitalWrite(RXD1, LOW);
    delay(500);
  }
}
