// Saves the per-film reciprocity tuning factors to flash, keyed by film id (so reordering films is safe).
#pragma once

void loadTune() {
  Preferences p;
  p.begin("tune", true);
  for (uint8_t i = 0; i < pc::FILM_COUNT; i++) {
    float k = p.getFloat(pc::FILMS[i].id, 1.0f);
    pc::tuneSet(i, k);
  }
  p.end();
}

void saveTune(uint8_t film) {
  Preferences p;
  p.begin("tune", false);
  p.putFloat(pc::FILMS[film].id, pc::tuneGet(film));
  p.end();
}
