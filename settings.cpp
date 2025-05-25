#include "settings.h"
#include <EEPROM.h>

Settings settings;

#define EEPROM_SIZE 512

void settingsDefault() {
  strcpy(settings.ssid, "your_wifi");
  strcpy(settings.wifiPassword, "your_password");
  strcpy(settings.casterHost, "ntrip.caster.tld");
  settings.casterPort = 2101;
  strcpy(settings.mountpoint, "MYMOUNT");
  strcpy(settings.ntripUser, "user");
  strcpy(settings.ntripPassword, "pass");
  settings.gnssBaud = 115200;
  settings.oneShotMode = 0;
}

void settingsLoad() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) != 0xAB) {
    settingsDefault();
    settingsSave();
    return;
  }
  EEPROM.get(1, settings);
}

void settingsSave() {
  EEPROM.write(0, 0xAB);
  EEPROM.put(1, settings);
  EEPROM.commit();
}