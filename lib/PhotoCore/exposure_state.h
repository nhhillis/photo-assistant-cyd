// Exposure screen state + calculation. Pure C++, no Arduino. The UI only calls these.
// Stack order (docs/SCREENS.md): metered time -> aperture -> ISO -> filter/ND -> bellows -> reciprocity.
#pragma once
#include <stdint.h>
#include "exposure.h"

namespace pc {

// Stepping lists (full-stop shutter speeds, full-stop apertures, common ISOs).
extern const float SHUTTER_S[];
extern const uint8_t SHUTTER_COUNT;
extern const float APERTURES[];  // full stops plus the f-numbers actually used (135, 137, 167, 215); used for metered and shoot aperture
extern const uint8_t APERTURE_COUNT;
extern const float FOCALS[];  // lens focal lengths in mm: 18, 35, 50, 85
extern const uint8_t FOCAL_COUNT;
extern const uint16_t ISOS[];
extern const uint8_t ISO_COUNT;

struct ExposureState {
  uint8_t film;          // index into FILMS
  uint8_t timeIdx;       // metered time, index into SHUTTER_S
  uint8_t apIdx;         // metered aperture, index into APERTURES
  uint8_t meterIsoIdx;   // ISO the meter was set to, index into ISOS
  uint16_t filmIso;      // ISO the film is exposed at (defaults to the film's box speed)
  float shootAperture;   // f-number actually used; 0 = same as metered
  uint8_t filterThirds;  // filter/ND in 1/3 stops (0..60)
  float bellowsMm;       // bellows extension; <= focalMm means no compensation
  float focalMm;         // lens focal length (for bellows factor)
};

struct ExposureResult {
  float meteredTime;  // as entered, seconds
  float baseTime;     // after aperture, ISO, filter, bellows (before reciprocity), seconds
  float finalTime;    // after reciprocity, seconds
  float compStops;    // log2(baseTime / meteredTime)
  float recipStops;   // stops added by reciprocity
  float totalStops;   // log2(finalTime / meteredTime)
  float devPct;       // development change, percent
  bool beyondData;    // reciprocity result is extrapolated / untested
  bool colorWarning;  // color film: experimental, expect color shift
};

// Per-film reciprocity tuning factor (see reciprocityTuned). One shared table, indexed like FILMS,
// used by every screen. Default 1.0; the UI persists it. Range 0.50..2.00 in 0.05 steps.
float tuneGet(uint8_t film);
void tuneSet(uint8_t film, float k);  // clamps and rounds to 0.05
void tuneReset();                     // all films back to 1.0
void stepFilmTune(ExposureState& s, int dir);

void exposureInit(ExposureState& s);                   // 1/60 @ f/16, meter ISO 100, Ilford FP4+
void exposureSetFilm(ExposureState& s, uint8_t film);  // also resets filmIso to the box speed
ExposureResult exposureCompute(const ExposureState& s);

// Step helpers for [-]/[+] buttons (dir = +1 / -1). Clamp at the ends (film wraps around).
void stepTime(ExposureState& s, int dir);
void stepAperture(ExposureState& s, int dir);
void stepMeterIso(ExposureState& s, int dir);
void stepFilmIso(ExposureState& s, int dir);
void stepFilter(ExposureState& s, int dir);   // 1/3 stop
void stepBellows(ExposureState& s, int dir);  // 10 mm; first + jumps past the focal length, - below it returns to off
void stepFocal(ExposureState& s, int dir);    // steps through FOCALS
void stepShootAperture(ExposureState& s, int dir);  // full stops; landing on the metered f-number resets to 0 (same)
void stepFilm(ExposureState& s, int dir);

}  // namespace pc
