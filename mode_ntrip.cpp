#include "mode_ntrip.h"
#include "settings.h"
#include "ntrip.h"
#include "gnss_uart.h"
#include <WiFi.h>

#define UART2_RX 16
#define UART2_TX 17
#define LED_PIN 2

extern Settings settings;

NtripSourceClient ntrip;

// You may tune this chunk size and/or add message detection for RTCM
#define RTCM_BUF_SIZE 256
uint8_t rtcmBuf[RTCM_BUF_SIZE];
size_t rtcmBufPos = 0;

void runNtripServerMode() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);
    delay(100);

    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(settings.ssid, settings.wifiPassword);
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        if (millis() - t0 > 15000) {
            ESP.restart();
        }
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
    digitalWrite(LED_PIN, LOW);

    Serial.print("WiFi connected. IP address: ");
    Serial.println(WiFi.localIP());

    // Start GNSS UART
    GNSS.begin(settings.gnssBaud, SERIAL_8N1, UART2_RX, UART2_TX);

    // Connect to NTRIP Caster as source
    bool connected = ntrip.connect(
        settings.casterHost,
        settings.casterPort,
        "RTKLIB/2.4",      // Agent name (can adjust if needed)
        settings.mountpoint,
        settings.ntripPassword // pass as "password" (username is ignored)
    );

    if (!connected) {
        Serial.println("[NTRIP] Initial connection failed. Rebooting...");
        delay(2000);
        ESP.restart();
    }

    // Main loop
    for (;;) {
        // Buffer and send RTCM in blocks, with header each time
        while (GNSS.available()) {
            uint8_t b = GNSS.read();
            rtcmBuf[rtcmBufPos++] = b;
            if (rtcmBufPos == RTCM_BUF_SIZE) {
                ntrip.sendHeader("RTKLIB/2.4", settings.mountpoint, settings.ntripPassword); // send header before block
                ntrip.sendData(rtcmBuf, rtcmBufPos);
                rtcmBufPos = 0;
            }
        }

        // Flush buffer every 200ms, even if not full
        static unsigned long lastFlush = 0;
        if (rtcmBufPos > 0 && millis() - lastFlush > 200) {
            ntrip.sendHeader("RTKLIB/2.4", settings.mountpoint, settings.ntripPassword);
            ntrip.sendData(rtcmBuf, rtcmBufPos);
            rtcmBufPos = 0;
            lastFlush = millis();
        }

        ntrip.handle();

        // Reconnect logic if dropped
        if (!ntrip.isConnected()) {
            Serial.println("[NTRIP] Disconnected, retrying in 5 seconds...");
            delay(5000);
            ESP.restart(); // or re-run the connect() logic, or break to outer loop
        }

        delay(1);
    }
}