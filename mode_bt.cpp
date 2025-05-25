#include "mode_bt.h"
#include <BluetoothSerial.h>
#include <HardwareSerial.h>
#include "settings.h"
#include "cli.h"
#include <WiFi.h> 
#include "gnss_uart.h"

extern Settings settings;

#define UART2_RX 16 // Set according to physical wiring
#define UART2_TX 17

#define MAGIC_SEQ "+++cli+++"

BluetoothSerial SerialBT;

bool detectMagicChar(char input, const char *magicSeq, size_t &index) {
  size_t len = strlen(magicSeq);
  if (input == magicSeq[index]) {
    index++;
    if (index == len) {
      index = 0; // Reset for retriggering
      return true;
    }
  } else {
    index = (input == magicSeq[0]) ? 1 : 0;
  }
  return false;
}

void runBluetoothTransparentMode() {
    // WiFi off for low power
    WiFi.mode(WIFI_OFF);
    btStop(); // stop classic, in case
    delay(50);
    btStart(); // start
    // Start Bluetooth
    SerialBT.begin("RFGNSSESP32");
    // Start GNSS UART
    GNSS.begin(settings.gnssBaud, SERIAL_8N1, UART2_RX, UART2_TX);

  size_t magicIndex = 0;

  for (;;) {
    if (SerialBT.available()) {
      char c = SerialBT.read();
      if (detectMagicChar(c, MAGIC_SEQ, magicIndex)) {
        cliMode(SerialBT);
        continue;
      }
      GNSS.write((uint8_t)c);
    }
    if (GNSS.available()) {
      SerialBT.write(GNSS.read());
    }
    delay(1);
  }
}