// Shared "pages of big +/- rows" UI used by the Exposure and Pinhole screens.
// Included from main.cpp after the theme colors, RGB() and tft are defined.
// Layout (240x320): result panel on top (0..110), three rows of buttons, Home and page buttons at the bottom.
#pragma once

struct Row {
  const char* label;
  uint8_t page;
  bool smallFont;  // long values (film name, preset)
  void (*step)(int dir);
  void (*value)(char* out, int n);
};

struct RowPage {
  const Row* rows;
  int n;
  int pages;
  const char* const* pageNames;
  uint8_t page;
  int lastRow;  // row whose centre was last tapped (RP_CENTER)
};

enum RpAction { RP_NONE, RP_HOME, RP_PANEL, RP_CHANGED, RP_PAGE, RP_CENTER };

// 60 px wide x 50 px tall buttons with 6 px gaps; hit zones split in the middle of the gap.
const int RP_ROW_H = 50, RP_ROW_PITCH = 56, RP_BTN_W = 60, RP_ROWS_Y = 112, RP_PANEL_H = 110, RP_BTN_Y = 280;

static int rpRowY(const RowPage& rp, int i) {  // y of row i within its page
  int k = 0;
  for (int j = 0; j < i; j++)
    if (rp.rows[j].page == rp.rows[i].page) k++;
  return RP_ROWS_Y + k * RP_ROW_PITCH;
}

void rpDrawRow(const RowPage& rp, int i) {
  const Row& r = rp.rows[i];
  const uint16_t amber = RGB(255, 170, 40), btn = RGB(40, 46, 66);
  int y = rpRowY(rp, i);
  int cx = RP_BTN_W + 6, cw = 240 - 2 * (RP_BTN_W + 6);
  tft.fillRect(0, y, 240, RP_ROW_H, C_BG);
  tft.fillRoundRect(0, y, RP_BTN_W, RP_ROW_H, 10, btn);
  tft.fillRoundRect(240 - RP_BTN_W, y, RP_BTN_W, RP_ROW_H, 10, btn);
  // drawn as shapes: the big built-in font has no "+" glyph
  int my = y + RP_ROW_H / 2, lx = RP_BTN_W / 2, rx = 240 - RP_BTN_W / 2;
  tft.fillRect(lx - 11, my - 2, 22, 5, amber);
  tft.fillRect(rx - 11, my - 2, 22, 5, amber);
  tft.fillRect(rx - 2, my - 11, 5, 22, amber);
  tft.fillRoundRect(cx, y, cw, RP_ROW_H, 10, C_TILE);
  tft.setTextColor(C_DIM, C_TILE);
  tft.drawCentreString(r.label, 120, y + 4, 1);
  char v[32];
  r.value(v, sizeof v);
  tft.setTextColor(C_TEXT, C_TILE);
  int font = 4;
  if (tft.textWidth(v, 4) > cw - 6) font = 2;
  if (tft.textWidth(v, 2) > cw - 6) font = 1;
  tft.drawCentreString(v, 120, y + (font == 4 ? 19 : font == 2 ? 25 : 29), font);
}

void rpDrawRows(const RowPage& rp) {
  for (int i = 0; i < rp.n; i++)
    if (rp.rows[i].page == rp.page) rpDrawRow(rp, i);
}

void rpDrawButtons() {
  const uint16_t btn = RGB(40, 46, 66);
  tft.fillRoundRect(0, RP_BTN_Y, 80, 40, 10, btn);
  tft.fillRoundRect(86, RP_BTN_Y, 74, 40, 10, btn);
  tft.fillRoundRect(166, RP_BTN_Y, 74, 40, 10, btn);
  tft.setTextColor(C_TEXT, btn);
  tft.drawCentreString("Home", 40, RP_BTN_Y + 12, 2);
  tft.drawCentreString("<", 123, RP_BTN_Y + 8, 4);
  tft.drawCentreString(">", 203, RP_BTN_Y + 8, 4);
}

// Handles a tap. On RP_CHANGED the rows are already redrawn; the caller redraws its result panel.
// On RP_PAGE the page changed; the caller redraws the whole screen.
RpAction rpTap(RowPage& rp, int x, int y) {
  if (y < RP_PANEL_H) return RP_PANEL;
  if (y >= RP_BTN_Y) {
    if (x < 83) return RP_HOME;
    if (x < 163) rp.page = (rp.page + rp.pages - 1) % rp.pages;
    else rp.page = (rp.page + 1) % rp.pages;
    return RP_PAGE;
  }
  for (int i = 0; i < rp.n; i++) {
    if (rp.rows[i].page != rp.page) continue;
    int ry = rpRowY(rp, i);
    if (y < ry || y >= ry + RP_ROW_H) continue;
    int dir = x < RP_BTN_W + 3 ? -1 : (x >= 240 - RP_BTN_W - 3 ? +1 : 0);
    if (dir == 0) {
      rp.lastRow = i;
      return RP_CENTER;
    }
    rp.rows[i].step(dir);
    rpDrawRows(rp);  // a change can affect other rows (film resets film speed, preset sets focal and hole)
    return RP_CHANGED;
  }
  return RP_NONE;
}
