// Custom location editor (pages of +/- rows) and a numeric keypad for typing coordinates.
// Included from main.cpp after rowpage.h and sun_screen.h's helpers are declared.
#pragma once

// ---- editor ----
static void lvLat(char* o, int n) { snprintf(o, n, "%.4f %c", fabsf(customLoc.lat), customLoc.lat >= 0 ? 'N' : 'S'); }
static void lvLon(char* o, int n) { snprintf(o, n, "%.4f %c", fabsf(customLoc.lon), customLoc.lon >= 0 ? 'E' : 'W'); }
static void lvUtc(char* o, int n) { snprintf(o, n, "UTC%+d", customLoc.tz.stdOffsetHours); }
static void lvDst(char* o, int n) { snprintf(o, n, "%s", customLoc.tz.usDst ? "on (US rules)" : "off"); }

static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

const Row locRows[] = {
    // page 0: where (+/- nudges 0.01 degrees, about 1 km; tap the value to type it)
    {"LATITUDE (tap to type)", 0, false, [](int d) { customLoc.lat = clampf(roundf((customLoc.lat + d * 0.01f) * 100) / 100, -90, 90); }, lvLat},
    {"LONGITUDE (tap to type)", 0, false, [](int d) { customLoc.lon = clampf(roundf((customLoc.lon + d * 0.01f) * 100) / 100, -180, 180); }, lvLon},
    // page 1: clock rules
    {"UTC OFFSET (STANDARD TIME)", 1, false, [](int d) { customLoc.tz.stdOffsetHours = (int8_t)clampf(customLoc.tz.stdOffsetHours + d, -12, 14); }, lvUtc},
    {"DAYLIGHT SAVING", 1, false, [](int) { customLoc.tz.usDst = !customLoc.tz.usDst; }, lvDst},
};
const char* const locPageNames[] = {"POSITION", "TIME ZONE"};
RowPage locPage = {locRows, sizeof(locRows) / sizeof(locRows[0]), 2, locPageNames, 0, 0};

void drawLocPanel() {
  const uint16_t amber = RGB(255, 170, 40);
  tft.fillRect(0, 0, 240, RP_PANEL_H, C_BG);
  char pg[32], line[48];
  snprintf(pg, sizeof pg, "%d/%d %s", locPage.page + 1, locPage.pages, locPageNames[locPage.page]);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawString("CUSTOM LOCATION", 6, 6, 2);
  tft.drawRightString(pg, 234, 24, 1);
  // preview, so a typo shows up straight away
  int y, m, d;
  bool approx;
  clockToday(y, m, d, approx);
  pc::SunDay s = pc::sunDay(customLoc, y, m, d);
  char a[16], b[16];
  pc::formatClock(s.sunrise, a, sizeof a);
  pc::formatClock(s.sunset, b, sizeof b);
  tft.setTextColor(C_TEXT, C_BG);
  snprintf(line, sizeof line, "%.4f, %.4f", customLoc.lat, customLoc.lon);
  tft.drawCentreString(line, 120, 40, 2);
  tft.setTextColor(amber, C_BG);
  snprintf(line, sizeof line, "Sunrise %s", a);
  tft.drawCentreString(line, 120, 64, 2);
  snprintf(line, sizeof line, "Sunset %s", b);
  tft.drawCentreString(line, 120, 84, 2);
}

void drawLocEdit() {
  tft.fillScreen(C_BG);
  rpDrawRows(locPage);
  drawLocPanel();
  rpDrawButtons();
}

// ---- keypad ----
pc::NumEntry kpEntry;
int kpField = 0;  // 0 = latitude, 1 = longitude
bool kpBad = false;

struct Key { char ch; const char* label; };
const Key KP_KEYS[12] = {{'7', "7"}, {'8', "8"}, {'9', "9"}, {'4', "4"}, {'5', "5"}, {'6', "6"},
                         {'1', "1"}, {'2', "2"}, {'3', "3"}, {'-', "+/-"}, {'0', "0"}, {'.', "."}};
const int KP_X0 = 4, KP_W = 72, KP_GAP = 6, KP_Y0 = 80, KP_H = 48, KP_VGAP = 6, KP_BTN_Y = 280;

void drawKeypadDisplay() {
  tft.fillRect(0, 0, 240, 76, C_BG);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawCentreString(kpField == 0 ? "LATITUDE  (south is negative)" : "LONGITUDE  (west is negative)", 120, 4, 1);
  tft.fillRoundRect(4, 18, 232, 54, 10, C_TILE);
  tft.setTextColor(kpBad ? RGB(255, 90, 70) : C_TEXT, C_TILE);
  char t[20];
  snprintf(t, sizeof t, "%s_", kpEntry.text);
  int font = tft.textWidth(t, 6) <= 216 ? 6 : 4;
  tft.drawCentreString(t, 120, font == 6 ? 22 : 34, font);
  if (kpBad) {
    tft.setTextColor(RGB(255, 90, 70), C_BG);
    tft.drawCentreString(kpField == 0 ? "must be -90 to 90" : "must be -180 to 180", 120, 62 + 12, 1);
  }
}

void drawKeypad() {
  const uint16_t btn = RGB(40, 46, 66), amber = RGB(255, 170, 40);
  tft.fillScreen(C_BG);
  drawKeypadDisplay();
  for (int i = 0; i < 12; i++) {
    int x = KP_X0 + (i % 3) * (KP_W + KP_GAP), y = KP_Y0 + (i / 3) * (KP_H + KP_VGAP);
    tft.fillRoundRect(x, y, KP_W, KP_H, 10, btn);
    tft.setTextColor(C_TEXT, btn);
    tft.drawCentreString(KP_KEYS[i].label, x + KP_W / 2, y + 11, 4);
  }
  tft.fillRoundRect(4, KP_BTN_Y, 72, 40, 10, btn);
  tft.fillRoundRect(82, KP_BTN_Y, 72, 40, 10, btn);
  tft.fillRoundRect(160, KP_BTN_Y, 76, 40, 10, RGB(30, 120, 70));
  tft.setTextColor(C_TEXT, btn);
  tft.drawCentreString("Cancel", 40, KP_BTN_Y + 12, 2);
  tft.drawCentreString("Del", 118, KP_BTN_Y + 12, 2);
  tft.setTextColor(TFT_WHITE, RGB(30, 120, 70));
  tft.drawCentreString("OK", 198, KP_BTN_Y + 12, 2);
}

void startKeypad(int field) {
  kpField = field;
  kpBad = false;
  if (field == 0) pc::neInit(kpEntry, customLoc.lat, -90, 90);
  else pc::neInit(kpEntry, customLoc.lon, -180, 180);
  screen = KEYPAD;
  drawKeypad();
}

// Returns true when the keypad closes (OK with a valid value, or Cancel).
bool keypadTap(int x, int y) {
  if (y >= KP_BTN_Y) {
    if (x < 79) return true;  // cancel
    if (x < 157) {
      pc::neBackspace(kpEntry);
      kpBad = false;
      drawKeypadDisplay();
      return false;
    }
    float v;
    if (pc::neValue(kpEntry, v)) {
      if (kpField == 0) customLoc.lat = v;
      else customLoc.lon = v;
      return true;
    }
    kpBad = true;
    drawKeypadDisplay();
    return false;
  }
  if (y < KP_Y0) return false;
  int col = (x - KP_X0) / (KP_W + KP_GAP), row = (y - KP_Y0) / (KP_H + KP_VGAP);
  if (col < 0 || col > 2 || row < 0 || row > 3) return false;
  pc::neKey(kpEntry, KP_KEYS[row * 3 + col].ch);
  kpBad = false;
  drawKeypadDisplay();
  return false;
}

// ---- editor entry / exit ----
void startLocEdit() {
  if (!customSaved) {  // start from the location being viewed
    const pc::Location& cur = currentLoc();
    customLoc.lat = cur.lat;
    customLoc.lon = cur.lon;
    customLoc.tz = cur.tz;
  }
  locPage.page = 0;
  screen = LOC_EDIT;
  drawLocEdit();
}

// Returns true when the user leaves the editor (saved).
bool locEditTap(int x, int y) {
  switch (rpTap(locPage, x, y)) {
    case RP_HOME:
      saveCustomLoc();
      locIdx = pc::PRESET_COUNT;
      return true;
    case RP_CHANGED: drawLocPanel(); return false;
    case RP_PAGE: drawLocEdit(); return false;
    case RP_CENTER:
      if (locPage.lastRow == 0 || locPage.lastRow == 1) startKeypad(locPage.lastRow);
      return false;
    default: return false;
  }
}
