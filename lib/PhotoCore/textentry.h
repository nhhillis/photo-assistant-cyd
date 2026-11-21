// On-screen keyboard text entry (WiFi names and passwords) and the clock's time zone list. Pure C++.
#pragma once
#include <stdint.h>

namespace pc {

// ---- text entry buffer
struct TextEntry {
  char text[65];
  uint8_t len;
  uint8_t maxLen;  // 32 for a WiFi name, 63 for a WPA password
};

void teInit(TextEntry& e, const char* initial, uint8_t maxLen);
bool teAppend(TextEntry& e, char c);  // printable ASCII only (32..126), up to maxLen
void teBackspace(TextEntry& e);
void teClear(TextEntry& e);

// ---- keyboard layers: 8 columns x 5 rows = 40 keys each. Space is a separate button.
// Together the three layers plus space cover every printable ASCII character.
const int KB_LAYERS = 3;
const int KB_COLS = 8;
const int KB_ROWS = 5;
const int KB_KEYS = KB_COLS * KB_ROWS;
const char* kbLayer(int layer);  // KB_KEYS characters (not NUL-terminated beyond that)

// ---- clock time zones (POSIX TZ strings, applied with tzset)
struct Zone {
  const char* name;
  const char* posix;
};
extern const Zone ZONES[];
extern const uint8_t ZONE_COUNT;
const uint8_t DEFAULT_ZONE = 1;  // US Central

}  // namespace pc
