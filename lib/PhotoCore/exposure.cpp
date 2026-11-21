#include "exposure.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdio.h>

namespace pc {

static float interpLogLog(const TablePt* p, uint8_t n, float tm) {
  // below first point: no correction
  if (tm <= p[0].t) return tm;
  uint8_t i = 0;
  while (i + 2 < n && tm > p[i + 1].t) i++;  // last segment is reused for extrapolation
  float x0 = log10f(p[i].t), x1 = log10f(p[i + 1].t);
  float y0 = log10f(p[i].tc), y1 = log10f(p[i + 1].tc);
  float y = y0 + (log10f(tm) - x0) * (y1 - y0) / (x1 - x0);
  return powf(10.f, y);
}

static float devAt(const Film& f, float tm) {
  if (!f.dev || f.nDev == 0) return 0.f;
  if (tm <= f.dev[0].t) return f.dev[0].pct;
  if (tm >= f.dev[f.nDev - 1].t) return f.dev[f.nDev - 1].pct;  // clamp past the data
  uint8_t i = 0;
  while (i + 2 < f.nDev && tm > f.dev[i + 1].t) i++;
  float x0 = log10f(f.dev[i].t), x1 = log10f(f.dev[i + 1].t);
  return f.dev[i].pct + (log10f(tm) - x0) * (f.dev[i + 1].pct - f.dev[i].pct) / (x1 - x0);
}

Recip reciprocity(const Film& f, float tm) {
  Recip r{tm, 0.f, false, tm > f.dataLimit, devAt(f, tm)};
  if (tm <= 0.f) return r;
  float tc = tm;
  if (f.model == RecipModel::Power) {
    if (tm > 1.f) tc = powf(tm, f.p);
  } else {
    tc = interpLogLog(f.table, f.nTable, tm);
  }
  r.tc = tc;
  r.applied = fabsf(tc - tm) > 1e-6f * tm;
  r.stops = log2f(tc / tm);
  return r;
}

Recip reciprocityTuned(const Film& f, float tm, float k) {
  Recip r = reciprocity(f, tm);
  if (tm <= 0.f || k == 1.f) return r;
  r.stops *= k;
  r.tc = tm * powf(2.f, r.stops);
  r.applied = fabsf(r.tc - tm) > 1e-6f * tm;
  return r;
}

float filterFactorFromStops(float stops) { return powf(2.f, stops); }
float bellowsFactor(float extensionMm, float focalMm) {
  float m = extensionMm / focalMm;
  return m * m;
}

float timeFromEV(float ev100, float n, float iso) { return (n * n * 100.f) / (iso * powf(2.f, ev100)); }
float scaleToAperture(float tRef, float fRef, float fNew) { return tRef * (fNew * fNew) / (fRef * fRef); }
float scaleToISO(float t, float isoMeter, float isoFilm) { return t * isoMeter / isoFilm; }

float pinholeFNumber(float focalMm, float diameterMm) { return focalMm / diameterMm; }
float pinholeOptimalDiameterMm(float focalMm, float lambdaMm) { return 1.9f * sqrtf(focalMm * lambdaMm); }
float fovDegrees(float frameMm, float focalMm) { return 2.f * atanf(frameMm / (2.f * focalMm)) * 180.f / (float)M_PI; }

void formatTime(float s, char* out, int n) {
  if (s < 0.f) s = 0.f;
  if (s < 0.95f && s > 0.f) {
    snprintf(out, n, "1/%d", (int)lroundf(1.f / s));
  } else if (s < 60.f) {
    snprintf(out, n, s < 10.f ? "%.1f s" : "%.0f s", s);
  } else if (s < 3600.f) {
    int t = (int)lroundf(s);
    snprintf(out, n, "%d m %02d s", t / 60, t % 60);
  } else {
    int t = (int)lroundf(s / 60.f);
    snprintf(out, n, "%d h %02d m", t / 60, t % 60);
  }
}

}  // namespace pc
