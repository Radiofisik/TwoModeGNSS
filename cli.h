#pragma once
#include <BluetoothSerial.h>

#define MAGIC_SEQ "+++cli+++"

void cliMode(Stream &serial);

bool detectMagicChar(char input, const char *magicSeq, size_t &index);