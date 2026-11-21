// Exposure countdown screen. Included from main.cpp after exposure_screen.h.
// The timing logic is pc::Countdown (lib/PhotoCore/countdown); this only draws and flashes the backlight.
#pragma once

pc::Countdown cd;
uint32_t cdDoneAt = 0;
char cdLastText[16] = "";
int cdLastSize = 0;

const int CD_BTN_Y = 240, CD_BTN_H = 72;
Screen cdReturn = EXPOSURE;  // where BACK goes
char cdLabel[32] = "";      // film name shown under the progress bar

static uint16_t cdDigitColor() {
  if (cd.state == pc::Countdown::Done) return ((millis() - cdDoneAt) / 250) % 2 ? RGB(255, 60, 50) : TFT_WHITE;
  if (cd.state == pc::Countdown::Paused) return RGB(255, 170, 40);
  return cd.remainingMs <= 10000 && cd.state == pc::Countdown::Running ? RGB(255, 170, 40) : RGB(60, 230, 110);
}

void cdDrawButtons() {
  const uint16_t btn = RGB(40, 46, 66);
  const char* left = "START";
  const char* right = "BACK";
  switch (cd.state) {
    case pc::Countdown::Ready: break;
    case pc::Countdown::Running: left = "PAUSE"; right = "STOP"; break;
    case pc::Countdown::Paused: left = "RESUME"; right = "STOP"; break;
    case pc::Countdown::Done: left = "DONE"; right = "BACK"; break;
  }
  uint16_t leftCol = cd.state == pc::Countdown::Running ? RGB(255, 170, 40) : RGB(60, 230, 110);
  tft.fillRoundRect(0, CD_BTN_Y, 116, CD_BTN_H, 12, btn);
  tft.fillRoundRect(124, CD_BTN_Y, 116, CD_BTN_H, 12, btn);
  tft.setTextColor(leftCol, btn);
  tft.drawCentreString(left, 58, CD_BTN_Y + CD_BTN_H / 2 - 13, 4);
  tft.setTextColor(C_TEXT, btn);
  tft.drawCentreString(right, 182, CD_BTN_Y + CD_BTN_H / 2 - 13, 4);
}

void cdDrawProgress() {
  const int x = 10, y = 188, w = 220, h = 12;
  int fill = (int)(cdFraction(cd) * w);
  tft.drawRoundRect(x, y, w, h, 5, RGB(60, 66, 90));
  tft.fillRect(x + 2, y + 2, w - 4, h - 4, C_BG);
  if (fill > 4) tft.fillRoundRect(x + 2, y + 2, fill - 4, h - 4, 3, cdDigitColor());
}

// force = redraw even if the text did not change (state change)
void cdDrawTime(bool force) {
  char t[16];
  pc::formatCountdown(cd.remainingMs, t, sizeof t);
  if (!force && strcmp(t, cdLastText) == 0) return;
  strcpy(cdLastText, t);
  int sz = tft.textWidth(t, 6) * 2 <= 232 ? 2 : 1;
  if (sz != cdLastSize || force) {
    tft.fillRect(0, 62, 240, 118, C_BG);
    cdLastSize = sz;
  }
  tft.setTextSize(sz);
  tft.setTextColor(cdDigitColor(), C_BG);
  tft.setTextPadding(236);
  tft.drawCentreString(t, 120, sz == 2 ? 74 : 98, 6);
  tft.setTextPadding(0);
  tft.setTextSize(1);
  cdDrawProgress();
}

void cdDrawStatus() {
  const char* st = "READY";
  switch (cd.state) {
    case pc::Countdown::Ready: break;
    case pc::Countdown::Running: st = "EXPOSING"; break;
    case pc::Countdown::Paused: st = "PAUSED"; break;
    case pc::Countdown::Done: st = "TIME'S UP"; break;
  }
  tft.setTextColor(C_TEXT, C_BG);
  tft.setTextPadding(236);
  tft.drawCentreString(st, 120, 30, 4);
  tft.setTextPadding(0);
}

void cdDrawAll() {
  tft.fillScreen(C_BG);
  char tot[24], line[48];
  pc::formatTime(cd.totalMs / 1000.f, tot, sizeof tot);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawCentreString("EXPOSURE TIMER", 120, 6, 2);
  snprintf(line, sizeof line, "%s   total %s", cdLabel, tot);
  tft.drawCentreString(line, 120, 212, 2);
  cdLastText[0] = 0;
  cdDrawStatus();
  cdDrawTime(true);
  cdDrawButtons();
}

// Called when the user taps the final time on the Exposure or Pinhole screen.
void startCountdownScreen(float seconds, const char* filmName, Screen returnTo) {
  pc::cdSet(cd, seconds);
  snprintf(cdLabel, sizeof cdLabel, "%s", filmName);
  cdReturn = returnTo;
  screen = COUNTDOWN;
  digitalWrite(TFT_BL, HIGH);
  cdDrawAll();
}

// Run every loop() while the countdown screen is up.
void countdownTick() {
  if (cd.state == pc::Countdown::Running) {
    if (pc::cdUpdate(cd, millis())) {
      cdDoneAt = millis();
      cdLastText[0] = 0;
      cdDrawStatus();
      cdDrawTime(true);
      cdDrawButtons();
    } else {
      cdDrawTime(false);
    }
  } else if (cd.state == pc::Countdown::Done) {
    uint32_t el = millis() - cdDoneAt;
    if (el < 6000) {  // flash the backlight and digits for 6 s, then stay on
      digitalWrite(TFT_BL, (el / 250) % 2 ? LOW : HIGH);
      if (el % 250 < 40) cdDrawTime(true);
    } else {
      digitalWrite(TFT_BL, HIGH);
    }
  }
}

// Returns true when the user leaves the screen.
bool countdownTap(int x, int y) {
  if (y < CD_BTN_Y) return false;
  bool left = x < 120;
  switch (cd.state) {
    case pc::Countdown::Ready:
      if (left) {
        pc::cdStart(cd, millis());
        cdDrawStatus();
        cdDrawTime(true);
        cdDrawButtons();
        return false;
      }
      return true;
    case pc::Countdown::Running:
      if (left) pc::cdPause(cd, millis());
      else pc::cdReset(cd);
      break;
    case pc::Countdown::Paused:
      if (left) pc::cdStart(cd, millis());
      else pc::cdReset(cd);
      break;
    case pc::Countdown::Done:
      digitalWrite(TFT_BL, HIGH);
      return true;
  }
  cdDrawStatus();
  cdDrawTime(true);
  cdDrawButtons();
  return false;
}
