#include "mode_bt.h"
#include "mode_ntrip.h"
#include "settings.h"

#define BUTTON_PIN 0  // set according to your connection

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  settingsLoad();

  if (settings.oneShotMode == 1 || settings.oneShotMode == 2) {
    // Clear flag for next boot!
    uint8_t mode = settings.oneShotMode;
    settings.oneShotMode = 0;
    settingsSave();

    if (mode == 1) {
      runNtripServerMode();
    } else if (mode == 2) {
      runBluetoothTransparentMode();
    }
  }

  if (digitalRead(BUTTON_PIN) == LOW) {
    runNtripServerMode();
  } else {
    runBluetoothTransparentMode();
  }
}

void loop() {
  // Nothing, all logic inside task loops
}