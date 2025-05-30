#include "cli.h"
#include "settings.h"

extern Settings settings;

bool detectMagicChar(char input, const char *magicSeq, size_t &index) {
  size_t len = strlen(magicSeq);
  if (input == magicSeq[index]) {
    index++;
    if (index == len) {
      index = 0;  // Reset for retriggering
      return true;
    }
  } else {
    index = (input == magicSeq[0]) ? 1 : 0;
  }
  return false;
}

// Helper: Read one line with echo (supports backspace)
String cliReadLineWithEcho(Stream &serial) {
  String line = "";
  while (true) {
    while (!serial.available()) delay(10);
    char c = serial.read();
    if (c == '\r') continue;   // Ignore CR
    if (c == '\n') break;      // End of line
    if (c == 8 || c == 127) {  // Backspace or DEL
      if (line.length() > 0) {
        line.remove(line.length() - 1);
        serial.print("\b \b");  // Move cursor back, erase char, move back
      }
    } else if (isPrintable(c)) {
      line += c;
      serial.print(c);
    }
  }
  serial.println();
  return line;
}

void cliMode(Stream &serial) {
  serial.println("\n--- CLI MODE ---");
  while (true) {
    serial.print("Enter command: (show, set param_name value, save, exit, reboot [bt, ntrip])\r\n> ");
    String cmdline = cliReadLineWithEcho(serial);
    cmdline.trim();
    if (cmdline.length() == 0) continue;

    int firstSpace = cmdline.indexOf(' ');
    String cmd = (firstSpace == -1) ? cmdline : cmdline.substring(0, firstSpace);
    cmd.trim();

    if (cmd == "show") {
      serial.printf("wifi: %s / %s\r\n", settings.ssid, settings.wifiPassword);
      serial.printf("ntrip host: %s:%u\r\n", settings.casterHost, settings.casterPort);
      serial.printf("mountpoint: %s\r\n", settings.mountpoint);
      serial.printf("user: %s\r\n", settings.ntripUser);
      serial.printf("pass: %s\r\n", settings.ntripPassword);
      serial.printf("baud: %lu\r\n", (unsigned long)settings.gnssBaud);
    } else if (cmd == "set") {
      // Syntax: set param_name value
      String remain = cmdline.substring(firstSpace + 1);
      remain.trim();
      int paramSpace = remain.indexOf(' ');
      if (paramSpace == -1) {
        serial.println("Invalid set command. Use: set param_name value");
        continue;
      }
      String par = remain.substring(0, paramSpace);
      String val = remain.substring(paramSpace + 1);
      par.trim();
      val.trim();

      if (par == "ssid") strncpy(settings.ssid, val.c_str(), sizeof(settings.ssid) - 1);
      else if (par == "wifipass") strncpy(settings.wifiPassword, val.c_str(), sizeof(settings.wifiPassword) - 1);
      else if (par == "host") strncpy(settings.casterHost, val.c_str(), sizeof(settings.casterHost) - 1);
      else if (par == "port") settings.casterPort = val.toInt();
      else if (par == "mount") strncpy(settings.mountpoint, val.c_str(), sizeof(settings.mountpoint) - 1);
      else if (par == "user") strncpy(settings.ntripUser, val.c_str(), sizeof(settings.ntripUser) - 1);
      else if (par == "pass") strncpy(settings.ntripPassword, val.c_str(), sizeof(settings.ntripPassword) - 1);
      else if (par == "baud") settings.gnssBaud = val.toInt();
      else serial.println("Unknown param");
    } else if (cmd == "save") {
      settingsSave();
      serial.println("Settings saved.");
    } else if (cmd == "exit") {
      serial.println("Exiting CLI.");
      break;
    } else if (cmd == "reboot") {
      // Syntax: reboot [mode]
      String remain = cmdline.substring(firstSpace + 1);
      remain.trim();

      uint8_t requested = 0;  // 0=use normal
      if (remain.length() == 0) {
        // No mode, just normal reboot
        requested = 0;
      } else if (remain.equalsIgnoreCase("ntrip")) {
        requested = 1;
      } else if (remain.equalsIgnoreCase("bt") || remain.equalsIgnoreCase("bluetooth")) {
        requested = 2;
      } else {
        serial.println("Unknown mode. Use: reboot [ntrip|bt]");
        continue;
      }
      if (requested) {
        settings.oneShotMode = requested;
        settingsSave();  // save override to EEPROM
        serial.printf("Rebooting to %s mode...\r\n", requested == 1 ? "NTRIP" : "Bluetooth");
      } else {
        serial.println("Rebooting...");
      }
      delay(100);
      ESP.restart();  // Reboot immediately
    }
  }
}