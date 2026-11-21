#include "pinhole_state.h"
#include <math.h>

namespace pc {

// The two 6x6 pinholes (docs/SCREENS.md): 50 mm / 0.30 mm = f/167, 35 mm / 0.26 mm = f/135.
const PinholePreset PINHOLE_PRESETS[] = {
    {"50 mm / 0.30 mm", 50.f, 0.30f},
    {"35 mm / 0.26 mm", 35.f, 0.26f},
};
const uint8_t PINHOLE_PRESET_COUNT = sizeof(PINHOLE_PRESETS) / sizeof(PINHOLE_PRESETS[0]);

void pinholeSetPreset(PinholeState& s, uint8_t idx) {
  if (idx >= PINHOLE_PRESET_COUNT) return;
  s.preset = idx;
  s.focalMm = PINHOLE_PRESETS[idx].focalMm;
  s.holeMm = PINHOLE_PRESETS[idx].holeMm;
}

void pinholeInit(PinholeState& s) {
  exposureInit(s.ex);
  pinholeSetPreset(s, 0);
}

bool pinholeMatchesPreset(const PinholeState& s) {
  const PinholePreset& p = PINHOLE_PRESETS[s.preset];
  return fabsf(s.focalMm - p.focalMm) < 0.001f && fabsf(s.holeMm - p.holeMm) < 0.0005f;
}

PinholeResult pinholeCompute(const PinholeState& s) {
  PinholeResult r{};
  r.fNumber = pinholeFNumber(s.focalMm, s.holeMm);
  r.optimalHoleMm = pinholeOptimalDiameterMm(s.focalMm);
  r.fovDeg = fovDegrees(PINHOLE_FRAME_MM, s.focalMm);
  ExposureState e = s.ex;
  e.shootAperture = r.fNumber;
  r.exp = exposureCompute(e);
  return r;
}

bool pinholeSanitize(PinholeState& s) {
  bool ok = exposureSanitize(s.ex);
  if (s.preset >= PINHOLE_PRESET_COUNT) { s.preset = 0; ok = false; }
  if (!(s.focalMm >= 10.f && s.focalMm <= 300.f) || !(s.holeMm >= 0.10f && s.holeMm <= 1.00f)) {
    pinholeSetPreset(s, s.preset);
    ok = false;
  }
  return ok;
}

void pinholeStepPreset(PinholeState& s, int dir) {
  int n = (int)s.preset + dir;
  if (n < 0) n = PINHOLE_PRESET_COUNT - 1;
  if (n >= PINHOLE_PRESET_COUNT) n = 0;
  pinholeSetPreset(s, (uint8_t)n);
}

void pinholeStepFocal(PinholeState& s, int dir) {
  s.focalMm += dir * 5.f;
  if (s.focalMm < 10.f) s.focalMm = 10.f;
  if (s.focalMm > 300.f) s.focalMm = 300.f;
}

void pinholeStepHole(PinholeState& s, int dir) {
  s.holeMm = roundf((s.holeMm + dir * 0.01f) * 100.f) / 100.f;
  if (s.holeMm < 0.10f) s.holeMm = 0.10f;
  if (s.holeMm > 1.00f) s.holeMm = 1.00f;
}

}  // namespace pc
