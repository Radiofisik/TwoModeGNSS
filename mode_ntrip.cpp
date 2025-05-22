#include "mode_ntrip.h"
#include "settings.h"
#include <WiFi.h>
#include <WiFiClient.h>
// #include "NTRIPClient.h" // You’ll need to install or copy an NTRIP server/client implementation
#include <HardwareSerial.h>
#include "gnss_uart.h"

extern Settings settings;

#define UART2_RX 16 // Set according to physical wiring
#define UART2_TX 17


void runNtripServerMode() {
  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(settings.ssid, settings.wifiPassword);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    if (millis() - t0 > 15000) {
      // timeout, fallback to BT for config if needed
      // blink LED or similar, or reboot
      ESP.restart();
    }
  }

  GNSS.begin(settings.gnssBaud, SERIAL_8N1, UART2_RX, UART2_TX);

  // NTRIPServer ntrip;
  // ntrip.begin(settings.casterHost,
  //             settings.casterPort,
  //             settings.mountpoint,
  //             settings.ntripUser,
  //             settings.ntripPassword);

  for (;;) {
    // Forward GNSS output to NTRIP server
    while (GNSS.available()) {
      // ntrip.send(GNSS.read());
    }
    // ntrip.handleClient(); // allow NTRIP server to clear clients/make connections
    delay(1);
  }
}