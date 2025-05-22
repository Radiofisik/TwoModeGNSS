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
#define MAGIC_LEN 8
char magicBuf[MAGIC_LEN+1] = {0};  // 8 chars + null
uint8_t magicPos = 0;

BluetoothSerial SerialBT;

void runBluetoothTransparentMode() {
  // WiFi off for low power
  WiFi.mode(WIFI_OFF);
  btStop(); // stop classic, in case
  delay(50);
  btStart(); // start
  // Start Bluetooth
  SerialBT.begin("GNSSBridge");
  // Start GNSS UART
  GNSS.begin(settings.gnssBaud, SERIAL_8N1, UART2_RX, UART2_TX);

  for (;;) {
    // Bluetooth -> GNSS
    if (SerialBT.available()) {
      int c = SerialBT.read();
     
      //--- Shift buffer and add new character ---
      memmove(magicBuf, magicBuf + 1, MAGIC_LEN - 1);
      magicBuf[MAGIC_LEN - 1] = c;
      magicBuf[MAGIC_LEN] = 0;

      // Check for match
      if (strcmp(magicBuf, MAGIC_SEQ) == 0) {
          // Reset buffer so retrigger can work after return from CLI
          memset(magicBuf, 0, sizeof(magicBuf));
          cliMode(SerialBT);
          continue;
      }

      GNSS.write((uint8_t)c);
    }
    // GNSS -> Bluetooth
    if (GNSS.available()) {
      SerialBT.write(GNSS.read());
    }
    delay(1);
  }
}