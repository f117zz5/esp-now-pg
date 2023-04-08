/*  ============================================================================

     MrDIY ULP Trigger Sensors

     THE GATEWAY (MINIMAL)

  ============================================================================= */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

/* ------------------------ Change Me --------------------------------------- */

const char*     ssid              = "........";
const char*     password          = "........";
const char*     mqtt_server       = "........";
const char*     mqtt_username     = "........";
const char*     mqtt_password     = "........";
bool            use_fahrenheit    = true;

/* ------------------------------------------------------------------ */

#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

/* ------------------------ Messages --------------------------------------- */

#define MESH_ID               6734922
#define GROUP_SWITCH          1
#define GROUP_HT              2
#define GROUP_MOTION          3

typedef struct struct_message {
  int     mesh_id;
  uint8_t sensor_id[6];
  byte    category;
  bool    status ;
  float   temperature;
  float   humidity;
  float   battery;
} struct_message;


struct_message  msg;
uint8_t         incomingData[sizeof(struct struct_message)];
size_t          received_msg_length;

/* --------------------------- Wifi/MQTT ---------------------------------------------- */

WiFiClient    wifiClient;
#define       MQTT_MSG_SIZE    200
char          mqttTopic[MQTT_MSG_SIZE];
#define       MSG_BUFFER_SIZE  (50)
PubSubClient  mqttClient(wifiClient);
String        thingName;
const char*   willTopic         = "LWT";
const char*   willMessage       = "offline";
boolean       willRetain        = false;
byte          willQoS           = 0;

/* ############################ Setup ############################################ */

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  while (WiFi.status() != WL_CONNECTED) {
    debug(".");
    delay(1000);
  }
  debug("IP address:\t");
  debugln(WiFi.localIP());

  mqttClient.setServer( mqtt_server , 1883);
  String MAC = WiFi.macAddress();
  MAC.replace(":", "");
  thingName = "MrDIY_Hub_3_" + MAC;

}

/* ############################ Loop ############################################# */

void loop() {

  mqttReconnect();
  mqttClient.loop();

  if (Serial.available()) {
    received_msg_length = Serial.readBytesUntil('\n', incomingData, sizeof(incomingData));
    if (received_msg_length == sizeof(incomingData)) {  // got a msg from a sensor
      memcpy(&msg, incomingData, sizeof(msg));
      if ( msg.mesh_id == MESH_ID ) sensorMessageOverMQTT();
    }
  }
}

/* ############################ Sensors ############################################# */

void sensorMessageOverMQTT() {

  char macAddr[18];
  sprintf(macAddr, "%02X%02X%02X%02X%02X%02X", msg.sensor_id[0], msg.sensor_id[1], msg.sensor_id[2], msg.sensor_id[3], msg.sensor_id[4], msg.sensor_id[5]);
  DynamicJsonDocument sensor(256);

  if ( msg.category == GROUP_SWITCH) {
    sensor["data"]["category"] = "switch";
    sensor["data"]["status"]  = (int)msg.status ;
  } else if ( msg.category == GROUP_MOTION) {
    sensor["data"]["category"] = "motion";
    sensor["data"]["status"]  = (int)msg.status ;
  } else if ( msg.category == GROUP_HT) {
    sensor["data"]["category"] = "climate";
    sensor["data"]["temperature"]  = getUserUnitTemperature(msg.temperature);
    sensor["data"]["humidity"]  = msg.humidity;
    if (  use_fahrenheit   ) sensor["data"]["unit"]  = "°F";
    else sensor["data"]["unit"]  = "°C";
  }
  sensor["data"]["battery"] = float(int(msg.battery * 100)) / 100;
  char payload[100];
  size_t n = serializeJson(sensor, payload);
  mqttPublish(macAddr, payload, n );

}

/* ################################# MQTT ########################################### */

void mqttReconnect() {

  if ( mqttClient.connected() ) return;
  if (mqttClient.connect( thingName.c_str(), mqtt_username, mqtt_password, "stat/mrdiy_sensors/LWT", willQoS, willRetain, willMessage)) {
    mqttClient.publish("stat/mrdiy_sensors/status", "online");
    mqttClient.publish("stat/mrdiy_sensors/LWT", "online");
    debugln(F("stat/mrdiy_sensors/status ... online "));
  }
}

void mqttPublish(char macAdress[], String payload,  size_t len ) {

  strcpy (mqttTopic, "stat/mrdiy_sensor_");
  strcat (mqttTopic, macAdress);
  strcat (mqttTopic, "/status");
  debug(mqttTopic);
  debug(' ');
  debugln(payload);
  mqttClient.publish(mqttTopic, payload.c_str() , len);
}

/* ################################# Tools ########################################### */

float getUserUnitTemperature(float t) {

  if ( use_fahrenheit  ) return t * 1.8 + 32;
  return t;
}
