#pragma once
#include <BluetoothSerial.h>

#define BT_BUF_SIZE 2048
extern uint8_t buf[BT_BUF_SIZE];

extern BluetoothSerial SerialBT;

void initBluetooth();