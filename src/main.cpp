#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

#include "screen.h"
#include "webapp.h"
#include "agujas.h"


bool got_ip = false;

EventGroupHandle_t wifi_event_group;
#define WIFI_GOT_IP_BIT BIT0


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
    got_ip = true;
    xEventGroupSetBits(wifi_event_group, WIFI_GOT_IP_BIT);
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



TaskHandle_t screen_handle = NULL;
void task_screen(void *params){
  draw_test1();
  vTaskDelete(NULL);
}

void setup() {

  Serial.begin(115200);
  wifi_event_group = xEventGroupCreate();
  WiFi.onEvent(WiFiEvent);

  delay(10000);

  Serial.println("Iniciando reloj minora");
  ESP_LOGI("setup", "prueba");

  if (!SPIFFS.begin(true)) {
    Serial.println("Error montando SPIFFS");
    return;
  }

  xTaskCreate(task_leds, "LEDS", configMINIMAL_STACK_SIZE*4, NULL, 2, &LEDS_handle);
  xTaskCreate(task_screen, "Screen", configMINIMAL_STACK_SIZE*4, NULL, 3, &screen_handle);

  webapp_check_creds();

  webapp_init();

  xEventGroupWaitBits(
    wifi_event_group,
    WIFI_GOT_IP_BIT,
    pdFALSE,
    pdTRUE,
    portMAX_DELAY
  );

  syncTimeFromNTP();
}


void loop() {
  // dnsServer.processNextRequest();

}