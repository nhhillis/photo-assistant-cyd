// Film reciprocity profiles. Pure C++ (no Arduino deps) so it can be unit-tested on a PC.
#pragma once
#include <stdint.h>

namespace pc {

// Power: Tc = Tm^p for Tm > 1 s (Ilford style).
// Table: log-log interpolation between published (indicated, adjusted) points (Kodak style).
enum class RecipModel : uint8_t { Power, Table };

struct TablePt {
  float t;   // indicated (metered) time, seconds
  float tc;  // adjusted time, seconds
};

struct DevPt {
  float t;    // indicated time, seconds
  float pct;  // development change, percent (negative = reduce)
};

struct Film {
  const char* id;
  const char* name;
  RecipModel model;
  float p;                // exponent for Power films (ignored for Table)
  const TablePt* table;   // Table films
  uint8_t nTable;
  const DevPt* dev;       // optional development adjustment points
  uint8_t nDev;
  float dataLimit;        // seconds; beyond this the result is extrapolated / untested
  bool color;             // color negative: show experimental + color-shift warning
  const char* source;     // where the numbers came from (shown on the info screen)
  uint16_t boxIso;        // manufacturer's box speed (default film ISO in the exposure stack)
};

extern const Film FILMS[];
extern const uint8_t FILM_COUNT;

const Film* findFilm(const char* id);  // nullptr if not found

}  // namespace pc
