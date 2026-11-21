// Pinhole screen (portrait 240x320). Included from main.cpp after rowpage.h.
// All math lives in lib/PhotoCore/pinhole_state; this file only draws and routes taps.
#pragma once

pc::PinholeState ph;

static void pvPreset(char* o, int n) {
  if (pc::pinholeMatchesPreset(ph)) snprintf(o, n, "%s", pc::PINHOLE_PRESETS[ph.preset].name);
  else snprintf(o, n, "custom");
}
static void pvFocal(char* o, int n) { snprintf(o, n, "%d mm", (int)ph.focalMm); }
static void pvHole(char* o, int n) { snprintf(o, n, "%.2f mm", ph.holeMm); }
static void pvFilm(char* o, int n) { snprintf(o, n, "%s", pc::FILMS[ph.ex.film].name); }
static void pvFilmIso(char* o, int n) { snprintf(o, n, "ISO %u", ph.ex.filmIso); }
static void pvTime(char* o, int n) { pc::formatTime(pc::SHUTTER_S[ph.ex.timeIdx], o, n); }
static void pvMeterAp(char* o, int n) { snprintf(o, n, "f/%g", pc::APERTURES[ph.ex.apIdx]); }
static void pvMeterIso(char* o, int n) { snprintf(o, n, "ISO %u", pc::ISOS[ph.ex.meterIsoIdx]); }
static void pvFilter(char* o, int n) {
  if (ph.ex.filterThirds == 0) snprintf(o, n, "none");
  else snprintf(o, n, "+%.1f stops", ph.ex.filterThirds / 3.0f);
}

const Row phRows[] = {
    // page 0: the pinhole
    {"PINHOLE PRESET", 0, true, [](int d) { pc::pinholeStepPreset(ph, d); }, pvPreset},
    {"FOCAL LENGTH", 0, false, [](int d) { pc::pinholeStepFocal(ph, d); }, pvFocal},
    {"HOLE DIAMETER", 0, false, [](int d) { pc::pinholeStepHole(ph, d); }, pvHole},
    // page 1: film + meter reading
    {"FILM", 1, true, [](int d) { pc::stepFilm(ph.ex, d); }, pvFilm},
    {"METERED TIME", 1, false, [](int d) { pc::stepTime(ph.ex, d); }, pvTime},
    {"METERED AT APERTURE", 1, false, [](int d) { pc::stepAperture(ph.ex, d); }, pvMeterAp},
    // page 2: ISO and filter
    {"METER ISO", 2, false, [](int d) { pc::stepMeterIso(ph.ex, d); }, pvMeterIso},
    {"FILM SPEED (EI)", 2, false, [](int d) { pc::stepFilmIso(ph.ex, d); }, pvFilmIso},
    {"FILTER / ND", 2, false, [](int d) { pc::stepFilter(ph.ex, d); }, pvFilter},
};
const char* const phPageNames[] = {"PINHOLE", "FILM + METER", "ISO + FILTER"};
RowPage phPage = {phRows, sizeof(phRows) / sizeof(phRows[0]), 3, phPageNames, 0, 0};

void drawPhPanel() {
  const uint16_t green = RGB(60, 230, 110), warn = RGB(255, 90, 70);
  pc::PinholeResult r = pc::pinholeCompute(ph);
  tft.fillRect(0, 0, 240, RP_PANEL_H, C_BG);
  char t[24], line[48], pg[32];
  pc::formatTime(r.exp.finalTime, t, sizeof t);
  snprintf(pg, sizeof pg, "%d/%d %s", phPage.page + 1, phPage.pages, phPageNames[phPage.page]);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawString(r.exp.finalTime >= 1.f ? "TAP TO TIME" : "PINHOLE", 6, 6, 2);
  tft.drawRightString(pg, 234, 10, 1);
  int sz = tft.textWidth(t, 4) * 2 <= 230 ? 2 : 1;
  tft.setTextSize(sz);
  tft.setTextColor(green, C_BG);
  tft.drawCentreString(t, 120, sz == 2 ? 24 : 36, 4);
  tft.setTextSize(1);
  snprintf(line, sizeof line, "f/%.0f    FOV %.0f deg", r.fNumber, r.fovDeg);
  tft.setTextColor(C_TEXT, C_BG);
  tft.drawCentreString(line, 120, 78, 2);
  snprintf(line, sizeof line, "optimal hole %.3f mm   %+.1f stops", r.optimalHoleMm, r.exp.totalStops);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawCentreString(line, 120, 97, 1);
  tft.setTextColor(warn, C_BG);
  if (r.exp.colorWarning && r.exp.beyondData) tft.drawCentreString("Color film, beyond data: very rough", 120, 100 + 8, 1);
  else if (r.exp.colorWarning) tft.drawCentreString("Color film: experimental, expect shift", 120, 100 + 8, 1);
  else if (r.exp.beyondData) tft.drawCentreString("Beyond published data: extrapolated", 120, 100 + 8, 1);
}

void drawPinhole() {
  tft.fillScreen(C_BG);
  rpDrawRows(phPage);
  drawPhPanel();
  rpDrawButtons();
}

// Returns 0 = nothing, 1 = go home, 2 = start the countdown (tap on the result panel, time >= 1 s).
int pinholeTap(int x, int y) {
  switch (rpTap(phPage, x, y)) {
    case RP_HOME: return 1;
    case RP_PANEL: return pc::pinholeCompute(ph).exp.finalTime >= 1.f ? 2 : 0;
    case RP_CHANGED: drawPhPanel(); return 0;
    case RP_PAGE: drawPinhole(); return 0;
    default: return 0;
  }
}
