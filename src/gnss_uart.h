#pragma once
#include <HardwareSerial.h>

#define UART2_RX 16  // Set according to physical wiring
#define UART2_TX 17

#define SERIAL_SIZE_RX 4096  

extern HardwareSerial GNSS;

void initGNSSUART();