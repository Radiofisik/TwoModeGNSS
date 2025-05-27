#include "ntrip.h"
#include <Arduino.h>

NtripSourceClient::NtripSourceClient()
  : connected(false), bytesSent(0),
    lastSentRTCM_ms(0), lastReport_ms(0),
    connectionTimeout(3000),
    maxTimeBeforeHangup_ms(10000)
{}

bool NtripSourceClient::connect(const char* casterHost, uint16_t casterPort,
                                const char* agentName, const char* mountpoint, const char* password)
{
  if (client.connected()) {
    client.stop();
    delay(100);
  }
  Serial.printf("[NTRIP] Opening socket to %s:%d\n", casterHost, casterPort);

  if (!client.connect(casterHost, casterPort, connectionTimeout)) {
    Serial.println("[NTRIP] TCP connect failed");
    connected = false;
    return false;
  }

  // Send RTKLIB/Source header (credentials)
  sendHeader(agentName, mountpoint, password);

  // Wait for response
  unsigned long timeout = millis();
  bool connectionSuccess = false;
  char response[512];
  int responseSpot = 0;

  while (!client.available()) {
    if (millis() - timeout > connectionTimeout) {
      Serial.println("[NTRIP] Caster Timeout");
      client.stop();
      connected = false;
      return false;
    }
    delay(100);
    yield();
  }
  while (client.available()) {
    char chr = client.read();
    if (responseSpot < 511) response[responseSpot++] = chr;
    // Look for 'ICY 200 OK'
    if (strstr(response, "200")) connectionSuccess = true;
    yield();
  }
  response[responseSpot] = '\0';

  if (!connectionSuccess) {
    Serial.printf("[NTRIP] Failed to connect: %s\n", response);
    client.stop();
    connected = false;
    return false;
  }
  Serial.printf("[NTRIP] Connected! Response: %s\n", response);
  connected = true;
  bytesSent = 0;
  lastSentRTCM_ms = millis();
  lastReport_ms = millis();
  return true;
}

void NtripSourceClient::sendHeader(const char* agentName, const char* mountpoint, const char* password) {
  // Compose header
  char hdr[256];
  snprintf(hdr, sizeof(hdr),
           "SOURCE %s /%s\r\nSource-Agent: NTRIP %s\r\n\r\n",
           password, mountpoint, agentName);
  Serial.printf("[NTRIP] Sending header:\n%s\n", hdr);
  client.write((const uint8_t*)hdr, strlen(hdr));
}

void NtripSourceClient::sendData(const uint8_t* data, size_t len) {
  if (connected && client.connected()) {
    client.write(data, len);
    bytesSent += len;
    lastSentRTCM_ms = millis();
  }
}

void NtripSourceClient::sendByte(uint8_t b) {
  if (connected && client.connected()) {
    client.write(b);
    ++bytesSent;
    lastSentRTCM_ms = millis();
  }
}

bool NtripSourceClient::isConnected() {
  return connected && client.connected();
}

uint32_t NtripSourceClient::getBytesSent() const {
  return bytesSent;
}
void NtripSourceClient::resetBytesSent() {
  bytesSent = 0;
}

void NtripSourceClient::disconnect() {
  Serial.println("Disconnecting NTRIP...");
  client.stop();
  connected = false;
  bytesSent = 0;
}

void NtripSourceClient::handle() {
  // Hangup if not sending RTCM for too long
  if (connected && (millis() - lastSentRTCM_ms > maxTimeBeforeHangup_ms)) {
    Serial.println("[NTRIP] RTCM timeout. Disconnecting...");
    disconnect();
  }
  // Optionally print stats
  if (connected && (millis() - lastReport_ms > 250)) {
    lastReport_ms += 250;
    Serial.printf("[NTRIP] Total sent: %u\n", bytesSent);
  }
}