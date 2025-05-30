#include "mode_bt.h"
#include <BluetoothSerial.h>
#include <HardwareSerial.h>
#include "settings.h"
#include "cli.h"
#include <WiFi.h>
#include "gnss_uart.h"
#include "esp_wifi.h"
#include "bt.h"

extern Settings settings;


void runBluetoothTransparentMode() {
  // WiFi off for low power
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  setCpuFrequencyMhz(80);

  initBluetooth();
  initGNSSUART();

  size_t magicIndex = 0;

  for (;;) {
    // Bluetooth -> GNSS
    int blen = SerialBT.available();
    if (blen > 0) {
      int rlen = SerialBT.readBytes(buf, min(blen, BT_BUF_SIZE));
      GNSS.write(buf, rlen);
    }

    // GNSS -> Bluetooth
    int ulen = GNSS.available();
    if (ulen > 0) {
      int rlen = GNSS.readBytes(buf, min(ulen, BT_BUF_SIZE));
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

    delay(5); 
  }
}