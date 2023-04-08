/*  ============================================================================

     MrDIY ULP Trigger Sensors

     THE RECEIVER

  ============================================================================= */

#include <ESP8266WiFi.h>
#include <espnow.h>
uint8_t  fixedAddress[]      = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; // locally managed macAddress


/* ############################ Setup ############################################ */

void setup() {

  Serial.begin(115200);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  wifi_set_macaddr(STATION_IF, &fixedAddress[0]);
  if (esp_now_init() != 0) {
    delay(1000);
    ESP.restart();
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

/* ############################ Loop ############################################# */

void loop() {}

void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  Serial.write(incomingData, len);
  Serial.write('\n');
}
