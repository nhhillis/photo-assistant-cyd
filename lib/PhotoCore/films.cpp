#include "films.h"
#include <string.h>

namespace pc {

// ---- Kodak tables (anchored at 1/10 s = no correction, per Kodak's tables) ----
// Tri-X 320/400: 1 s -> 2 s, 10 s -> 50 s, 100 s -> 1200 s (Kodak F-4017)
static const TablePt TRIX[] = {{0.1f, 0.1f}, {1.f, 2.f}, {10.f, 50.f}, {100.f, 1200.f}};
static const DevPt TRIX_DEV[] = {{0.1f, 0.f}, {1.f, -10.f}, {10.f, -20.f}, {100.f, -30.f}};

// T-Max 100: +1/3 stop @1 s, +1/2 stop (15 s) @10 s, +1 stop (200 s) @100 s
static const TablePt TMX[] = {{0.1f, 0.1f}, {1.f, 1.26f}, {10.f, 15.f}, {100.f, 200.f}};
// T-Max 400: +1/3 stop @1 s, +1/2 stop (15 s) @10 s, +1 1/2 stops (300 s) @100 s
static const TablePt TMY[] = {{0.1f, 0.1f}, {1.f, 1.26f}, {10.f, 15.f}, {100.f, 300.f}};

#define ILFORD "Ilford tech sheet (Dec 2023), Tc = Tm^P, none <= 1 s"
#define KODAK_COLOR "Ballpark P=1.30 (community); Kodak gives no data >1 s. Expect color shift."

const Film FILMS[] = {
    // id           name                    model              p     table nT dev     nD  limit color source  boxIso
    {"tmx100", "Kodak T-Max 100", RecipModel::Table, 0.f, TMX, 4, nullptr, 0, 100.f, false, "Kodak F-4016 (verify vs printed sheet)", 100},
    {"tmy400", "Kodak T-Max 400", RecipModel::Table, 0.f, TMY, 4, nullptr, 0, 100.f, false, "Kodak F-4016 (verify vs printed sheet)", 400},
    {"trix400", "Kodak Tri-X 400", RecipModel::Table, 0.f, TRIX, 4, TRIX_DEV, 4, 100.f, false, "Kodak F-4017", 400},
    {"delta100", "Ilford Delta 100", RecipModel::Power, 1.26f, nullptr, 0, nullptr, 0, 100.f, false, ILFORD, 100},
    {"fp4", "Ilford FP4+", RecipModel::Power, 1.26f, nullptr, 0, nullptr, 0, 100.f, false, ILFORD, 125},
    {"hp5", "Ilford HP5+", RecipModel::Power, 1.31f, nullptr, 0, nullptr, 0, 100.f, false, ILFORD, 400},
    {"panf", "Ilford Pan F+", RecipModel::Power, 1.33f, nullptr, 0, nullptr, 0, 100.f, false, ILFORD, 50},
    {"ortho", "Ilford Ortho+", RecipModel::Power, 1.25f, nullptr, 0, nullptr, 0, 100.f, false, ILFORD, 80},
    {"kent100", "Kentmere 100", RecipModel::Power, 1.26f, nullptr, 0, nullptr, 0, 100.f, false, ILFORD, 100},
    {"kent400", "Kentmere 400", RecipModel::Power, 1.30f, nullptr, 0, nullptr, 0, 100.f, false, ILFORD, 400},
    {"gold200", "Kodak Gold 200", RecipModel::Power, 1.30f, nullptr, 0, nullptr, 0, 1.f, true, KODAK_COLOR, 200},
    {"ektar100", "Kodak Ektar 100", RecipModel::Power, 1.30f, nullptr, 0, nullptr, 0, 1.f, true, KODAK_COLOR, 100},
    {"portra160", "Kodak Portra 160", RecipModel::Power, 1.30f, nullptr, 0, nullptr, 0, 1.f, true, KODAK_COLOR, 160},
};
const uint8_t FILM_COUNT = sizeof(FILMS) / sizeof(FILMS[0]);

const Film* findFilm(const char* id) {
  for (uint8_t i = 0; i < FILM_COUNT; i++)
    if (strcmp(FILMS[i].id, id) == 0) return &FILMS[i];
  return nullptr;
}

}  // namespace pc
