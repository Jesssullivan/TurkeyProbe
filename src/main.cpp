#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <thermistor.h>
#include "HardwareSerial.h"

// WiFi Password
const char* ssid = "";
const char* password = "";

// hardware pins:
#define TURKEY1_NTC_PIN A0

// Thermistor object
THERMISTOR thermistor(TURKEY1_NTC_PIN,       
                      100000,          // Nominal resistance at 25 ºC
                      4300,           // thermistor's beta coefficient
                      10000);         // Value of the pullup resistor

// Global temperature reading
uint16_t temp;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// update speed:
unsigned long lastTime = 0;  
unsigned long timerDelay = 500;

// read temps from probes:
String getSensorReadings() {
  temp = thermistor.read() / 10;   // Read temperature
  char probeJson[56];

// JS "tempurature" element key we'll use to update temp readings with:
  sprintf(probeJson, "{\"tempurature\":%hu}", temp);

  return probeJson; 
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
   Serial.println("LittleFS mounted OwO");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      notifyClients(getSensorReadings());
  }
}

// minimally handle webSocket events:
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      // Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  // Serial.begin(115200);
  initWiFi();
  initFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    
    Serial.print(getSensorReadings());

    notifyClients(getSensorReadings());

  lastTime = millis();

  }

  ws.cleanupClients();
}