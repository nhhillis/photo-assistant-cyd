// Photo Assistant (CYD) - UI SCAFFOLD. NOT yet compiled/tested on hardware.
// The tested logic lives in lib/PhotoCore; this file only draws screens and reads touch.
// Phase 1 screens (see docs/SCREENS.md): Home, Exposure, Pinhole, Sun.
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Preferences.h>
#include <time.h>
#include "exposure.h"
#include "exposure_state.h"
#include "solar.h"
#include <string.h>
#include "countdown.h"
#include "pinhole_state.h"
#include "clock_sync.h"

// CYD touch is on its own SPI bus
#define T_CS 33
#define T_IRQ 36
#define T_CLK 25
#define T_MOSI 32
#define T_MISO 39

TFT_eSPI tft;
XPT2046_Touchscreen ts(T_CS, T_IRQ);

enum Screen { HOME, EXPOSURE, PINHOLE, SUN, COUNTDOWN };
Screen screen = HOME;
uint8_t locIdx = 0;  // index into PRESET_LOCATIONS (custom location: TODO, persist with Preferences)

// ---- Home screen look: dark navy, accent-colored tiles with drawn icons ----
enum Icon { ICON_APERTURE, ICON_PINHOLE, ICON_SUN, ICON_METER };

struct Tile {
  const char* label;
  const char* sub;
  Screen target;
  Icon icon;
  uint16_t accent;  // RGB565
  bool enabled;
  int x, y, w, h;
};

#define RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
const uint16_t C_BG = RGB(8, 10, 18);
const uint16_t C_TILE = RGB(22, 26, 40);
const uint16_t C_TEXT = RGB(235, 238, 245);
const uint16_t C_DIM = RGB(120, 128, 150);

Tile tiles[] = {
    {"Exposure", "reciprocity", EXPOSURE, ICON_APERTURE, RGB(255, 170, 40), true, 8, 76, 108, 106},
    {"Pinhole", "f-number & time", PINHOLE, ICON_PINHOLE, RGB(70, 200, 230), true, 124, 76, 108, 106},
    {"Sun / Golden", "light times", SUN, ICON_SUN, RGB(255, 110, 90), true, 8, 190, 108, 106},
    {"Light Meter", "coming soon", HOME, ICON_METER, RGB(80, 86, 105), false, 124, 190, 108, 106},
};

void drawIcon(Icon ic, int cx, int cy, uint16_t col, uint16_t bg) {
  const int r = 20;
  switch (ic) {
    case ICON_APERTURE: {  // iris: outer ring, hexagon, six blade lines
      tft.drawCircle(cx, cy, r, col);
      tft.drawCircle(cx, cy, r - 1, col);
      float hx[6], hy[6];
      for (int i = 0; i < 6; i++) {
        float ang = i * 1.0472f + 0.26f;
        hx[i] = cx + cosf(ang) * 9;
        hy[i] = cy + sinf(ang) * 9;
      }
      for (int i = 0; i < 6; i++) {
        int j = (i + 1) % 6;
        tft.drawLine(hx[i], hy[i], hx[j], hy[j], col);
        float ang = i * 1.0472f + 0.26f + 1.9f;
        tft.drawLine(hx[i], hy[i], cx + cosf(ang) * (r - 2), cy + sinf(ang) * (r - 2), col);
      }
      break;
    }
    case ICON_PINHOLE: {  // light rays crossing a tiny hole in a wall
      tft.fillRect(cx - 2, cy - r, 4, r - 2, col);
      tft.fillRect(cx - 2, cy + 3, 4, r - 3, col);
      tft.drawLine(cx - r, cy - r + 4, cx, cy, col);
      tft.drawLine(cx - r, cy, cx, cy, col);
      tft.drawLine(cx - r, cy + r - 4, cx, cy, col);
      tft.drawLine(cx, cy, cx + r, cy + r - 4, col);
      tft.drawLine(cx, cy, cx + r, cy - r + 4, col);
      tft.drawLine(cx, cy, cx + r, cy, col);
      tft.fillCircle(cx, cy, 2, TFT_WHITE);
      break;
    }
    case ICON_SUN: {  // half sun on the horizon with rays
      tft.fillCircle(cx, cy + 6, 10, col);
      for (int i = 0; i < 7; i++) {
        float ang = 3.14159f + i * 0.5236f;  // upper half-plane
        tft.drawLine(cx + cosf(ang) * 14, cy + 6 + sinf(ang) * 14,
                     cx + cosf(ang) * 20, cy + 6 + sinf(ang) * 20, col);
      }
      tft.fillRect(cx - r - 4, cy + 7, 2 * r + 8, 14, bg);
      tft.drawFastHLine(cx - r - 2, cy + 7, 2 * r + 4, col);
      tft.drawFastHLine(cx - r + 6, cy + 12, 2 * r - 12, col);
      break;
    }
    case ICON_METER: {  // simple dial
      tft.drawCircle(cx, cy + 4, r - 2, col);
      tft.drawLine(cx, cy + 4, cx + 9, cy - 8, col);
      tft.fillCircle(cx, cy + 4, 2, col);
      break;
    }
  }
}

void drawTile(const Tile& t, bool pressed) {
  uint16_t bg = pressed ? t.accent : C_TILE;
  uint16_t fg = pressed ? C_BG : C_TEXT;
  uint16_t sub = pressed ? C_BG : C_DIM;
  uint16_t ic = pressed ? C_BG : t.accent;
  tft.fillRoundRect(t.x, t.y, t.w, t.h, 10, bg);
  tft.drawRoundRect(t.x, t.y, t.w, t.h, 10, t.accent);
  if (!pressed) tft.fillRoundRect(t.x, t.y + 10, 5, t.h - 20, 2, t.accent);  // accent stripe
  drawIcon(t.icon, t.x + t.w / 2, t.y + 36, ic, bg);
  tft.setTextColor(t.enabled ? fg : C_DIM, bg);
  tft.drawCentreString(t.label, t.x + t.w / 2, t.y + 66, 2);
  tft.setTextColor(sub, bg);
  tft.drawCentreString(t.sub, t.x + t.w / 2, t.y + 86, 1);
}

void drawHomeFooter() {
  char t[40];
  clockStatusText(t, sizeof t);
  tft.fillRect(0, 300, 240, 20, C_BG);
  tft.setTextColor(ckState == CK_FAILED ? RGB(255, 90, 70) : C_DIM, C_BG);
  tft.drawString(t, 8, 306, 1);
  tft.drawRightString("v0.1", 232, 306, 1);
}

void drawHome() {
  tft.fillScreen(C_BG);
  // header: mini lens mark, title, location chip
  tft.drawCircle(20, 18, 11, RGB(255, 170, 40));
  tft.drawCircle(20, 18, 5, RGB(255, 170, 40));
  tft.setTextColor(C_TEXT, C_BG);
  tft.drawString("PHOTO ASSISTANT", 40, 6, 2);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawString("pinhole / large format", 40, 24, 1);
  const char* loc = pc::PRESET_LOCATIONS[locIdx].name;
  tft.fillRoundRect(8, 44, 224, 24, 12, C_TILE);
  tft.setTextColor(C_TEXT, C_TILE);
  tft.drawCentreString(loc, 120, 48, 2);
  tft.drawFastHLine(8, 71, 224, RGB(40, 46, 66));
  for (auto& t : tiles) drawTile(t, false);
  drawHomeFooter();
}

// ---- Touch calibration: two corner taps, saved to flash (hold a finger on the screen at boot to redo) ----
struct Cal { int x1, y1, x2, y2; bool ok; };  // raw values at screen points (CAL_A) and (CAL_B)
Cal cal = {0, 0, 0, 0, false};
Preferences prefs;
const int CAL_AX = 20, CAL_AY = 20, CAL_BX = 220, CAL_BY = 300;

void drawCross(int x, int y) {
  tft.drawFastHLine(x - 12, y, 25, TFT_WHITE);
  tft.drawFastVLine(x, y - 12, 25, TFT_WHITE);
  tft.drawCircle(x, y, 6, TFT_RED);
}

void tapRaw(int sx, int sy, int& rx, int& ry) {
  tft.fillScreen(C_BG);
  tft.setTextColor(C_TEXT, C_BG);
  tft.drawCentreString("Touch calibration", 120, 130, 2);
  tft.drawCentreString("Tap the crosshair", 120, 154, 2);
  drawCross(sx, sy);
  while (ts.touched()) delay(10);  // wait for release
  delay(150);
  while (!ts.touched()) delay(10);
  long ax = 0, ay = 0; int n = 0;
  uint32_t t0 = millis();
  while (ts.touched() && millis() - t0 < 400) {  // average while the finger is down
    TS_Point p = ts.getPoint();
    ax += p.x; ay += p.y; n++;
    delay(10);
  }
  rx = n ? ax / n : 0; ry = n ? ay / n : 0;
  Serial.printf("cal tap (%d,%d) raw (%d,%d) n=%d\n", sx, sy, rx, ry, n);
}

void runCalibration() {
  tapRaw(CAL_AX, CAL_AY, cal.x1, cal.y1);
  tapRaw(CAL_BX, CAL_BY, cal.x2, cal.y2);
  cal.ok = cal.x1 != cal.x2 && cal.y1 != cal.y2;
  if (cal.ok) {
    prefs.putBytes("cal", &cal, sizeof(cal));
    Serial.printf("cal saved: %d %d %d %d\n", cal.x1, cal.y1, cal.x2, cal.y2);
  }
}

void loadCalibration() {
  prefs.begin("touch", false);
  if (prefs.getBytesLength("cal") != sizeof(cal)) return;
  prefs.getBytes("cal", &cal, sizeof(cal));
}

void toScreen(int rx, int ry, int& x, int& y) {
  x = CAL_AX + (float)(rx - cal.x1) * (CAL_BX - CAL_AX) / (cal.x2 - cal.x1);
  y = CAL_AY + (float)(ry - cal.y1) * (CAL_BY - CAL_AY) / (cal.y2 - cal.y1);
}

#include "rowpage.h"
#include "exposure_screen.h"
#include "pinhole_screen.h"
#include "countdown_screen.h"
#include "sun_screen.h"

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
  else if (s == EXPOSURE) drawExposure();
  else if (s == PINHOLE) drawPinhole();
  else drawPlaceholder("Pinhole");
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.init();
  tft.setRotation(0);  // portrait 240x320
  SPI.begin(T_CLK, T_MISO, T_MOSI, T_CS);  // touch owns the default (VSPI) bus; display uses HSPI
  ts.begin();
  ts.setRotation(0);
  pc::exposureInit(ex);
  pc::pinholeInit(ph);
  loadCalibration();
  delay(100);
  if (!cal.ok || ts.touched()) runCalibration();
  Serial.printf("Films loaded: %u\n", pc::FILM_COUNT);
  clockBegin();  // WiFi + NTP in the background; the Home footer shows progress
  go(HOME);
}

void loop() {
  if (clockTick()) {  // sync state changed: refresh whatever shows the clock
    if (screen == HOME) drawHomeFooter();
    else if (screen == SUN) drawSun();
  }
  if (screen == COUNTDOWN) countdownTick();
  static uint32_t lastTap = 0;
  if (ts.tirqTouched() && ts.touched() && millis() - lastTap > 250) {
    lastTap = millis();
    TS_Point p = ts.getPoint();
    int x, y;
    toScreen(p.x, p.y, x, y);
    if (screen == HOME) {
      if (y >= 298 && ckState == CK_FAILED) {  // tap the footer to retry a failed sync
        clockBegin();
        drawHomeFooter();
      }
      for (auto& t : tiles)
        if (t.enabled && x >= t.x && x < t.x + t.w && y >= t.y && y < t.y + t.h) {
          drawTile(t, true);
          delay(120);
          go(t.target);
        }
    } else if (screen == EXPOSURE) {
      int act = exposureTap(x, y);
      if (act == 1) go(HOME);
      else if (act == 2) startCountdownScreen(pc::exposureCompute(ex).finalTime, pc::FILMS[ex.film].name, EXPOSURE);
    } else if (screen == COUNTDOWN) {
      if (countdownTap(x, y)) go(cdReturn);
    } else if (screen == PINHOLE) {
      int act = pinholeTap(x, y);
      if (act == 1) go(HOME);
      else if (act == 2) startCountdownScreen(pc::pinholeCompute(ph).exp.finalTime, pc::FILMS[ph.ex.film].name, PINHOLE);
    } else if (screen == SUN) {
      if (sunTap(x, y)) go(HOME);
    } else {
      go(HOME);
    }
  }
}
