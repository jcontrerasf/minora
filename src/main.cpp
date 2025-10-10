#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

AsyncWebServer server(80);
DNSServer dnsServer;
Preferences prefs;

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

void setup() {
  Serial.begin(115200);

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
}

void loop() {
  dnsServer.processNextRequest();
}