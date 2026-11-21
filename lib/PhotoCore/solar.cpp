#include "solar.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdio.h>

namespace pc {

const Location PRESET_LOCATIONS[] = {
    {"Edmond, OK", 35.6528f, -97.4781f, {-6, true}},
    {"Broken Bow, OK", 34.0287f, -94.7388f, {-6, true}},
    {"Okarche, OK", 35.7314f, -97.9753f, {-6, true}},
    {"Tulsa, OK", 36.1540f, -95.9928f, {-6, true}},
};
const uint8_t PRESET_COUNT = sizeof(PRESET_LOCATIONS) / sizeof(PRESET_LOCATIONS[0]);

static const double D2R = M_PI / 180.0, R2D = 180.0 / M_PI;

static double julianDay(int y, int m, int d) {  // 0h UT of the given calendar date
  if (m <= 2) { y -= 1; m += 12; }
  int A = y / 100, B = 2 - A + A / 4;
  return floor(365.25 * (y + 4716)) + floor(30.6001 * (m + 1)) + d + B - 1524.5;
}

static void sunParams(double jd, double& eqTimeMin, double& decRad) {
  double T = (jd - 2451545.0) / 36525.0;
  double L0 = fmod(280.46646 + T * (36000.76983 + T * 0.0003032), 360.0);
  double M = 357.52911 + T * (35999.05029 - 0.0001537 * T);
  double e = 0.016708634 - T * (0.000042037 + 0.0000001267 * T);
  double Mr = M * D2R;
  double C = sin(Mr) * (1.914602 - T * (0.004817 + 0.000014 * T)) + sin(2 * Mr) * (0.019993 - 0.000101 * T) +
             sin(3 * Mr) * 0.000289;
  double trueLong = L0 + C;
  double omega = 125.04 - 1934.136 * T;
  double appLong = trueLong - 0.00569 - 0.00478 * sin(omega * D2R);
  double meanObl = 23.0 + (26.0 + (21.448 - T * (46.815 + T * (0.00059 - T * 0.001813))) / 60.0) / 60.0;
  double obl = (meanObl + 0.00256 * cos(omega * D2R)) * D2R;
  decRad = asin(sin(obl) * sin(appLong * D2R));
  double y = tan(obl / 2.0);
  y *= y;
  double L0r = L0 * D2R;
  eqTimeMin = 4.0 * R2D * (y * sin(2 * L0r) - 2 * e * sin(Mr) + 4 * e * y * sin(Mr) * cos(2 * L0r) -
                           0.5 * y * y * sin(4 * L0r) - 1.25 * e * e * sin(2 * Mr));
}

// UTC minutes after 0h UT for the sun crossing `angleDeg` (altitude) rising or setting. NaN if never.
static double eventUtcMin(double jd0, double lat, double lonEast, double angleDeg, bool rising) {
  double t = 720.0 - 4.0 * lonEast;  // first guess: solar noon
  for (int it = 0; it < 3; it++) {   // refine with sun position at the event time
    double eq, dec;
    sunParams(jd0 + t / 1440.0, eq, dec);
    double zen = (90.0 - angleDeg) * D2R, latr = lat * D2R;
    double c = cos(zen) / (cos(latr) * cos(dec)) - tan(latr) * tan(dec);
    if (c > 1.0 || c < -1.0) return NAN;
    double H = acos(c) * R2D;  // degrees
    double noon = 720.0 - 4.0 * lonEast - eq;
    t = rising ? noon - 4.0 * H : noon + 4.0 * H;
  }
  return t;
}

static int dayOfWeek(int y, int m, int d) {  // 0=Sunday
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3) y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

int utcOffsetFor(const Tz& tz, int y, int m, int d) {
  if (!tz.usDst) return tz.stdOffsetHours;
  // first Sunday of a month = 1 + (7 - dow(day 1)) % 7
  int firstSunMar = 1 + (7 - dayOfWeek(y, 3, 1)) % 7;
  int secondSunMar = firstSunMar + 7;
  int firstSunNov = 1 + (7 - dayOfWeek(y, 11, 1)) % 7;
  bool dst;
  if (m > 3 && m < 11) dst = true;
  else if (m == 3) dst = d >= secondSunMar;
  else if (m == 11) dst = d < firstSunNov;
  else dst = false;
  return tz.stdOffsetHours + (dst ? 1 : 0);
}

static Ev ev(double jd0, const Location& l, double angle, bool rising, int off) {
  double u = eventUtcMin(jd0, l.lat, l.lon, angle, rising);
  if (isnan(u)) return {false, 0.f};
  double local = u + off * 60.0;
  while (local < 0) local += 1440.0;
  while (local >= 1440.0) local -= 1440.0;
  return {true, (float)local};
}

SunDay sunDay(const Location& l, int y, int m, int d) {
  double jd0 = julianDay(y, m, d);
  int off = utcOffsetFor(l.tz, y, m, d);
  SunDay s;
  s.utcOffsetHours = off;
  s.civilDawn = ev(jd0, l, -6.0, true, off);
  s.blueMorningStart = ev(jd0, l, -8.0, true, off);
  s.blueMorningEnd = ev(jd0, l, -4.0, true, off);
  s.sunrise = ev(jd0, l, -0.833, true, off);
  s.goldenMorningEnd = ev(jd0, l, 6.0, true, off);
  s.goldenEveningStart = ev(jd0, l, 6.0, false, off);
  s.sunset = ev(jd0, l, -0.833, false, off);
  s.blueEveningStart = ev(jd0, l, -4.0, false, off);
  s.blueEveningEnd = ev(jd0, l, -8.0, false, off);
  s.civilDusk = ev(jd0, l, -6.0, false, off);
  return s;
}

void formatClock(Ev e, char* out, int n) {
  if (!e.valid) { snprintf(out, n, "--"); return; }
  int m = (int)lroundf(e.minutes) % 1440;
  int h = m / 60, mm = m % 60;
  const char* ap = h >= 12 ? "PM" : "AM";
  int h12 = h % 12 == 0 ? 12 : h % 12;
  snprintf(out, n, "%d:%02d %s", h12, mm, ap);
}

}  // namespace pc
