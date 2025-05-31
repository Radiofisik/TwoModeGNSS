This project is NTRIP server / transparent Bluetooth interface for GNSS receiver based on UM980. It is based on ESP32 board and uses the pins for UM980 connections

```c
#define UART2_RX 16  // Set according to physical wiring
#define UART2_TX 17
```

The firmware supports three mode
- Bluetooth classic transparent proxy to be used with android phone with software like SW Maps
- NTRIP server to be used with caster like YCCaster
- CLI mode to adjust the settings. To enter cli mode send send magic sequence +++cli+++ then you can send you settings and reboot

```
set ssid yourNetworkSSID
set wifipass yourWifiPassword
set host 192.168.1.14
set port 2101
set mount RFM
set pass 12345
set baud 115200
save
show
```

## Quick start
1) turn on the device. It will enter Bluetooth transparent mode
2) configure your GNSS receiver settings like for UM980. Execute `FRESET` first, then commands
```
unlog com1
unlog com2
unlog com3

config undulation 0
mode base time 60
CONFIG SIGNALGROUP 2

CONFIG MMP ENABLE

rtcm1006 10
rtcm1033 10

rtcm1019 10
rtcm1020 10

rtcm1033 30

rtcm1077 2
rtcm1087 2
rtcm1097 2
rtcm1127 2

saveconfig
```
3) enter to cli mode with `+++cli+++`
4) send your settings 
```
set ssid yourNetworkSSID
set wifipass yourWifiPassword
set host 192.168.1.14
set port 2101
set mount RFM
set pass 12345
set baud 115200
save
show
```
5) reboot ntrip mode with `reboot ntrip`
6) now you have working base station but if you turn off and on you have to enter cli and reboot ntrip again. To make ntrip mode default connect GPIO15 to GND

This repo link
https://github.com/Radiofisik/TwoModeGNSS