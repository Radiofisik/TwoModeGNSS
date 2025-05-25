#pragma once
#include <Arduino.h>

struct Settings {
  char ssid[33];
  char wifiPassword[65];
  char casterHost[40];
  uint16_t casterPort;
  char mountpoint[40];
  char ntripUser[40];
  char ntripPassword[40];
  uint32_t gnssBaud;
  uint8_t oneShotMode; // 0=none/normal, 1=ntrip, 2=bt
};

extern Settings settings;

void settingsLoad();
void settingsSave();