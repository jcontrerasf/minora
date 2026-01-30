#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>

#include "screen.h"

#define LED_PIN    1      // Pin donde está conectada la tira
#define LED_COUNT  16+60      // Número de LEDs

AsyncWebServer server(80);
DNSServer dnsServer;
Preferences prefs;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

String wifi_ssid = "";
String wifi_pass = "";
bool wifi_connecting = false;
bool wifi_connected = false;
unsigned long wifi_connect_start = 0;

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Desconectado, intentando reconectar...");
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("Conectado al WiFi");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    break;
  }
}


void syncTimeFromNTP() {
  const long gmtOffset_sec = -3 * 3600;   // Chile (GMT-3)
  const int daylightOffset_sec = 0;       // Ajuste horario si aplica (1*3600 en verano)

  configTime(gmtOffset_sec, daylightOffset_sec, "ntp.shoa.cl", "pool.ntp.org");

  Serial.println("Sincronizando hora con NTP...");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.printf("Hora sincronizada: %02d:%02d:%02d - %02d/%02d/%04d\n",
      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
      timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  } else {
    Serial.println("No se pudo obtener la hora del servidor NTP");
  }
}

void setup() {

  Serial.begin(115200);

  // delay(10000);

  Serial.println("Iniciando reloj minora");

  if (!SPIFFS.begin(true)) {
    Serial.println("Error montando SPIFFS");
    return;
  }

  prefs.begin("wifi", true);
  String saved_ssid = prefs.getString("ssid", "");
  String saved_pass = prefs.getString("pass", "");
  prefs.end();

  if (saved_ssid != "") {
    WiFi.mode(WIFI_STA);
    Serial.printf("Conectando a red guardada: %s\n", saved_ssid.c_str());
    WiFi.begin(saved_ssid.c_str(), saved_pass.c_str());
    //Si falla n veces, iniciar AP
  } else {
    WiFi.mode(WIFI_AP_STA);
    Serial.println("No hay credenciales guardadas, iniciando AP");
    WiFi.softAP("minora");
  }

  IPAddress apIP(192,168,4,1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  dnsServer.start(53, "*", apIP);

  // WiFi.onEvent(WiFiEvent);
  // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   Serial.println("WiFi Failed!");
  //   return;
  // }
  // Serial.println();
  // Serial.print("IP Address: ");
  // Serial.println(WiFi.localIP());

  if (!MDNS.begin("minora")) {
    Serial.println("Error iniciando mDNS");
  }
  Serial.println("mDNS iniciado");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/creds.html", "text/html");
  });

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, data);
      if (err) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"JSON invalido\"}");
        return;
      }

      wifi_ssid = doc["ssid"].as<String>();
      wifi_pass = doc["password"].as<String>();

      Serial.printf("Recibido SSID: %s, PASS: %s\n", wifi_ssid.c_str(), wifi_pass.c_str());

      // Iniciar intento de conexión
      WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
      wifi_connecting = true;
      wifi_connected = false;
      wifi_connect_start = millis();

      request->send(200, "application/json", "{\"status\":\"received\"}");
  });

  server.on("/wifi-status", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;

    if (wifi_connecting) {
      wl_status_t status = WiFi.status();

      if (status == WL_CONNECTED) {
        prefs.begin("wifi", false);
        prefs.putString("ssid", wifi_ssid);
        prefs.putString("pass", wifi_pass);
        prefs.end();
        wifi_connecting = false;
        wifi_connected = true;
        doc["status"] = "connected";
        doc["ip"] = WiFi.localIP().toString();
      } else if (millis() - wifi_connect_start > 15000) { // timeout
        wifi_connecting = false;
        wifi_connected = false;
        doc["status"] = "failed";
      } else {
        doc["status"] = "connecting";
      }
    } else {
      doc["status"] = wifi_connected ? "connected" : "idle";
    }

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  server.onNotFound(notFound);
  server.begin();

  strip.begin();
  strip.show();

  syncTimeFromNTP();

  draw_test1();
}
                    //  ABCDEFGH
// uint8_t digitos[10] = {B11111100, //0
//                        B01100001, //1
//                        B11011010, //2
//                        B11110010, //3
//                        B01100110, //4
//                        B10110110, //5
//                        B10111110, //6
//                        B11100000, //7
//                        B11111110, //8
//                        B11110110, //9
//                        };

                  //    ABFGHCDE
uint8_t digitos[10] = {B11100111, //0
                       B01001100, //1
                       B11010011, //2
                       B11010110, //3
                       B01110100, //4
                       B10110110, //5
                       B10110111, //6
                       B11000100, //7
                       B11110111, //8
                       B11110110, //9
                       };


void setDigit(int digit, uint8_t brillo, uint8_t brillo_1, uint8_t inicio){
  uint8_t br;
  for (int i = 0; i < 8; i++){
    if(i == 4){ //corresponde al segmento H, que tiene un LED más que los otros segmentos
      br = brillo_1;
    }else{
      br = brillo;
    }
    strip.setPixelColor(inicio + i, ((digitos[digit] >> (7 - i)) & 1) ? strip.Color(br, br, br) : 0);
  }
  // strip.show();
}


void loop() {
  dnsServer.processNextRequest();

  for (int i = 0; i < 60; i++) {
    Serial.printf("-------Mostrando digito %d-------\n", i);
    setDigit(i%10, 200, 205, 0);
    setDigit(i/10, 200, 205, 8);

    if(i % 15 == 0){
      strip.setPixelColor(i+16, strip.Color(0, 0, 5));
    }else if(i % 5 == 0){
      strip.setPixelColor(i+16, strip.Color(5, 0, 5));
    }else{
      strip.setPixelColor(i+16, strip.Color(0, 10, 0));
    }
    strip.show();
    delay(1000);
  }

  for (int i = 16; i < LED_COUNT; i++) {      //G  R  B
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();

}