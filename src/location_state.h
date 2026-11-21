// Current location = one of the presets, or the user's saved custom location (stored in flash).
// Included from main.cpp right after `locIdx` is declared.
#pragma once

pc::Location customLoc = {"Custom", 35.6528f, -97.4781f, {-6, true}};
bool customSaved = false;

struct CustomStore { float lat, lon; int8_t stdOffset; uint8_t dst; };

void loadCustomLoc() {
  Preferences p;
  p.begin("loc", true);
  CustomStore c;
  if (p.getBytesLength("custom") == sizeof(c)) {
    p.getBytes("custom", &c, sizeof(c));
    customLoc.lat = c.lat;
    customLoc.lon = c.lon;
    customLoc.tz.stdOffsetHours = c.stdOffset;
    customLoc.tz.usDst = c.dst != 0;
    customSaved = true;
  }
  p.end();
}

void saveCustomLoc() {
  Preferences p;
  p.begin("loc", false);
  CustomStore c = {customLoc.lat, customLoc.lon, customLoc.tz.stdOffsetHours, (uint8_t)(customLoc.tz.usDst ? 1 : 0)};
  p.putBytes("custom", &c, sizeof(c));
  p.end();
  customSaved = true;
}

// Number of selectable locations: the presets, plus Custom once one is saved.
int locCount() { return pc::PRESET_COUNT + (customSaved ? 1 : 0); }

const pc::Location& currentLoc() {
  if (customSaved && locIdx == pc::PRESET_COUNT) return customLoc;
  return pc::PRESET_LOCATIONS[locIdx < pc::PRESET_COUNT ? locIdx : 0];
}
