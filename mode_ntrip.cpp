#include "mode_ntrip.h"
#include "settings.h"
#include "gnss_uart.h"
#include <WiFi.h>
#include "bt.h"
#include "cli.h"

#define LED_PIN 2
//choose one
// #define CONSOLE Serial
#define CONSOLE SerialBT

extern Settings settings;

WiFiClient ntripCaster;

long lastSentRTCM_ms = 0;            //Time of last data pushed to socket
int maxTimeBeforeHangup_ms = 10000;  //If we fail to get a complete RTCM frame after 10s, then disconnect from caster
uint32_t serverBytesSent = 0;        //Just a running total
long lastReport_ms = 0;              //Time of last report of bytes sent
long actualReportDelay = 0;

#define NTRIP_BUF_SIZE 2048
uint8_t ntripbuf[NTRIP_BUF_SIZE];

void beginServer() {
  CONSOLE.println("Begin transmitting to caster. Press any key to stop");
  delay(10);  //Wait for any serial to arrive
  while (CONSOLE.available())
    CONSOLE.read();  //Flush

  while (CONSOLE.available() == 0) {
    //Connect if we are not already
    if (ntripCaster.connected() == false) {
      CONSOLE.printf("Opening socket to %s\n", settings.casterHost);

      if (ntripCaster.connect(settings.casterHost, settings.casterPort) == true)  //Attempt connection
      {
        CONSOLE.printf("Connected to %s:%d\n", settings.casterHost, settings.casterPort);

        const int SERVER_BUFFER_SIZE = 512;
        char serverRequest[SERVER_BUFFER_SIZE];

        snprintf(serverRequest,
                 SERVER_BUFFER_SIZE,
                 "SOURCE %s /%s\r\nSource-Agent: NTRIP SparkFun UM980 Server v1.0\r\n\r\n",
                 settings.ntripPassword, settings.mountpoint);

        CONSOLE.println(F("Sending server request:"));
        CONSOLE.println(serverRequest);
        ntripCaster.write(serverRequest, strlen(serverRequest));

        //Wait for response
        unsigned long timeout = millis();
        while (ntripCaster.available() == 0) {
          if (millis() - timeout > 5000) {
            CONSOLE.println("Caster timed out!");
            ntripCaster.stop();
            return;
          }
          delay(10);
        }

        //Check reply
        bool connectionSuccess = false;
        char response[512];
        int responseSpot = 0;
        while (ntripCaster.available()) {
          response[responseSpot++] = ntripCaster.read();
          if (strstr(response, "200") != nullptr)  //Look for 'ICY 200 OK'
            connectionSuccess = true;
          if (responseSpot == 512 - 1)
            break;
        }
        response[responseSpot] = '\0';

        if (connectionSuccess == false) {
          CONSOLE.printf("Failed to connect to Caster: %s", response);
          return;
        }
      }  //End attempt to connect
      else {
        CONSOLE.println("Connection to host failed");
        return;
      }
    }  //End connected == false

    if (ntripCaster.connected() == true) {
      delay(10);
      while (CONSOLE.available())
        CONSOLE.read();  //Flush any endlines or carriage returns

      lastReport_ms = millis();
      lastSentRTCM_ms = millis();

      //This is the main sending loop. We scan for new data but processRTCM() is where the data actually gets sent out.
      while (1) {
        if (CONSOLE.available())
          break;

        //Write incoming RTCM to the NTRIP Caster

        int glen = GNSS.available();
        if (glen > 0) {
          int rlen = GNSS.readBytes(ntripbuf, min(glen, NTRIP_BUF_SIZE));
          ntripCaster.write(ntripbuf, rlen);
          serverBytesSent += rlen;
          lastSentRTCM_ms = millis();
        }

        //Close socket if we don't have new data for 10s
        //RTK2Go will ban your IP address if you abuse it. See http://www.rtk2go.com/how-to-get-your-ip-banned/
        //So let's not leave the socket open/hanging without data
        if (millis() - lastSentRTCM_ms > maxTimeBeforeHangup_ms) {
          CONSOLE.println("RTCM timeout. Disconnecting...");
          ntripCaster.stop();
          return;
        }

        delay(10);

        //Report some statistics every 250
        actualReportDelay = millis() - lastReport_ms;
        if (actualReportDelay > 1000) {
          ntripCaster.stop();
          return;
        }
        if (actualReportDelay > 250) {
          lastReport_ms += actualReportDelay;
          CONSOLE.printf("Total sent: %d\n", serverBytesSent);
        }
      }
    }

    delay(10);
  }
}

void runNtripServerMode() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  initGNSSUART();
  initBluetooth();
  delay(250);

  CONSOLE.println();
  CONSOLE.println("ntrip server mode");

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(settings.ssid, settings.wifiPassword);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    if (millis() - t0 > 15000) {
      if (!CONSOLE.available()) {
        CONSOLE.println("restart to ntrip mode due to timeout connecting to wifi");
        settings.oneShotMode = 1;
        settingsSave();
        ESP.restart();
      }
      else{
         cliMode(CONSOLE);
      }
    }
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  digitalWrite(LED_PIN, LOW);

  CONSOLE.print("WiFi connected. IP address: ");
  CONSOLE.println(WiFi.localIP());


  size_t magicIndex = 0;

  //Clear any serial characters from the buffer
  while (CONSOLE.available())
    CONSOLE.read();


  // Main loop
  for (;;) {
    beginServer();

    int blen = CONSOLE.available();
    // Magic sequence check
    if (blen > 0) {
      int rlen = CONSOLE.readBytes(buf, min(blen, BT_BUF_SIZE));
      for (int i = 0; i < blen; ++i) {
        if (detectMagicChar(buf[i], MAGIC_SEQ, magicIndex)) {
          cliMode(CONSOLE);
          break;
        }
      }
    }

    delay(500);
  }
}