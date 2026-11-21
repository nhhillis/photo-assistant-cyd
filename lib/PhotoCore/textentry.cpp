#include "textentry.h"
#include <string.h>

namespace pc {

void teInit(TextEntry& e, const char* initial, uint8_t maxLen) {
  if (maxLen > 64) maxLen = 64;
  e.maxLen = maxLen;
  e.len = 0;
  e.text[0] = 0;
  for (const char* c = initial; c && *c; c++) teAppend(e, *c);
}

bool teAppend(TextEntry& e, char c) {
  if (c < 32 || c > 126) return false;
  if (e.len >= e.maxLen) return false;
  e.text[e.len++] = c;
  e.text[e.len] = 0;
  return true;
}

void teBackspace(TextEntry& e) {
  if (e.len > 0) e.text[--e.len] = 0;
}

void teClear(TextEntry& e) {
  e.len = 0;
  e.text[0] = 0;
}

static const char LAYER0[] = "abcdefghijklmnopqrstuvwxyz0123456789.-_@";
static const char LAYER1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_@";
static const char LAYER2[] = "!\"#$%&'()*+,/:;<=>?[\\]^`{|}~.-_@01234567";

const char* kbLayer(int layer) {
  switch (layer) {
    case 1: return LAYER1;
    case 2: return LAYER2;
    default: return LAYER0;
  }
}

const Zone ZONES[] = {
    {"US Eastern", "EST5EDT,M3.2.0,M11.1.0"},
    {"US Central", "CST6CDT,M3.2.0,M11.1.0"},
    {"US Mountain", "MST7MDT,M3.2.0,M11.1.0"},
    {"Arizona (no DST)", "MST7"},
    {"US Pacific", "PST8PDT,M3.2.0,M11.1.0"},
    {"Alaska", "AKST9AKDT,M3.2.0,M11.1.0"},
    {"Hawaii", "HST10"},
    {"UTC", "UTC0"},
};
const uint8_t ZONE_COUNT = sizeof(ZONES) / sizeof(ZONES[0]);

}  // namespace pc
