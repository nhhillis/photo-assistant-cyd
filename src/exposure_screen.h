// Exposure screen (portrait 240x320). Included from main.cpp after rowpage.h.
// All math lives in lib/PhotoCore/exposure_state; this file only draws and routes taps.
#pragma once

pc::ExposureState ex;

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

const Row exRows[] = {
    // page 0: setup
    {"FILM", 0, true, [](int d) { pc::stepFilm(ex, d); }, vFilm},
    {"FILM SPEED (EI)", 0, false, [](int d) { pc::stepFilmIso(ex, d); }, vFilmIso},
    {"SHOOT AT APERTURE", 0, false, [](int d) { pc::stepShootAperture(ex, d); }, vShootAp},
    // page 1: what the meter said
    {"METERED TIME", 1, false, [](int d) { pc::stepTime(ex, d); }, vTime},
    {"METERED AT APERTURE", 1, false, [](int d) { pc::stepAperture(ex, d); }, vMeterAp},
    {"METER ISO", 1, false, [](int d) { pc::stepMeterIso(ex, d); }, vMeterIso},
    // page 2: compensation
    {"FILTER / ND", 2, false, [](int d) { pc::stepFilter(ex, d); }, vFilter},
    {"BELLOWS EXTENSION", 2, false, [](int d) { pc::stepBellows(ex, d); }, vBellows},
    {"LENS FOCAL LENGTH", 2, false, [](int d) { pc::stepFocal(ex, d); }, vFocal},
};
const char* const exPageNames[] = {"SETUP", "METER", "COMPENSATION"};
RowPage exPage = {exRows, sizeof(exRows) / sizeof(exRows[0]), 3, exPageNames, 0};

void drawExPanel() {
  const uint16_t green = RGB(60, 230, 110), warn = RGB(255, 90, 70);
  pc::ExposureResult r = pc::exposureCompute(ex);
  tft.fillRect(0, 0, 240, RP_PANEL_H, C_BG);
  char t[24], line[40], pg[32];
  pc::formatTime(r.finalTime, t, sizeof t);
  snprintf(pg, sizeof pg, "%d/%d %s", exPage.page + 1, exPage.pages, exPageNames[exPage.page]);
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

void drawExposure() {
  tft.fillScreen(C_BG);
  rpDrawRows(exPage);
  drawExPanel();
  rpDrawButtons();
}

// Returns 0 = nothing, 1 = go home, 2 = start the countdown (tap on the result panel, exposure >= 1 s).
int exposureTap(int x, int y) {
  switch (rpTap(exPage, x, y)) {
    case RP_HOME: return 1;
    case RP_PANEL: return pc::exposureCompute(ex).finalTime >= 1.f ? 2 : 0;
    case RP_CHANGED: drawExPanel(); return 0;
    case RP_PAGE: drawExposure(); return 0;
    default: return 0;
  }
}
