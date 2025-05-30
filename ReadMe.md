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
