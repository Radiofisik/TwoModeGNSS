#include "mode_bt.h"
#include "mode_ntrip.h"
#include "settings.h"

#define BUTTON_PIN 0 // set according to your connection

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  settingsLoad();

  // Detect mode
  if (digitalRead(BUTTON_PIN) == LOW) {
    runNtripServerMode();
  } else {
    runBluetoothTransparentMode();
  }
}

void loop() {
  // Nothing, all logic inside task loops
}