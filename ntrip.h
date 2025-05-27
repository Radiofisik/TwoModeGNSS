#pragma once
#include <WiFi.h>

class NtripSourceClient {
public:
    NtripSourceClient();
    bool connect(const char* casterHost, uint16_t casterPort,
                 const char* agentName, const char* mountpoint, const char* password);
    void disconnect();
    bool isConnected();
    void sendHeader(const char* agentName, const char* mountpoint, const char* password);
    void sendData(const uint8_t* data, size_t len);
    void sendByte(uint8_t b);
    void handle();
    uint32_t getBytesSent() const;
    void resetBytesSent();

private:
    WiFiClient client;
    bool connected;
    uint32_t bytesSent;
    unsigned long lastSentRTCM_ms;
    unsigned long lastReport_ms;
    int connectionTimeout;
    int maxTimeBeforeHangup_ms;
};