// Host-side tests for PhotoCore. Build/run: make -C test/host
#include <math.h>
#include <stdio.h>
#include "exposure.h"
#include "solar.h"

using namespace pc;

static int fails = 0;
#define CHECK_NEAR(name, got, want, tol)                                                   \
  do {                                                                                     \
    double g = (got), w = (want);                                                          \
    bool ok = fabs(g - w) <= (tol);                                                        \
    printf("%s %-46s got %-10.4g want %-10.4g\n", ok ? "PASS" : "FAIL", name, g, w);       \
    if (!ok) fails++;                                                                      \
  } while (0)

int main() {
  // --- Ilford power law (example in Ilford's own sheet: HP5+ 10 s -> 20.4 s)
  CHECK_NEAR("HP5+ 10s", reciprocity(*findFilm("hp5"), 10).tc, 20.4, 0.1);
  CHECK_NEAR("HP5+ 1s unchanged", reciprocity(*findFilm("hp5"), 1).tc, 1.0, 1e-4);
  CHECK_NEAR("HP5+ 0.5s unchanged", reciprocity(*findFilm("hp5"), 0.5f).tc, 0.5, 1e-4);
  CHECK_NEAR("FP4+ 100s", reciprocity(*findFilm("fp4"), 100).tc, 331.1, 1.0);
  CHECK_NEAR("Pan F+ 100s", reciprocity(*findFilm("panf"), 100).tc, 457.1, 1.0);

  // --- Kodak tables hit their published anchor points exactly
  CHECK_NEAR("Tri-X 1s", reciprocity(*findFilm("trix400"), 1).tc, 2, 0.01);
  CHECK_NEAR("Tri-X 10s", reciprocity(*findFilm("trix400"), 10).tc, 50, 0.1);
  CHECK_NEAR("Tri-X 100s", reciprocity(*findFilm("trix400"), 100).tc, 1200, 1);
  CHECK_NEAR("Tri-X 0.05s unchanged", reciprocity(*findFilm("trix400"), 0.05f).tc, 0.05, 1e-5);
  CHECK_NEAR("T-Max100 10s", reciprocity(*findFilm("tmx100"), 10).tc, 15, 0.1);
  CHECK_NEAR("T-Max100 100s", reciprocity(*findFilm("tmx100"), 100).tc, 200, 0.5);
  CHECK_NEAR("T-Max400 100s", reciprocity(*findFilm("tmy400"), 100).tc, 300, 0.5);
  // between anchors: Tri-X 30 s lies between 50 and 1200, log-log
  float t30 = reciprocity(*findFilm("trix400"), 30).tc;
  printf("INFO Tri-X 30s -> %.0f s (between 50 and 1200)\n", t30);
  if (!(t30 > 50 && t30 < 1200)) { printf("FAIL interpolation range\n"); fails++; }
  // extrapolation beyond data is flagged
  Recip x = reciprocity(*findFilm("trix400"), 1000);
  printf("INFO Tri-X 1000s -> %.0f s (%.1f h) beyondData=%d\n", x.tc, x.tc / 3600, x.beyondData);
  if (!x.beyondData) { printf("FAIL beyondData flag\n"); fails++; }
  CHECK_NEAR("Tri-X dev @10s", reciprocity(*findFilm("trix400"), 10).devPct, -20, 0.01);
  CHECK_NEAR("Tri-X dev @100s", reciprocity(*findFilm("trix400"), 100).devPct, -30, 0.01);

  // --- Color ballpark: 10 s ~ 1 stop, 100 s ~ 2 stops
  CHECK_NEAR("Ektar 10s", reciprocity(*findFilm("ektar100"), 10).tc, 19.95, 0.1);
  CHECK_NEAR("Ektar 100s stops", reciprocity(*findFilm("ektar100"), 100).stops, 1.99, 0.05);

  // --- Pinhole geometry (your two pinholes)
  CHECK_NEAR("f-number 50mm/0.30mm", pinholeFNumber(50, 0.30f), 166.7, 0.1);
  CHECK_NEAR("f-number 35mm/0.26mm", pinholeFNumber(35, 0.26f), 134.6, 0.1);
  CHECK_NEAR("optimal dia 50mm", pinholeOptimalDiameterMm(50), 0.315, 0.005);
  CHECK_NEAR("optimal dia 35mm", pinholeOptimalDiameterMm(35), 0.264, 0.005);
  CHECK_NEAR("FOV 6x6 @50mm", fovDegrees(56, 50), 58.5, 0.3);   // along one side
  CHECK_NEAR("FOV 6x6 @35mm", fovDegrees(56, 35), 77.3, 0.3);

  // --- Exposure scaling: sunny 16 -> EV15 at ISO100, f/167 -> ~ 1/? s
  CHECK_NEAR("EV15 f/16 ISO100", timeFromEV(15, 16, 100), 1.0 / 128, 1e-4);
  CHECK_NEAR("scale f/16 -> f/167 (x109)", scaleToAperture(1.0 / 125, 16, 166.7f), 1.0 / 125 * 108.5, 0.1);
  CHECK_NEAR("bellows 2x ext = 4x", bellowsFactor(200, 100), 4, 1e-5);

  // --- Time formatting
  char b[32];
  formatTime(1.0f / 60, b, sizeof b); printf("INFO 1/60 -> %s\n", b);
  formatTime(245, b, sizeof b); printf("INFO 245 -> %s\n", b);
  formatTime(4320, b, sizeof b); printf("INFO 4320 -> %s\n", b);

  // --- Sun: Edmond, known neighbourhood of OKC values
  const Location& ed = PRESET_LOCATIONS[0];
  struct { int y, m, d; const char* label; float riseMin, setMin; } cases[] = {
      {2026, 12, 21, "Edmond winter solstice", 7 * 60 + 36, 17 * 60 + 20},   // CST, hand-derived (day length + eq. of time)
      {2026, 6, 21, "Edmond summer solstice", 6 * 60 + 15, 20 * 60 + 50},    // CDT, matches published OKC-area times
  };
  for (auto& c : cases) {
    SunDay s = sunDay(ed, c.y, c.m, c.d);
    char r[16], t[16];
    formatClock(s.sunrise, r, 16); formatClock(s.sunset, t, 16);
    printf("INFO %s: sunrise %s sunset %s (UTC%+d)\n", c.label, r, t, s.utcOffsetHours);
    CHECK_NEAR("  sunrise minutes", s.sunrise.minutes, c.riseMin, 3);
    CHECK_NEAR("  sunset minutes", s.sunset.minutes, c.setMin, 3);
  }
  // DST edges 2026: starts Mar 8, ends Nov 1
  CHECK_NEAR("DST Mar 7 2026 (CST)", utcOffsetFor(ed.tz, 2026, 3, 7), -6, 0);
  CHECK_NEAR("DST Mar 8 2026 (CDT)", utcOffsetFor(ed.tz, 2026, 3, 8), -5, 0);
  CHECK_NEAR("DST Oct 31 2026 (CDT)", utcOffsetFor(ed.tz, 2026, 10, 31), -5, 0);
  CHECK_NEAR("DST Nov 1 2026 (CST)", utcOffsetFor(ed.tz, 2026, 11, 1), -6, 0);

  // Broken Bow is ~11 min later than Edmond (further east)
  SunDay sb = sunDay(PRESET_LOCATIONS[1], 2026, 6, 21), se = sunDay(ed, 2026, 6, 21);
  printf("INFO Broken Bow vs Edmond sunrise offset: %.1f min earlier-than-Edmond-clock\n",
         se.sunrise.minutes - sb.sunrise.minutes);

  // Today's table, all four presets
  for (int i = 0; i < PRESET_COUNT; i++) {
    SunDay s = sunDay(PRESET_LOCATIONS[i], 2026, 11, 21);
    char a[16], b2[16], c2[16], d2[16];
    formatClock(s.sunrise, a, 16); formatClock(s.goldenEveningStart, b2, 16);
    formatClock(s.sunset, c2, 16); formatClock(s.blueEveningEnd, d2, 16);
    printf("INFO Nov 21 %-16s rise %-9s golden-eve %-9s set %-9s blue-end %s\n", PRESET_LOCATIONS[i].name, a, b2, c2, d2);
  }

  printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASSED", fails, fails == 1 ? "" : "s");
  return fails ? 1 : 0;
}
