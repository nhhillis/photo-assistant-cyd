// Pinhole camera state + calculation. Pure C++, no Arduino.
// Exposure = the Exposure stack (metered time -> aperture -> ISO -> filter -> reciprocity), with the shoot
// aperture set to the pinhole's f-number (focal length / hole diameter).
#pragma once
#include <stdint.h>
#include "exposure_state.h"

namespace pc {

struct PinholePreset {
  const char* name;
  float focalMm;  // pinhole-to-film distance
  float holeMm;   // hole diameter
};

extern const PinholePreset PINHOLE_PRESETS[];
extern const uint8_t PINHOLE_PRESET_COUNT;

const float PINHOLE_FRAME_MM = 56.f;  // 6x6 frame side, used for field of view

struct PinholeState {
  uint8_t preset;  // last preset chosen (the values below may have been edited since)
  float focalMm;
  float holeMm;
  ExposureState ex;  // metering, film, filter; ex.shootAperture is overwritten with the f-number
};

struct PinholeResult {
  float fNumber;
  float optimalHoleMm;  // Rayleigh: 1.9 * sqrt(f * 550 nm)
  float fovDeg;         // across one side of a 6x6 frame
  ExposureResult exp;
};

void pinholeInit(PinholeState& s);                    // preset 0, default exposure state
void pinholeSetPreset(PinholeState& s, uint8_t idx);  // loads focal + hole
bool pinholeMatchesPreset(const PinholeState& s);     // false once focal or hole was edited
PinholeResult pinholeCompute(const PinholeState& s);
bool pinholeSanitize(PinholeState& s);  // like exposureSanitize; true if nothing needed fixing

void pinholeStepPreset(PinholeState& s, int dir);  // wraps
void pinholeStepFocal(PinholeState& s, int dir);   // 5 mm, 10..300
void pinholeStepHole(PinholeState& s, int dir);    // 0.01 mm, 0.10..1.00

}  // namespace pc
