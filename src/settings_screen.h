// Settings screen (clock time zone, sync now, WiFi networks) and the on-screen keyboard for typing
// WiFi names and passwords. Included from main.cpp after rowpage.h and clock_sync.h.
#pragma once

// ---- settings rows
static void svZone(char* o, int n) { snprintf(o, n, "%s", pc::ZONES[tzIdx].name); }
static void svSync(char* o, int n) {
  switch (ckState) {
    case CK_CONNECTING:
    case CK_WAIT_TIME: snprintf(o, n, "syncing..."); break;
    case CK_NO_WIFI_CONFIG: snprintf(o, n, "add a network"); break;
    default: snprintf(o, n, "tap to sync"); break;
  }
}
static void svStatus(char* o, int n) { clockStatusText(o, n); }
static void svNet(char* o, int n, int i) { snprintf(o, n, "%s", savedNets[i].ssid[0] ? savedNets[i].ssid : "(empty)"); }
static void svNet0(char* o, int n) { svNet(o, n, 0); }
static void svNet1(char* o, int n) { svNet(o, n, 1); }
static void svNet2(char* o, int n) { svNet(o, n, 2); }

const Row setRows[] = {
    // page 0: clock
    {"TIME ZONE", 0, true,
     [](int d) {
       tzIdx = (tzIdx + pc::ZONE_COUNT + d) % pc::ZONE_COUNT;
       saveTz();
       clockApplyZone();
     },
     svZone},
    {"SYNC CLOCK NOW", 0, false, [](int) {}, svSync},
    {"CLOCK", 0, true, [](int) {}, svStatus},
    // page 1: WiFi (tap the name to edit; empty name + OK clears the slot)
    {"WIFI NETWORK 1", 1, true, [](int) {}, svNet0},
    {"WIFI NETWORK 2", 1, true, [](int) {}, svNet1},
    {"WIFI NETWORK 3", 1, true, [](int) {}, svNet2},
};
const char* const setPageNames[] = {"CLOCK", "WIFI (tap to edit)"};
RowPage setPage = {setRows, sizeof(setRows) / sizeof(setRows[0]), 2, setPageNames, 0, 0};

void drawSettingsPanel() {
  tft.fillRect(0, 0, 240, RP_PANEL_H, C_BG);
  char pg[32], t[40];
  snprintf(pg, sizeof pg, "%d/%d %s", setPage.page + 1, setPage.pages, setPageNames[setPage.page]);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawString("SETTINGS", 6, 6, 2);
  tft.drawRightString(pg, 234, 10, 1);
  tft.setTextColor(C_TEXT, C_BG);
  time_t now = time(nullptr);
  if (now > 1700000000L) {
    struct tm lt;
    localtime_r(&now, &lt);
    strftime(t, sizeof t, "%I:%M %p", &lt);
    tft.setTextColor(RGB(60, 230, 110), C_BG);
    tft.drawCentreString(t, 120, 34, 4);
    strftime(t, sizeof t, "%a %b %d  %Z", &lt);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawCentreString(t, 120, 66, 2);
  } else {
    tft.setTextColor(RGB(255, 90, 70), C_BG);
    tft.drawCentreString("Clock not set", 120, 40, 4);
  }
  tft.setTextColor(C_DIM, C_BG);
  clockStatusText(t, sizeof t);
  tft.drawCentreString(t, 120, 90, 1);
}

void drawSettings() {
  tft.fillScreen(C_BG);
  rpDrawRows(setPage);
  drawSettingsPanel();
  rpDrawButtons();
}

// ---- keyboard
pc::TextEntry kbEntry;
int kbSlot = 0, kbField = 0, kbLayerIdx = 0;  // field 0 = network name, 1 = password
char kbPendingSsid[33];

const int KB_X0 = 4, KB_CW = 29, KB_Y0 = 66, KB_RH = 42, KB_BTN_Y = 280;
// bottom bar: Cancel | layer | Space | Del | OK
const int KB_BX[5] = {0, 53, 98, 153, 196}, KB_BW[5] = {50, 42, 52, 40, 44};

void drawKbDisplay() {
  tft.fillRect(0, 0, 240, KB_Y0 - 2, C_BG);
  char lab[40];
  snprintf(lab, sizeof lab, kbField == 0 ? "WIFI NAME  (network %d)" : "PASSWORD  (network %d)", kbSlot + 1);
  tft.setTextColor(C_DIM, C_BG);
  tft.drawString(lab, 6, 2, 1);
  tft.fillRoundRect(2, 14, 236, 46, 8, C_TILE);
  // show the end of the text if it is wider than the box
  char shown[72];
  snprintf(shown, sizeof shown, "%s_", kbEntry.text);
  const char* p = shown;
  while (*p && tft.textWidth(p, 4) > 224) p++;
  tft.setTextColor(C_TEXT, C_TILE);
  tft.drawString(p, 8, 24, 4);
}

void drawKbKeys() {
  const uint16_t btn = RGB(40, 46, 66);
  const char* k = pc::kbLayer(kbLayerIdx);
  for (int i = 0; i < pc::KB_KEYS; i++) {
    int x = KB_X0 + (i % pc::KB_COLS) * KB_CW, y = KB_Y0 + (i / pc::KB_COLS) * KB_RH;
    tft.fillRoundRect(x, y, KB_CW - 2, KB_RH - 4, 5, btn);
    tft.setTextColor(C_TEXT, btn);
    char s[2] = {k[i], 0};
    tft.drawCentreString(s, x + (KB_CW - 2) / 2, y + 11, 2);
  }
}

void drawKbBar() {
  const uint16_t btn = RGB(40, 46, 66), ok = RGB(30, 120, 70);
  const char* layerName[] = {"ABC", "#+=", "abc"};  // label = the layer you get by tapping
  const char* labels[5] = {"Cancel", layerName[kbLayerIdx], "Space", "Del", "OK"};
  for (int i = 0; i < 5; i++) {
    uint16_t bg = i == 4 ? ok : btn;
    tft.fillRoundRect(KB_BX[i], KB_BTN_Y, KB_BW[i], 40, 8, bg);
    tft.setTextColor(TFT_WHITE, bg);
    tft.drawCentreString(labels[i], KB_BX[i] + KB_BW[i] / 2, KB_BTN_Y + 12, i == 0 ? 1 : 2);
  }
}

void drawKeyboard() {
  tft.fillScreen(C_BG);
  drawKbDisplay();
  drawKbKeys();
  drawKbBar();
}

void startNetEdit(int slot) {
  kbSlot = slot;
  kbField = 0;
  kbLayerIdx = 0;
  pc::teInit(kbEntry, savedNets[slot].ssid, 32);
  screen = KEYBOARD;
  drawKeyboard();
}

// Returns true when the keyboard closes.
bool keyboardTap(int x, int y) {
  if (y >= KB_BTN_Y) {
    int b = -1;
    for (int i = 0; i < 5; i++)
      if (x >= KB_BX[i] - 2 && x < KB_BX[i] + KB_BW[i] + 2) b = i;
    switch (b) {
      case 0: return true;  // cancel
      case 1:
        kbLayerIdx = (kbLayerIdx + 1) % pc::KB_LAYERS;
        drawKbKeys();
        drawKbBar();
        return false;
      case 2: pc::teAppend(kbEntry, ' '); break;
      case 3: pc::teBackspace(kbEntry); break;
      case 4:
        if (kbField == 0) {
          if (kbEntry.len == 0) {  // empty name + OK = clear this slot
            savedNets[kbSlot].ssid[0] = 0;
            savedNets[kbSlot].pass[0] = 0;
            saveNet(kbSlot);
            return true;
          }
          snprintf(kbPendingSsid, sizeof kbPendingSsid, "%s", kbEntry.text);
          bool same = strcmp(kbPendingSsid, savedNets[kbSlot].ssid) == 0;
          pc::teInit(kbEntry, same ? savedNets[kbSlot].pass : "", 63);  // keep the old password if only retyping the same name
          kbField = 1;
          kbLayerIdx = 0;
          drawKeyboard();
          return false;
        }
        snprintf(savedNets[kbSlot].ssid, sizeof savedNets[kbSlot].ssid, "%s", kbPendingSsid);
        snprintf(savedNets[kbSlot].pass, sizeof savedNets[kbSlot].pass, "%s", kbEntry.text);
        saveNet(kbSlot);
        clockBegin();  // try the new network straight away
        return true;
      default: return false;
    }
    drawKbDisplay();
    return false;
  }
  if (y < KB_Y0) return false;
  int col = (x - KB_X0) / KB_CW, row = (y - KB_Y0) / KB_RH;
  if (col < 0 || col >= pc::KB_COLS || row < 0 || row >= pc::KB_ROWS) return false;
  pc::teAppend(kbEntry, pc::kbLayer(kbLayerIdx)[row * pc::KB_COLS + col]);
  drawKbDisplay();
  return false;
}

// ---- settings taps. Returns true when the user leaves (Home).
bool settingsTap(int x, int y) {
  switch (rpTap(setPage, x, y)) {
    case RP_HOME: return true;
    case RP_CHANGED: drawSettings(); return false;
    case RP_PAGE: drawSettings(); return false;
    case RP_CENTER:
      if (setPage.lastRow == 1) {
        clockBegin();
        drawSettings();
      } else if (setPage.lastRow >= 3) {
        startNetEdit(setPage.lastRow - 3);
      }
      return false;
    default: return false;
  }
}
