// Remembers the Exposure and Pinhole settings across reboots (flash, via Preferences).
// Saved a few seconds after the last tap, and when leaving a screen, so flash is not written on every press.
// A saved blob of the wrong size (firmware changed) or with the wrong magic is ignored; anything loaded is sanitized.
#pragma once

struct SavedState {
  uint32_t magic;
  pc::ExposureState ex;
  pc::PinholeState ph;
};
const uint32_t STATE_MAGIC = 0x50415831;  // "PAX1"

bool stateDirty = false;
uint32_t stateDirtyAt = 0;

void loadState() {
  Preferences p;
  p.begin("state", true);
  SavedState st;
  if (p.getBytesLength("blob") == sizeof(st)) {
    p.getBytes("blob", &st, sizeof(st));
    if (st.magic == STATE_MAGIC) {
      pc::exposureSanitize(st.ex);
      pc::pinholeSanitize(st.ph);
      ex = st.ex;
      ph = st.ph;
    }
  }
  uint8_t sel = p.getUChar("loc", 0);
  p.end();
  locIdx = sel < locCount() ? sel : 0;
}

void saveState() {
  Preferences p;
  p.begin("state", false);
  SavedState st;
  memset(&st, 0, sizeof st);
  st.magic = STATE_MAGIC;
  st.ex = ex;
  st.ph = ph;
  p.putBytes("blob", &st, sizeof(st));
  p.putUChar("loc", locIdx);
  p.end();
  stateDirty = false;
}

void markStateDirty() {
  stateDirty = true;
  stateDirtyAt = millis();
}

// Call every loop(): saves 3 s after the last change.
void stateTick() {
  if (stateDirty && millis() - stateDirtyAt > 3000) saveState();
}
