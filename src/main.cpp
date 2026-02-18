#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Adafruit_NeoPixel.h>

#include "screen.h"
#include "webapp.h"

#define LED_PIN    1      // Pin donde está conectada la tira
#define LED_COUNT  16+60      // Número de LEDs


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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

TaskHandle_t LEDS_handle = NULL;
void task_leds(void *params){
  strip.begin();
  strip.show();
  while(1){
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
  Serial.println("Tiene IP");
  syncTimeFromNTP();
}


void loop() {
  // dnsServer.processNextRequest();

}