// Sunrise/sunset/golden/blue hour from lat/lon/date (NOAA solar algorithm). No network needed.
#pragma once
#include <stdint.h>

namespace pc {

struct Tz {
  int8_t stdOffsetHours;  // e.g. -6 for US Central standard time
  bool usDst;             // apply US daylight saving rules (2nd Sun Mar - 1st Sun Nov)
};

struct Location {
  const char* name;
  float lat;  // degrees north
  float lon;  // degrees east (west is negative)
  Tz tz;
};

extern const Location PRESET_LOCATIONS[];
extern const uint8_t PRESET_COUNT;

struct Ev {
  bool valid;      // false if the sun never reaches that angle that day
  float minutes;   // local minutes since midnight
};

struct SunDay {
  Ev civilDawn;          // sun at -6 deg
  Ev blueMorningStart;   // -8 deg
  Ev blueMorningEnd;     // -4 deg
  Ev sunrise;            // -0.833 deg
  Ev goldenMorningEnd;   // +6 deg
  Ev goldenEveningStart; // +6 deg
  Ev sunset;
  Ev blueEveningStart;   // -4 deg
  Ev blueEveningEnd;     // -8 deg
  Ev civilDusk;
  int utcOffsetHours;    // offset actually applied (handles DST)
};

int utcOffsetFor(const Tz& tz, int year, int month, int day);
SunDay sunDay(const Location& loc, int year, int month, int day);
void formatClock(Ev e, char* out, int n);  // "7:03 AM" or "--"

}  // namespace pc
