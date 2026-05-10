#include  "agujas.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN    1      // Pin donde está conectada la tira
#define MINUTE_START_IDX  16 //desde qué indice comienzan los LEDs de los minutos
#define LED_COUNT  MINUTE_START_IDX+60      // Número de LEDs

TaskHandle_t LEDS_handle = NULL;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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

void clear_minutes(void){
  for (int i = MINUTE_START_IDX; i < LED_COUNT; i++) {      //G  R  B
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  // strip.show();
}

void show(int hour, int minute, int second, bool modo){

  static int last_hour = -1;
  static int last_minute = -1;
  static int last_second = -1;

  Serial.printf("Mostrando hora %02d:%02d:%02d\n", hour, minute, second);

  if(last_second == second){
    return;
  }

  last_second = second;

  if(last_hour != hour){
    last_hour = hour;
    setDigit(hour%10, 200, 205, 0);
    setDigit(hour/10, 200, 205, 8);
  }

  if(minute == 0 && second == 0){
    clear_minutes();
    strip.setPixelColor(0 + MINUTE_START_IDX, strip.Color(0, 0, 5));
    strip.show();
    return;
  }

  for(int i = 0; i < 60; i++){
    bool drawMinute = (i <= minute) && (modo || i == minute);
    bool drawSecond = (i == second);
    uint32_t color;

    if(drawMinute && i > 0){
      if(i % 15 == 0){
        color = strip.Color(0, 0, 5);
      }else if(i % 5 == 0){
        color = strip.Color(5, 0, 5);
      }else{
        color = strip.Color(0, 7, 0);
      }
    }else{
      color = strip.Color(0, 0, 0);
    }

    if(drawSecond){
      if(drawMinute){ //segundos y minutos juntos
        if(i % 15 == 0){
        color = strip.Color(0, 0, 10);
      }else if(i % 5 == 0){
        color = strip.Color(10, 0, 10);
      }else{
        color = strip.Color(0, 20, 0);
      }
      }else{ // solo segundos
        if(i % 5 == 0){
          color = strip.Color(0, 0, 5);
        }else{
          color = strip.Color(0, 5, 0);
        }
      }
    }

    strip.setPixelColor(i + MINUTE_START_IDX, color);

  }

    strip.show();
}

void test_sequence(void){
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

void task_leds(void *params){
  strip.begin();
  strip.show();

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(1000);

  while(1){
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    struct tm now;
    if (getLocalTime(&now)) {
      show(now.tm_hour, now.tm_min, now.tm_sec, true);
    }
  }
}