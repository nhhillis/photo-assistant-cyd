#include "exposure_state.h"
#include <math.h>
#include <string.h>

namespace pc {

const float SHUTTER_S[] = {1.f / 1000, 1.f / 500, 1.f / 250, 1.f / 125, 1.f / 60, 1.f / 30, 1.f / 15, 1.f / 8,
                           1.f / 4,    1.f / 2,   1.f,       2.f,       4.f,      8.f,      15.f,     30.f,
                           60.f,       120.f,     240.f,     480.f,     900.f,    1800.f};
const uint8_t SHUTTER_COUNT = sizeof(SHUTTER_S) / sizeof(SHUTTER_S[0]);
const float APERTURES[] = {1.f, 1.4f, 2.f, 2.8f, 4.f, 5.6f, 8.f, 11.f, 16.f, 22.f, 32.f, 45.f, 64.f, 90.f, 128.f, 180.f, 256.f};
const uint8_t APERTURE_COUNT = sizeof(APERTURES) / sizeof(APERTURES[0]);
const uint16_t ISOS[] = {6, 12, 25, 50, 64, 80, 100, 125, 160, 200, 250, 320, 400, 800, 1600, 3200};
const uint8_t ISO_COUNT = sizeof(ISOS) / sizeof(ISOS[0]);

static uint8_t indexOfNearest(const float* v, uint8_t n, float x) {
  uint8_t best = 0;
  for (uint8_t i = 1; i < n; i++)
    if (fabsf(logf(v[i] / x)) < fabsf(logf(v[best] / x))) best = i;
  return best;
}

static void step(uint8_t& idx, uint8_t count, int dir) {
  if (dir > 0 && idx + 1 < count) idx++;
  else if (dir < 0 && idx > 0) idx--;
}

void exposureInit(ExposureState& s) {
  memset(&s, 0, sizeof s);
  s.timeIdx = indexOfNearest(SHUTTER_S, SHUTTER_COUNT, 1.f / 60);
  s.apIdx = indexOfNearest(APERTURES, APERTURE_COUNT, 16.f);
  for (uint8_t i = 0; i < ISO_COUNT; i++)
    if (ISOS[i] == 100) s.meterIsoIdx = i;
  s.focalMm = 150.f;  // placeholder lens; the UI sets it when bellows is used
  for (uint8_t i = 0; i < FILM_COUNT; i++)
    if (strcmp(FILMS[i].id, "fp4") == 0) s.film = i;
  s.filmIso = FILMS[s.film].boxIso;
}

void exposureSetFilm(ExposureState& s, uint8_t film) {
  if (film >= FILM_COUNT) return;
  s.film = film;
  s.filmIso = FILMS[film].boxIso;
}

ExposureResult exposureCompute(const ExposureState& s) {
  const Film& f = FILMS[s.film];
  ExposureResult r{};
  r.meteredTime = SHUTTER_S[s.timeIdx];
  float t = r.meteredTime;
  if (s.shootAperture > 0.f) t = scaleToAperture(t, APERTURES[s.apIdx], s.shootAperture);
  t = scaleToISO(t, ISOS[s.meterIsoIdx], s.filmIso);
  t *= filterFactorFromStops(s.filterThirds / 3.f);
  if (s.bellowsMm > s.focalMm && s.focalMm > 0.f) t *= bellowsFactor(s.bellowsMm, s.focalMm);
  r.baseTime = t;
  Recip rc = reciprocity(f, t);
  r.finalTime = rc.tc;
  r.recipStops = rc.stops;
  r.devPct = rc.devPct;
  r.beyondData = rc.beyondData;
  r.colorWarning = f.color;
  r.compStops = log2f(r.baseTime / r.meteredTime);
  r.totalStops = log2f(r.finalTime / r.meteredTime);
  return r;
}

void stepTime(ExposureState& s, int dir) { step(s.timeIdx, SHUTTER_COUNT, dir); }
void stepAperture(ExposureState& s, int dir) { step(s.apIdx, APERTURE_COUNT, dir); }
void stepMeterIso(ExposureState& s, int dir) { step(s.meterIsoIdx, ISO_COUNT, dir); }
void stepFilmIso(ExposureState& s, int dir) {
  uint8_t idx = 0;
  for (uint8_t i = 0; i < ISO_COUNT; i++)
    if (ISOS[i] <= s.filmIso) idx = i;  // box speeds not in the list snap to the entry below
  step(idx, ISO_COUNT, dir);
  s.filmIso = ISOS[idx];
}
void stepFilter(ExposureState& s, int dir) {
  if (dir > 0 && s.filterThirds < 60) s.filterThirds++;
  else if (dir < 0 && s.filterThirds > 0) s.filterThirds--;
}
void stepBellows(ExposureState& s, int dir) {
  s.bellowsMm += dir * 5.f;
  if (s.bellowsMm < 0.f) s.bellowsMm = 0.f;
  if (s.bellowsMm > 1000.f) s.bellowsMm = 1000.f;
}
void stepFilm(ExposureState& s, int dir) {
  int n = (int)s.film + dir;
  if (n < 0) n = FILM_COUNT - 1;
  if (n >= FILM_COUNT) n = 0;
  exposureSetFilm(s, (uint8_t)n);
}

}  // namespace pc
