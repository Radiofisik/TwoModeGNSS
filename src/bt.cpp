#include "bt.h"
#include "esp_bt.h"

BluetoothSerial SerialBT;
uint8_t buf[BT_BUF_SIZE];

void initBluetooth() {
  btStop();  // stop classic, in case
  delay(50);
  btStart();  // start
  esp_bt_controller_mem_release(ESP_BT_MODE_BLE); // If not using BLE
  esp_bt_sleep_enable();

  // Start Bluetooth
  SerialBT.begin("RFGNSSESP32");
}