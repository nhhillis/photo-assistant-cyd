// Exposure math: reciprocity, filters, bellows, pinhole geometry.
#pragma once
#include "films.h"

namespace pc {

struct Recip {
  float tc;            // corrected time, seconds
  float stops;         // stops added by reciprocity (log2(tc/tm))
  bool applied;        // false if no correction was needed
  bool beyondData;     // true if tm is past the film's published/tested range
  float devPct;        // development change, percent (0 if film has no data)
};

// Reciprocity-corrected time for a metered time tm (seconds) on a film.
Recip reciprocity(const Film& f, float tm);
// Same, with a user tuning factor k on the stops reciprocity adds: tc = tm * 2^(k * stops).
// k = 1 is the published curve, k = 0 disables it, k = 1.5 adds half as many extra stops again.
// For a Power film this is the same as using P' = 1 + k * (P - 1).
Recip reciprocityTuned(const Film& f, float tm, float k);

// Filter / ND helpers (multiply the metered time BEFORE reciprocity).
float filterFactorFromStops(float stops);   // 2^stops
float bellowsFactor(float extensionMm, float focalMm);  // (ext/f)^2

// Exposure from EV at ISO 100: t = N^2 / 2^EV, scaled for ISO.
float timeFromEV(float ev100, float fNumber, float iso);
// Convert a time metered at fRef to another aperture (square law).
float scaleToAperture(float tRef, float fRef, float fNew);
// Convert a time metered at isoRef to the film's ISO.
float scaleToISO(float t, float isoMeter, float isoFilm);

// Pinhole geometry
float pinholeFNumber(float focalMm, float diameterMm);
float pinholeOptimalDiameterMm(float focalMm, float lambdaMm = 0.00055f);  // Rayleigh, 1.9*sqrt(f*lambda)
float fovDegrees(float frameMm, float focalMm);  // along one frame dimension

// Pretty-print seconds: "1/60", "12.3 s", "4 m 05 s", "1 h 12 m"
void formatTime(float seconds, char* out, int n);

}  // namespace pc
