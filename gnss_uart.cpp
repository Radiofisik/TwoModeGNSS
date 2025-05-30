#include "gnss_uart.h"
#include "settings.h"
#include <HardwareSerial.h>

HardwareSerial GNSS(2);
extern Settings settings;

void initGNSSUART(){
  // Start GNSS UART
  GNSS.setRxBufferSize(SERIAL_SIZE_RX);
  GNSS.begin(settings.gnssBaud, SERIAL_8N1, UART2_RX, UART2_TX);
}