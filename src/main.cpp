// Photo Assistant (CYD) - UI SCAFFOLD. NOT yet compiled/tested on hardware.
// The tested logic lives in lib/PhotoCore; this file only draws screens and reads touch.
// Phase 1 screens (see docs/SCREENS.md): Home, Exposure, Pinhole, Sun.
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <time.h>
#include "exposure.h"
#include "solar.h"

// CYD touch is on its own SPI bus
#define T_CS 33
#define T_IRQ 36
#define T_CLK 25
#define T_MOSI 32
#define T_MISO 39

TFT_eSPI tft;
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(T_CS, T_IRQ);

enum Screen { HOME, EXPOSURE, PINHOLE, SUN };
Screen screen = HOME;
uint8_t locIdx = 0;  // index into PRESET_LOCATIONS (custom location: TODO, persist with Preferences)

struct Tile { const char* label; Screen target; int x, y, w, h; };
Tile tiles[] = {
    {"Exposure", EXPOSURE, 10, 40, 145, 90},
    {"Pinhole", PINHOLE, 165, 40, 145, 90},
    {"Sun / Golden", SUN, 10, 140, 145, 90},
};

void drawHome() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Photo Assistant", 10, 10, 4);
  for (auto& t : tiles) {
    tft.fillRoundRect(t.x, t.y, t.w, t.h, 8, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.drawCentreString(t.label, t.x + t.w / 2, t.y + t.h / 2 - 10, 4);
  }
}

void drawSun() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(pc::PRESET_LOCATIONS[locIdx].name, 10, 10, 4);
  time_t now = time(nullptr);  // needs WiFi NTP sync first (TODO)
  struct tm lt;
  localtime_r(&now, &lt);
  pc::SunDay s = pc::sunDay(pc::PRESET_LOCATIONS[locIdx], lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
  char a[16], b[16], c[16], d[16];
  pc::formatClock(s.sunrise, a, 16);
  pc::formatClock(s.goldenEveningStart, b, 16);
  pc::formatClock(s.sunset, c, 16);
  pc::formatClock(s.blueEveningEnd, d, 16);
  tft.drawString(String("Sunrise  ") + a, 10, 60, 4);
  tft.drawString(String("Golden   ") + b, 10, 100, 4);
  tft.drawString(String("Sunset   ") + c, 10, 140, 4);
  tft.drawString(String("Blue end ") + d, 10, 180, 4);
}

void drawPlaceholder(const char* title) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(title, 10, 10, 4);
  tft.drawString("TODO - see docs/SCREENS.md", 10, 60, 2);
  tft.drawString("Tap anywhere to go home", 10, 90, 2);
}

void go(Screen s) {
  screen = s;
  if (s == HOME) drawHome();
  else if (s == SUN) drawSun();
  else if (s == EXPOSURE) drawPlaceholder("Exposure");
  else drawPlaceholder("Pinhole");
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.init();
  tft.setRotation(1);  // landscape 320x240
  touchSPI.begin(T_CLK, T_MISO, T_MOSI, T_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  Serial.printf("Films loaded: %u\n", pc::FILM_COUNT);
  go(HOME);
}

void loop() {
  static uint32_t lastTap = 0;
  if (ts.tirqTouched() && ts.touched() && millis() - lastTap > 250) {
    lastTap = millis();
    TS_Point p = ts.getPoint();
    // raw touch -> screen coords: calibrate these ranges for your unit
    int x = map(p.x, 200, 3700, 0, 320);
    int y = map(p.y, 240, 3800, 0, 240);
    if (screen == HOME) {
      for (auto& t : tiles)
        if (x >= t.x && x < t.x + t.w && y >= t.y && y < t.y + t.h) go(t.target);
    } else {
      go(HOME);
    }
  }
}
