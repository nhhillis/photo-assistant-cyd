// Sun / golden hour screen (portrait 240x320). Included from main.cpp after clock_sync.h.
// Times come from pc::sunDay (tested on the PC); the date comes from the synced clock.
#pragma once

static const int SUN_BTN_Y = 280;
void startLocEdit();  // location_screens.h

void drawSun() {
  const uint16_t amber = RGB(255, 170, 40), blue = RGB(110, 150, 255), btn = RGB(40, 46, 66);
  tft.fillScreen(C_BG);
  // header: [<]  location  [>]
  tft.fillRoundRect(0, 0, 52, 44, 10, btn);
  tft.fillRoundRect(188, 0, 52, 44, 10, btn);
  tft.fillRoundRect(58, 0, 124, 44, 10, C_TILE);
  tft.setTextColor(amber, btn);
  tft.drawCentreString("<", 26, 9, 4);
  tft.drawCentreString(">", 214, 9, 4);
  tft.setTextColor(C_TEXT, C_TILE);
  const pc::Location& loc = currentLoc();
  tft.drawCentreString(loc.name, 120, tft.textWidth(loc.name, 4) <= 116 ? 10 : 14, tft.textWidth(loc.name, 4) <= 116 ? 4 : 2);

  int y, m, d;
  bool approx;
  clockToday(y, m, d, approx);
  char line[48];
  if (approx) {
    snprintf(line, sizeof line, "%d-%02d-%02d  clock not synced, date approx", y, m, d);
    tft.setTextColor(RGB(255, 90, 70), C_BG);
  } else {
    snprintf(line, sizeof line, "%d-%02d-%02d", y, m, d);
    tft.setTextColor(C_DIM, C_BG);
  }
  tft.drawCentreString(line, 120, 52, approx ? 1 : 2);

  pc::SunDay s = pc::sunDay(loc, y, m, d);
  struct R { const char* label; pc::Ev ev; uint16_t col; };
  R rows[] = {
      {"Civil dawn", s.civilDawn, C_TEXT},
      {"Sunrise", s.sunrise, amber},
      {"Golden hour ends", s.goldenMorningEnd, amber},
      {"Golden hour starts", s.goldenEveningStart, amber},
      {"Sunset", s.sunset, amber},
      {"Blue hour ends", s.blueEveningEnd, blue},
  };
  int ry = 76;
  for (auto& r : rows) {
    char t[16];
    pc::formatClock(r.ev, t, sizeof t);
    tft.fillRoundRect(0, ry, 240, 32, 8, C_TILE);
    tft.setTextColor(C_DIM, C_TILE);
    tft.drawString(r.label, 8, ry + 8, 2);
    tft.setTextColor(r.col, C_TILE);
    tft.drawRightString(t, 232, ry + 4, 4);
    ry += 34;
  }
  tft.fillRoundRect(0, SUN_BTN_Y, 116, 40, 10, btn);
  tft.fillRoundRect(124, SUN_BTN_Y, 116, 40, 10, btn);
  tft.setTextColor(C_TEXT, btn);
  tft.drawCentreString("Home", 58, SUN_BTN_Y + 12, 2);
  tft.drawCentreString("Custom...", 182, SUN_BTN_Y + 12, 2);
}

// Returns true when the user leaves the screen.
bool sunTap(int x, int y) {
  if (y >= SUN_BTN_Y) {
    if (x < 120) return true;
    startLocEdit();
    return false;
  }
  if (y < 44) {
    int n = locCount();
    if (x < 56) locIdx = (locIdx + n - 1) % n;
    else if (x >= 184) locIdx = (locIdx + 1) % n;
    else return false;
    drawSun();
  }
  return false;
}
