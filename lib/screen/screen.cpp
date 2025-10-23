#if 1
#include <GxEPD2_BW.h>
#include "GxEPD2_290_E97.h"
#include "Fonts/FreeMono9pt7b.h"
#include "Fonts/FreeMonoBold12pt7b.h"

#include "screen.h"
#include "graphics.h"

const int EINK_BUSY = 6;
const int EINK_RST = 7;
const int EINK_DC = 9;
const int EINK_CS = SS;     // 20
const int EINK_SCK = SCK;   // 8
const int EINK_MOSI = MOSI; // 10

// GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT>
// display(GxEPD2_290_T94(EINK_CS, EINK_DC, EINK_RST, EINK_BUSY)); // GDEM029E97 128x296, SSD1675A (SSD1680)

GxEPD2_BW<GxEPD2_290_E97, GxEPD2_290_E97::HEIGHT>
display(GxEPD2_290_E97(EINK_CS, EINK_DC, EINK_RST, EINK_BUSY)); // GDEM029E97 128x296, SSD1675A (SSD1680)

void draw_test1(void) {

  delay(5000);

  display.init(115200);
  // display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.setRotation(1);
  display.display();

  delay(10000);

  display.drawBitmap(271, 5, image_wifi_bits, 19, 16, GxEPD_BLACK);

  display.drawBitmap(2, 60, image_sunny_bits, 64, 64, GxEPD_BLACK);

  display.drawBitmap(155, 57, image_storm_bits, 64, 64, GxEPD_BLACK);

  display.setTextColor(GxEPD_BLACK);
  // display.setFont(&FreeMonoBold12pt7b);
  // display.setTextSize(2);
  // display.setTextWrap(false);
  // display.setCursor(50, 12);
  // display.print("vie 10 oct 2025");

  display.setTextSize(1);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(50, 20);
  display.print("vie 10 oct 2025");
  display.setCursor(65, 79);
  display.print("Ahora");

  display.setCursor(71, 104);
  display.print("23");

  display.setFont(&FreeMono9pt7b);
  display.setCursor(106, 93);
  display.print("25 C");

  display.setCursor(105, 108);
  display.print("10 C");

  display.setCursor(224, 77);
  display.print("sab 11");

  display.setCursor(224, 104);
  display.print("9 / 27");

  display.display();
}
#endif