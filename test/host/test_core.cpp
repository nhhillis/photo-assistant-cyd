// Host-side tests for PhotoCore. Build/run: make -C test/host
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "exposure.h"
#include "exposure_state.h"
#include "countdown.h"
#include "pinhole_state.h"
#include "numentry.h"
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

  // --- Exposure state stack. Expected values are worked by hand from published anchors, not copied from the code.
  {
    ExposureState e;
    ExposureResult r;
    auto findIdx = [](const char* id) { for (int i = 0; i < FILM_COUNT; i++) if (!strcmp(FILMS[i].id, id)) return i; return -1; };
    auto setTime = [](ExposureState& st, float t) { st.timeIdx = 0; while (SHUTTER_S[st.timeIdx] < t * 0.99f) st.timeIdx++; };
    int hp5 = findIdx("hp5"), trix = findIdx("trix400"), ektar = findIdx("ektar100");

    exposureInit(e);
    r = exposureCompute(e);
    CHECK_NEAR("defaults: FP4+ box ISO 125", e.filmIso, 125, 0);
    CHECK_NEAR("defaults: 1/60 @ ISO100 -> ISO125 = 1/75 s", r.finalTime, 1.0 / 75, 1e-5);  // 1/60 * 100/125, by hand
    CHECK_NEAR("defaults: total stops = log2(100/125)", r.totalStops, -0.3219, 1e-3);

    // Ilford's own example: HP5+ 10 s -> 20.4 s. Get exactly 10 s from 1 s by going f/16 -> f/50.6 (x10).
    exposureInit(e); exposureSetFilm(e, hp5);
    e.meterIsoIdx = 12;  // meter ISO 400 = HP5+ box speed
    setTime(e, 1.f); e.shootAperture = 16.f * sqrtf(10.f);
    r = exposureCompute(e);
    CHECK_NEAR("stack: 1 s @f/16 -> f/50.6 = 10 s", r.baseTime, 10.0, 0.01);
    CHECK_NEAR("stack: HP5+ 10 s -> Ilford 20.4 s", r.finalTime, 20.4, 0.1);

    // Kodak anchors through the stack (Tri-X: 1 s -> 2 s, 10 s -> 50 s, dev -20% at 10 s)
    exposureInit(e); exposureSetFilm(e, trix); e.meterIsoIdx = 12;
    setTime(e, 1.f / 4); e.bellowsMm = 200; e.focalMm = 100;   // bellows 2x extension = x4 -> exactly 1 s
    r = exposureCompute(e);
    CHECK_NEAR("bellows 200/100 on 1/4 s -> 1 s", r.baseTime, 1.0, 1e-4);
    CHECK_NEAR("Tri-X 1 s -> 2 s (Kodak)", r.finalTime, 2.0, 0.01);
    e.bellowsMm = 0; setTime(e, 1.f / 8); e.filterThirds = 9;  // 3 stops of ND: 1/8 s -> 1 s
    CHECK_NEAR("ND 3 stops on 1/8 s -> 1 s", exposureCompute(e).baseTime, 1.0, 1e-4);
    setTime(e, 1.f); e.filterThirds = 0; e.shootAperture = 16.f * sqrtf(10.f);
    r = exposureCompute(e);
    CHECK_NEAR("Tri-X 10 s -> 50 s (Kodak)", r.finalTime, 50.0, 0.3);
    CHECK_NEAR("Tri-X dev at 10 s = -20%", r.devPct, -20, 0.1);
    CHECK_NEAR("total stops = log2(50/1)", r.totalStops, 5.644, 0.01);

    // Aperture and ISO scaling by hand: f/16->f/32 = x4; meter ISO 100 -> film 400 = /4
    exposureInit(e); e.filmIso = 100; e.shootAperture = 32;
    CHECK_NEAR("f/16 -> f/32 quadruples", exposureCompute(e).baseTime, 4.0 / 60, 1e-5);
    exposureInit(e); e.filmIso = 400;
    CHECK_NEAR("meter ISO 100, film 400 -> quarter", exposureCompute(e).baseTime, 1.0 / 240, 1e-6);

    // Bellows at or below the focal length gives no compensation
    exposureInit(e); e.filmIso = 100; e.bellowsMm = 150; e.focalMm = 150;
    CHECK_NEAR("bellows = focal -> no compensation", exposureCompute(e).baseTime, 1.0 / 60, 1e-6);

    // Warnings
    exposureInit(e); exposureSetFilm(e, ektar);
    CHECK_NEAR("color film sets warning", exposureCompute(e).colorWarning, 1, 0);
    exposureInit(e); exposureSetFilm(e, trix); setTime(e, 1800.f);
    CHECK_NEAR("1800 s on Tri-X beyond data", exposureCompute(e).beyondData, 1, 0);
    exposureInit(e);
    r = exposureCompute(e);
    CHECK_NEAR("default shows no warnings", r.beyondData || r.colorWarning, 0, 0);

    // Stepping clamps; bellows never negative; film steps wrap
    exposureInit(e);
    for (int i = 0; i < 100; i++) stepTime(e, +1);
    CHECK_NEAR("time clamps at top", e.timeIdx, SHUTTER_COUNT - 1, 0);
    for (int i = 0; i < 100; i++) stepTime(e, -1);
    CHECK_NEAR("time clamps at bottom", e.timeIdx, 0, 0);
    stepBellows(e, -1);
    CHECK_NEAR("bellows >= 0", e.bellowsMm, 0, 0);
    exposureInit(e); stepBellows(e, +1);
    CHECK_NEAR("bellows first + jumps past focal (50 -> 60)", e.bellowsMm, 60, 0);
    stepBellows(e, -1);
    CHECK_NEAR("bellows - back to off", e.bellowsMm, 0, 0);
    exposureInit(e); stepShootAperture(e, +1);
    CHECK_NEAR("shoot aperture f/16 +1 stop = f/22", e.shootAperture, 22, 0);
    stepShootAperture(e, -1);
    CHECK_NEAR("shoot aperture back to metered = 0 (same)", e.shootAperture, 0, 0);
    exposureInit(e); e.shootAperture = 167; stepShootAperture(e, -1);
    CHECK_NEAR("shoot aperture 167 - 1 step = 137", e.shootAperture, 137, 0);
    stepShootAperture(e, +1); stepShootAperture(e, +1); stepShootAperture(e, +1);
    CHECK_NEAR("shoot aperture 167 + 2 steps = 215 (via 180)", e.shootAperture, 215, 0);
    exposureInit(e);
    bool seen[4] = {false, false, false, false};
    for (int i = 0; i < 30; i++) {
      stepShootAperture(e, +1);
      if (e.shootAperture == 135) seen[0] = true;
      if (e.shootAperture == 137) seen[1] = true;
      if (e.shootAperture == 167) seen[2] = true;
      if (e.shootAperture == 215) seen[3] = true;
    }
    CHECK_NEAR("shoot list reaches 135, 137, 167, 215", seen[0] && seen[1] && seen[2] && seen[3], 1, 0);
    exposureInit(e); stepFocal(e, +1);
    CHECK_NEAR("focal 50 + 1 = 85", e.focalMm, 85, 0);
    stepFocal(e, +1);
    CHECK_NEAR("focal clamps at 85", e.focalMm, 85, 0);
    for (int i = 0; i < 10; i++) stepFocal(e, -1);
    CHECK_NEAR("focal bottoms at 18", e.focalMm, 18, 0);
    stepFocal(e, +1);
    CHECK_NEAR("focal 18 + 1 = 35", e.focalMm, 35, 0);
    exposureInit(e); stepTime(e, +1);
    CHECK_NEAR("step time up from 1/60 = 1/30", SHUTTER_S[e.timeIdx], 1.0 / 30, 1e-6);
    exposureInit(e); stepFilm(e, -1); stepFilm(e, +1);
    CHECK_NEAR("film step round trip", strcmp(FILMS[e.film].id, "fp4"), 0, 0);
    exposureInit(e); e.film = 0; stepFilm(e, -1);
    CHECK_NEAR("film steps wrap", e.film, FILM_COUNT - 1, 0);
    exposureInit(e); for (int i = 0; i < 100; i++) stepFilter(e, +1);
    CHECK_NEAR("filter clamps at 20 stops", e.filterThirds, 60, 0);
  }

  // --- Pinhole state (hand-worked: 1/60 s at f/16, ISO 100 meter, FP4+ at 125, shot at f/166.67)
  {
    PinholeState ph;
    pinholeInit(ph);
    PinholeResult pr = pinholeCompute(ph);
    CHECK_NEAR("pinhole preset 0: f/166.7", pr.fNumber, 166.7, 0.1);
    CHECK_NEAR("pinhole preset 0: optimal 0.315 mm", pr.optimalHoleMm, 0.315, 0.005);
    CHECK_NEAR("pinhole preset 0: FOV 58.5 deg", pr.fovDeg, 58.5, 0.3);
    // base = 1/60 * (166.67/16)^2 * 100/125 = 1.4468 s; FP4+ P=1.26 -> 1.4468^1.26 = 1.592 s
    CHECK_NEAR("pinhole preset 0: base time 1.4468 s", pr.exp.baseTime, 1.4468, 0.005);
    CHECK_NEAR("pinhole preset 0: FP4+ final 1.592 s", pr.exp.finalTime, 1.592, 0.01);
    CHECK_NEAR("pinhole does not edit the stored exposure state", ph.ex.shootAperture, 0, 0);
    pinholeStepPreset(ph, +1);
    pr = pinholeCompute(ph);
    CHECK_NEAR("pinhole preset 1: f/134.6", pr.fNumber, 134.6, 0.1);
    CHECK_NEAR("pinhole preset 1: optimal 0.264 mm", pr.optimalHoleMm, 0.264, 0.005);
    CHECK_NEAR("pinhole preset 1: FOV 77.3 deg", pr.fovDeg, 77.3, 0.3);
    pinholeStepPreset(ph, +1);
    CHECK_NEAR("pinhole presets wrap", ph.preset, 0, 0);
    pinholeStepHole(ph, +1);
    CHECK_NEAR("pinhole hole +0.01 mm = 0.31", ph.holeMm, 0.31, 1e-5);
    CHECK_NEAR("edited hole no longer matches preset", pinholeMatchesPreset(ph), 0, 0);
    pinholeStepHole(ph, -1);
    CHECK_NEAR("hole back to 0.30 matches preset again", pinholeMatchesPreset(ph), 1, 0);
    for (int i = 0; i < 200; i++) pinholeStepHole(ph, -1);
    CHECK_NEAR("hole clamps at 0.10 mm", ph.holeMm, 0.10, 1e-5);
    for (int i = 0; i < 100; i++) pinholeStepFocal(ph, +1);
    CHECK_NEAR("focal clamps at 300 mm", ph.focalMm, 300, 0);
  }

  // --- Numeric keypad entry (typing coordinates)
  {
    NumEntry n;
    float v = 0;
    auto type = [&](const char* keys) { for (const char* k = keys; *k; k++) neKey(n, *k); };
    neInit(n, 35.6528f, -90, 90);
    CHECK_NEAR("init shows 35.6528", strcmp(n.text, "35.6528"), 0, 0);
    neInit(n, -97.0f, -180, 180);
    CHECK_NEAR("init trims zeros: -97", strcmp(n.text, "-97"), 0, 0);
    neInit(n, 0.f, -90, 90);
    type("5");
    CHECK_NEAR("leading 0 replaced: 0 then 5 = 5", strcmp(n.text, "5"), 0, 0);
    neInit(n, 0.f, -90, 90); n.len = 0; n.text[0] = 0;
    type("35.6528");
    CHECK_NEAR("typed 35.6528 valid", neValue(n, v) && fabs(v - 35.6528) < 1e-4, 1, 0);
    CHECK_NEAR("second '.' ignored", neKey(n, '.'), 0, 0);
    n.len = 0; n.text[0] = 0;
    type("-97.4781");
    CHECK_NEAR("typed -97.4781 (lat range 90): out of range", neValue(n, v), 0, 0);
    NumEntry lon; neInit(lon, 0.f, -180, 180); lon.len = 0; lon.text[0] = 0;
    for (const char* k = "-97.4781"; *k; k++) neKey(lon, *k);
    CHECK_NEAR("typed -97.4781 valid for longitude", neValue(lon, v) && fabs(v + 97.4781) < 1e-4, 1, 0);
    neKey(lon, '-');
    CHECK_NEAR("'-' toggles sign off", strcmp(lon.text, "97.4781"), 0, 0);
    neKey(lon, '-');
    CHECK_NEAR("'-' toggles sign on", strcmp(lon.text, "-97.4781"), 0, 0);
    neBackspace(lon); neBackspace(lon);
    CHECK_NEAR("backspace x2", strcmp(lon.text, "-97.47"), 0, 0);
    NumEntry e2; neInit(e2, 0.f, -90, 90); e2.len = 0; e2.text[0] = 0;
    CHECK_NEAR("empty is not a value", neValue(e2, v), 0, 0);
    neKey(e2, '-');
    CHECK_NEAR("lone '-' is not a value", neValue(e2, v), 0, 0);
    neKey(e2, '.');
    CHECK_NEAR("'-.' becomes '-0.'", strcmp(e2.text, "-0."), 0, 0);
    neKey(e2, '5');
    CHECK_NEAR("-0.5 valid", neValue(e2, v) && fabs(v + 0.5) < 1e-6, 1, 0);
    e2.len = 0; e2.text[0] = 0;
    neKey(e2, '1'); neKey(e2, '2'); neKey(e2, '.');
    CHECK_NEAR("decimals capped at 5", (neKey(e2, '1'), neKey(e2, '1'), neKey(e2, '1'), neKey(e2, '1'), neKey(e2, '1'), neKey(e2, '1')), 0, 0);
  }

  // --- Per-film reciprocity tuning (stops scale by k; values worked by hand)
  {
    tuneReset();
    const Film* hp = findFilm("hp5");
    const Film* tx = findFilm("trix400");
    CHECK_NEAR("tuned k=1 = published (HP5+ 10 s -> 20.4)", reciprocityTuned(*hp, 10, 1.f).tc, 20.4, 0.1);
    CHECK_NEAR("tuned k=0 disables correction", reciprocityTuned(*hp, 10, 0.f).tc, 10.0, 1e-4);
    // HP5+ 10 s: 10^1.31 = 20.42 s = 1.0306 stops; k=1.5 -> 1.5459 stops -> 10 * 2^1.5459 = 29.2 s
    CHECK_NEAR("HP5+ k=1.5 -> 29.2 s", reciprocityTuned(*hp, 10, 1.5f).tc, 29.2, 0.1);
    // Tri-X 10 s -> 50 s = 2.3219 stops; k=0.5 -> 1.161 stops = x sqrt(5) -> 22.36 s
    CHECK_NEAR("Tri-X k=0.5 -> 22.36 s (half the stops)", reciprocityTuned(*tx, 10, 0.5f).tc, 22.36, 0.05);
    CHECK_NEAR("tuning does nothing at 1 s or less (HP5+ 0.5 s)", reciprocityTuned(*hp, 0.5f, 2.f).tc, 0.5, 1e-5);
    CHECK_NEAR("tuned flag keeps beyondData", reciprocityTuned(*tx, 1000, 0.5f).beyondData, 1, 0);
    CHECK_NEAR("tuned stops reported", reciprocityTuned(*tx, 10, 0.5f).stops, 1.161, 0.005);

    ExposureState e;
    exposureInit(e);
    int hpi = 0; for (int i = 0; i < FILM_COUNT; i++) if (!strcmp(FILMS[i].id, "hp5")) hpi = i;
    exposureSetFilm(e, hpi); e.meterIsoIdx = 12;
    e.timeIdx = 0; while (SHUTTER_S[e.timeIdx] < 1.f) e.timeIdx++;
    e.shootAperture = 16.f * sqrtf(10.f);   // 10 s
    CHECK_NEAR("stack uses tune 1.0 by default", exposureCompute(e).finalTime, 20.4, 0.1);
    stepFilmTune(e, +1); stepFilmTune(e, +1);
    CHECK_NEAR("two + steps = 1.10", tuneGet(hpi), 1.10, 1e-5);
    CHECK_NEAR("stack follows tune: 10 s at k=1.1 = 10*2^(1.1*1.0306)", exposureCompute(e).finalTime, 10 * pow(2, 1.1 * 1.0306), 0.1);
    exposureSetFilm(e, (hpi + 1) % FILM_COUNT);
    CHECK_NEAR("tune is per film", tuneGet((hpi + 1) % FILM_COUNT), 1.0, 1e-6);
    for (int i = 0; i < 100; i++) stepFilmTune(e, +1);
    CHECK_NEAR("tune clamps at 2.0", tuneGet(e.film), 2.0, 1e-5);
    for (int i = 0; i < 100; i++) stepFilmTune(e, -1);
    CHECK_NEAR("tune clamps at 0.5", tuneGet(e.film), 0.5, 1e-5);
    tuneReset();
    CHECK_NEAR("reset returns to 1.0", tuneGet(hpi), 1.0, 1e-6);
  }

  // --- Countdown timer (times worked by hand)
  {
    Countdown c;
    char b2[24];
    cdSet(c, 10.f);
    CHECK_NEAR("countdown set: Ready, 10000 ms", c.state == Countdown::Ready && c.remainingMs == 10000, 1, 0);
    cdStart(c, 1000);
    CHECK_NEAR("not done at +4 s", cdUpdate(c, 5000), 0, 0);
    CHECK_NEAR("remaining after 4 s of 10", c.remainingMs, 6000, 0);
    cdPause(c, 6000);   // 1 more second counted at pause
    CHECK_NEAR("pause counts time up to the pause", c.remainingMs, 5000, 0);
    cdUpdate(c, 20000);
    CHECK_NEAR("paused time does not run", c.remainingMs, 5000, 0);
    cdStart(c, 20000);
    CHECK_NEAR("not done at +4.9 s", cdUpdate(c, 24900), 0, 0);
    CHECK_NEAR("finishes exactly once (true)", cdUpdate(c, 25000), 1, 0);
    CHECK_NEAR("finished state is Done, 0 left", c.state == Countdown::Done && c.remainingMs == 0, 1, 0);
    CHECK_NEAR("no second finish (false)", cdUpdate(c, 26000), 0, 0);
    cdReset(c);
    CHECK_NEAR("reset returns to Ready, full time", c.state == Countdown::Ready && c.remainingMs == 10000, 1, 0);
    cdSet(c, 10.f); cdStart(c, 0xFFFFFF00UL); cdUpdate(c, 0x100UL);   // wraps: 0x200 = 512 ms elapsed
    CHECK_NEAR("millis() wraparound", c.remainingMs, 10000 - 512, 0);
    cdSet(c, 1e9f);
    CHECK_NEAR("capped at 99 h", c.totalMs, 99.0 * 3600 * 1000, 0);
    formatCountdown(245000, b2, sizeof b2);  CHECK_NEAR("fmt 245 s = 4:05", strcmp(b2, "4:05"), 0, 0);
    formatCountdown(4320000, b2, sizeof b2); CHECK_NEAR("fmt 4320 s = 1:12:00", strcmp(b2, "1:12:00"), 0, 0);
    formatCountdown(9400, b2, sizeof b2);    CHECK_NEAR("fmt 9.4 s = 9.4", strcmp(b2, "9.4"), 0, 0);
    formatCountdown(9401, b2, sizeof b2);    CHECK_NEAR("fmt rounds up 9.401 s = 9.5", strcmp(b2, "9.5"), 0, 0);
    formatCountdown(10000, b2, sizeof b2);   CHECK_NEAR("fmt 10 s = 10", strcmp(b2, "10"), 0, 0);
    formatCountdown(59001, b2, sizeof b2);   CHECK_NEAR("fmt 59.001 s = 1:00", strcmp(b2, "1:00"), 0, 0);
    formatCountdown(1, b2, sizeof b2);       CHECK_NEAR("fmt 1 ms = 0.1 (never 0.0 before done)", strcmp(b2, "0.1"), 0, 0);
    formatCountdown(0, b2, sizeof b2);       CHECK_NEAR("fmt 0 = 0.0", strcmp(b2, "0.0"), 0, 0);
  }

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
