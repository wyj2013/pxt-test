#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include "secrets.h"

const char* MQTT_SERVER = "mqttgo.io";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "phmhs/aqi";

// PMS5003 TXD -> D6/GPIO12；PMS5003 RXD -> D5/GPIO14（本程式不需使用 RXD）
SoftwareSerial pmsSerial(D6, D5);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000;

struct SensorData {
  uint16_t pm01;
  uint16_t pm25;
  uint16_t pm10;
  float temperature;
  float humidity;
};

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "HW628-" + String(ESP.getChipId(), HEX);
    Serial.print("Connecting to MQTT...");

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, state=");
      Serial.print(mqttClient.state());
      Serial.println("; retry in 2 seconds");
      delay(2000);
    }
  }
}

uint16_t wordAt(const uint8_t* frame, uint8_t index) {
  return ((uint16_t)frame[index] << 8) | frame[index + 1];
}

bool readPMS5003(SensorData& data) {
  const uint8_t requestRead[] = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};
  uint8_t frame[32];

  // PMS5003 被動模式下，必須每次主動要求一筆資料。
  while (pmsSerial.available()) pmsSerial.read();
  pmsSerial.write(requestRead, sizeof(requestRead));
  delay(40);

  unsigned long start = millis();
  while (millis() - start < 1200) {
    if (!pmsSerial.available()) {
      yield();
      continue;
    }
    if (pmsSerial.read() != 0x42) continue;
    if (!pmsSerial.available() || pmsSerial.read() != 0x4D) continue;

    frame[0] = 0x42;
    frame[1] = 0x4D;
    size_t received = pmsSerial.readBytes(frame + 2, 30);
    if (received != 30) return false;
    if (wordAt(frame, 2) != 28) return false;

    uint16_t checksum = 0;
    for (uint8_t i = 0; i < 30; i++) checksum += frame[i];
    if (checksum != wordAt(frame, 30)) return false;

    // PMS5003 大氣環境數值：PM1.0、PM2.5、PM10
    data.pm01 = wordAt(frame, 10);
    data.pm25 = wordAt(frame, 12);
    data.pm10 = wordAt(frame, 14);
    // PMS5003 沒有溫度與濕度感測功能
    data.temperature = -1;
    data.humidity = -1;
    return true;
  }

  return false;
}

bool publishSensorData() {
  SensorData data;
  char payload[160];
  if (readPMS5003(data)) {
    snprintf(payload, sizeof(payload),
             "{\"pm01\":%u,\"pm25\":%u,\"pm10\":%u,\"temperature\":%.1f,\"humidity\":%.1f}",
             data.pm01, data.pm25, data.pm10, data.temperature, data.humidity);
  } else {
    snprintf(payload, sizeof(payload),
             "{\"pm01\":-1,\"pm25\":-1,\"pm10\":-1,\"temperature\":-1,\"humidity\":-1}");
    Serial.println("PMS5003 data not ready; publishing -1 values");
  }

  if (!mqttClient.publish(MQTT_TOPIC, payload)) {
    Serial.println("MQTT publish failed");
    return false;
  }

  Serial.print("Published to ");
  Serial.print(MQTT_TOPIC);
  Serial.print(": ");
  Serial.println(payload);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  return true;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  pmsSerial.begin(9600);
  pmsSerial.setTimeout(150);
  delay(100);

  // 切換 PMS5003 為被動模式，之後由程式逐筆要求資料。
  const uint8_t passiveMode[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
  pmsSerial.write(passiveMode, sizeof(passiveMode));
  delay(100);

  connectWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
  lastPublish = millis() - PUBLISH_INTERVAL;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  if (millis() - lastPublish >= PUBLISH_INTERVAL) {
    publishSensorData();
    lastPublish = millis();
  }
}
