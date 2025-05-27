#include "mode_bt.h"
#include <BluetoothSerial.h>
#include <HardwareSerial.h>
#include "settings.h"
#include "cli.h"
#include <WiFi.h>
#include "gnss_uart.h"

extern Settings settings;

#define UART2_RX 16  // Set according to physical wiring
#define UART2_TX 17

#define MAGIC_SEQ "+++cli+++"

#define SERIAL_SIZE_RX 16384  //Using a large buffer. This might be much bigger than needed but the ESP32 has enough RAM
#define BRIDGE_BUF_SIZE 256
uint8_t buf[BRIDGE_BUF_SIZE];

BluetoothSerial SerialBT;

bool detectMagicChar(char input, const char *magicSeq, size_t &index) {
  size_t len = strlen(magicSeq);
  if (input == magicSeq[index]) {
    index++;
    if (index == len) {
      index = 0;  // Reset for retriggering
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
  btStop();  // stop classic, in case
  delay(50);
  setCpuFrequencyMhz(80);
  btStart();  // start
  // Start Bluetooth
  SerialBT.begin("RFGNSSESP32");
  // Start GNSS UART
  GNSS.setRxBufferSize(SERIAL_SIZE_RX);
  GNSS.begin(settings.gnssBaud, SERIAL_8N1, UART2_RX, UART2_TX);


  size_t magicIndex = 0;

  for (;;) {
    // Bluetooth -> GNSS
    int blen = SerialBT.available();
    if (blen > 0) {
      int rlen = SerialBT.readBytes(buf, min(blen, BRIDGE_BUF_SIZE));
      GNSS.write(buf, rlen);
    }

    // GNSS -> Bluetooth
    int ulen = GNSS.available();
    if (ulen > 0) {
      int rlen = GNSS.readBytes(buf, min(ulen, BRIDGE_BUF_SIZE));
      SerialBT.write(buf, rlen);
    }

    // Magic sequence check
    if (blen > 0) {
      for (int i = 0; i < blen; ++i) {
        if (detectMagicChar(buf[i], MAGIC_SEQ, magicIndex)) {
          cliMode(SerialBT);
          break;
        }
      }
    }

    // No or minimal delay!
    delay(1);  // or just yield()
  }
}