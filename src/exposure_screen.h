// Exposure screen (portrait 240x320). Included from main.cpp after the theme colors and tft are defined.
// All math lives in lib/PhotoCore/exposure_state; this file only draws and routes taps.
#pragma once

pc::ExposureState ex;
uint8_t exPage = 0;  // 0 = film + metering, 1 = adjustments

struct ExRow {
  const char* label;
  uint8_t page;
  bool smallFont;  // long values (film name)
  void (*stepFn)(pc::ExposureState&, int);
  void (*valueFn)(char*, int);
};

static void vFilm(char* o, int n) { snprintf(o, n, "%s", pc::FILMS[ex.film].name); }
static void vFilmIso(char* o, int n) { snprintf(o, n, "ISO %u", ex.filmIso); }
static void vTime(char* o, int n) { pc::formatTime(pc::SHUTTER_S[ex.timeIdx], o, n); }
static void vMeterAp(char* o, int n) { snprintf(o, n, "f/%g", pc::APERTURES[ex.apIdx]); }
static void vMeterIso(char* o, int n) { snprintf(o, n, "ISO %u", pc::ISOS[ex.meterIsoIdx]); }
static void vShootAp(char* o, int n) {
  if (ex.shootAperture > 0) snprintf(o, n, "f/%g", ex.shootAperture);
  else snprintf(o, n, "same f/%g", pc::APERTURES[ex.apIdx]);
}
static void vFilter(char* o, int n) {
  if (ex.filterThirds == 0) snprintf(o, n, "none");
  else snprintf(o, n, "+%.1f stops", ex.filterThirds / 3.0f);
}
static void vBellows(char* o, int n) {
  if (ex.bellowsMm <= ex.focalMm) snprintf(o, n, "off");
  else snprintf(o, n, "%d mm", (int)ex.bellowsMm);
}
static void vFocal(char* o, int n) { snprintf(o, n, "%d mm", (int)ex.focalMm); }

ExRow exRows[] = {
    // page 0: setup
    {"FILM", 0, true, pc::stepFilm, vFilm},
    {"FILM SPEED (EI)", 0, false, pc::stepFilmIso, vFilmIso},
    {"SHOOT AT APERTURE", 0, false, pc::stepShootAperture, vShootAp},
    // page 1: what the meter said
    {"METERED TIME", 1, false, pc::stepTime, vTime},
    {"METERED AT APERTURE", 1, false, pc::stepAperture, vMeterAp},
    {"METER ISO", 1, false, pc::stepMeterIso, vMeterIso},
    // page 2: compensation
    {"FILTER / ND", 2, false, pc::stepFilter, vFilter},
    {"BELLOWS EXTENSION", 2, false, pc::stepBellows, vBellows},
    {"LENS FOCAL LENGTH", 2, false, pc::stepFocal, vFocal},
};
const int EX_ROWS = sizeof(exRows) / sizeof(exRows[0]);
const int EX_PAGES = 3;
const char* const EX_PAGE_NAMES[] = {"SETUP", "METER", "COMPENSATION"};
// Big targets with clear gaps: 60 px wide x 52 px tall buttons, 6 px gaps; hit zones split at the gap middle.
// Layout: result panel on top (0..110), three rows of buttons, then Home / page buttons.
const int EX_ROW_H = 50, EX_ROW_PITCH = 56, EX_BTN_W = 60, EX_ROWS_Y = 112, EX_PANEL_H = 110, EX_BTN_Y = 280;

static int exRowY(int i) {  // y of row i within its page
  int k = 0;
  for (int j = 0; j < i; j++)
    if (exRows[j].page == exRows[i].page) k++;
  return EX_ROWS_Y + k * EX_ROW_PITCH;
}

void drawExRow(int i) {
  const ExRow& r = exRows[i];
  const uint16_t amber = RGB(255, 170, 40), btn = RGB(40, 46, 66);
  int y = exRowY(i);
  int cx = EX_BTN_W + 6, cw = 240 - 2 * (EX_BTN_W + 6);
  tft.fillRect(0, y, 240, EX_ROW_H, C_BG);
  tft.fillRoundRect(0, y, EX_BTN_W, EX_ROW_H, 10, btn);
  tft.fillRoundRect(240 - EX_BTN_W, y, EX_BTN_W, EX_ROW_H, 10, btn);
  // drawn as shapes: the big built-in font has no "+" glyph
  int my = y + EX_ROW_H / 2, lx = EX_BTN_W / 2, rx = 240 - EX_BTN_W / 2;
  tft.fillRect(lx - 11, my - 2, 22, 5, amber);
  tft.fillRect(rx - 11, my - 2, 22, 5, amber);
  tft.fillRect(rx - 2, my - 11, 5, 22, amber);
  tft.fillRoundRect(cx, y, cw, EX_ROW_H, 10, C_TILE);
  tft.setTextColor(C_DIM, C_TILE);
  tft.drawCentreString(r.label, 120, y + 4, 1);
  char v[32];
  r.valueFn(v, sizeof v);
  tft.setTextColor(C_TEXT, C_TILE);
  int font = 4;
  if (tft.textWidth(v, 4) > cw - 6) font = 2;
  if (tft.textWidth(v, 2) > cw - 6) font = 1;
  tft.drawCentreString(v, 120, y + (font == 4 ? 20 : font == 2 ? 26 : 30), font);
}

void drawExPanel() {
  const uint16_t green = RGB(60, 230, 110), warn = RGB(255, 90, 70);
  pc::ExposureResult r = pc::exposureCompute(ex);
  tft.fillRect(0, 0, 240, EX_PANEL_H, C_BG);
  char t[24], line[40], pg[32];
  pc::formatTime(r.finalTime, t, sizeof t);
  snprintf(pg, sizeof pg, "%d/%d %s", exPage + 1, EX_PAGES, EX_PAGE_NAMES[exPage]);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawString(r.finalTime >= 1.f ? "TAP TO TIME" : "EXPOSURE", 6, 6, 2);
  tft.drawRightString(pg, 234, 10, 1);
  // double-size digits when they fit, otherwise normal size
  int sz = tft.textWidth(t, 4) * 2 <= 230 ? 2 : 1;
  tft.setTextSize(sz);
  tft.setTextColor(green, C_BG);
  tft.drawCentreString(t, 120, sz == 2 ? 24 : 36, 4);
  tft.setTextSize(1);
  snprintf(line, sizeof line, "%+.1f stops", r.totalStops);
  if (r.devPct != 0.f) snprintf(line + strlen(line), sizeof line - strlen(line), "   Dev %+.0f%%", r.devPct);
  tft.setTextColor(C_TEXT, C_BG);
  tft.drawCentreString(line, 120, 78, 2);
  tft.setTextColor(warn, C_BG);
  int wy = 96;
  if (r.colorWarning) {
    tft.drawCentreString("Color film: experimental, expect shift", 120, wy, 1);
    wy += 9;
  }
  if (r.beyondData) tft.drawCentreString("Beyond published data: extrapolated", 120, wy, 1);
}

void drawExButtons() {
  const uint16_t btn = RGB(40, 46, 66);
  tft.fillRoundRect(0, EX_BTN_Y, 80, 40, 10, btn);
  tft.fillRoundRect(86, EX_BTN_Y, 74, 40, 10, btn);
  tft.fillRoundRect(166, EX_BTN_Y, 74, 40, 10, btn);
  tft.setTextColor(C_TEXT, btn);
  tft.drawCentreString("Home", 40, EX_BTN_Y + 12, 2);
  tft.drawCentreString("<", 123, EX_BTN_Y + 8, 4);
  tft.drawCentreString(">", 203, EX_BTN_Y + 8, 4);
}

void drawExposure() {
  tft.fillScreen(C_BG);
  for (int i = 0; i < EX_ROWS; i++)
    if (exRows[i].page == exPage) drawExRow(i);
  drawExPanel();
  drawExButtons();
}

// Returns 0 = nothing, 1 = go home, 2 = start the countdown (tap on the result panel, exposure >= 1 s).
int exposureTap(int x, int y) {
  if (y < EX_PANEL_H) return pc::exposureCompute(ex).finalTime >= 1.f ? 2 : 0;
  if (y >= EX_BTN_Y) {
    if (x < 83) return 1;
    if (x < 163) exPage = (exPage + EX_PAGES - 1) % EX_PAGES;
    else exPage = (exPage + 1) % EX_PAGES;
    drawExposure();
    return 0;
  }
  for (int i = 0; i < EX_ROWS; i++) {
    if (exRows[i].page != exPage) continue;
    int ry = exRowY(i);
    if (y < ry || y >= ry + EX_ROW_H) continue;
    int dir = x < EX_BTN_W + 3 ? -1 : (x >= 240 - EX_BTN_W - 3 ? +1 : 0);
    if (dir == 0) return 0;
    exRows[i].stepFn(ex, dir);
    drawExRow(i);
    if (exRows[i].stepFn == pc::stepFilm) drawExRow(1);  // changing film resets film speed (same page)
    drawExPanel();
    return 0;
  }
  return 0;
}
