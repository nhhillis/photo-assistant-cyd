// Settings kept in flash: up to three WiFi networks entered on the device, and the clock's time zone.
// Included from main.cpp before clock_sync.h.
#pragma once

const int SAVED_NETS = 3;
struct SavedNet { char ssid[33]; char pass[64]; };
SavedNet savedNets[SAVED_NETS];
uint8_t tzIdx = pc::DEFAULT_ZONE;

static void netKey(char* out, char kind, int i) { snprintf(out, 4, "%c%d", kind, i); }

void loadSettings() {
  Preferences p;
  p.begin("settings", true);
  for (int i = 0; i < SAVED_NETS; i++) {
    char k[4];
    savedNets[i].ssid[0] = 0;
    savedNets[i].pass[0] = 0;
    netKey(k, 's', i);
    p.getString(k, savedNets[i].ssid, sizeof savedNets[i].ssid);
    netKey(k, 'p', i);
    p.getString(k, savedNets[i].pass, sizeof savedNets[i].pass);
  }
  uint8_t z = p.getUChar("tz", pc::DEFAULT_ZONE);
  tzIdx = z < pc::ZONE_COUNT ? z : pc::DEFAULT_ZONE;
  p.end();
}

void saveNet(int i) {
  Preferences p;
  p.begin("settings", false);
  char k[4];
  netKey(k, 's', i);
  p.putString(k, savedNets[i].ssid);
  netKey(k, 'p', i);
  p.putString(k, savedNets[i].pass);
  p.end();
}

void saveTz() {
  Preferences p;
  p.begin("settings", false);
  p.putUChar("tz", tzIdx);
  p.end();
}
