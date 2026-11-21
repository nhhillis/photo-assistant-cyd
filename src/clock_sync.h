// WiFi + NTP clock sync (non-blocking state machine). Included from main.cpp.
// Credentials live in src/wifi_secrets.h (gitignored); copy wifi_secrets.example.h to create it.
// WiFi is only on while syncing; afterwards the ESP32 system clock keeps time until the next reboot.
#pragma once
#include <WiFi.h>
#include <time.h>

#if __has_include("wifi_secrets.h")
#include "wifi_secrets.h"
#else
struct WifiNet { const char* ssid; const char* pass; };
static const WifiNet WIFI_NETS[] = {};
#define WIFI_NET_COUNT 0
#endif

// POSIX TZ string for US Central with US DST rules (same rule PhotoCore's Tz uses).
#define CLOCK_TZ "CST6CDT,M3.2.0,M11.1.0"

enum ClockState { CK_NO_WIFI_CONFIG, CK_CONNECTING, CK_WAIT_TIME, CK_SYNCED, CK_FAILED };

ClockState ckState = CK_NO_WIFI_CONFIG;
static int ckNet = 0;
static uint32_t ckT0 = 0;

static void ckStartConnect() {
  WiFi.mode(WIFI_STA);
  Serial.printf("[clock] connecting to network %d (\"%s\")\n", ckNet, WIFI_NETS[ckNet].ssid);
  WiFi.begin(WIFI_NETS[ckNet].ssid, WIFI_NETS[ckNet].pass);
  ckT0 = millis();
  ckState = CK_CONNECTING;
}

static void ckWifiOff() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

// Start (or restart) a sync attempt through the configured networks in order.
void clockBegin() {
  Serial.printf("[clock] %u network(s) configured\n", (unsigned)WIFI_NET_COUNT);
  if (WIFI_NET_COUNT == 0) {
    ckState = CK_NO_WIFI_CONFIG;
    return;
  }
  ckNet = 0;
  ckStartConnect();
}

bool clockBusy() { return ckState == CK_CONNECTING || ckState == CK_WAIT_TIME; }
bool clockSynced() { return ckState == CK_SYNCED; }

// Call every loop(). Returns true when the state changed (so the UI can redraw its status).
bool clockTick() {
  ClockState before = ckState;
  if (ckState == CK_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      configTzTime(CLOCK_TZ, "pool.ntp.org", "time.nist.gov");
      ckT0 = millis();
      ckState = CK_WAIT_TIME;
    } else if (millis() - ckT0 > 12000) {
      if (++ckNet < WIFI_NET_COUNT) ckStartConnect();
      else {
        ckWifiOff();
        ckState = CK_FAILED;
      }
    }
  } else if (ckState == CK_WAIT_TIME) {
    if (time(nullptr) > 1700000000L) {  // later than Nov 2023 = a real NTP time
      ckWifiOff();
      ckState = CK_SYNCED;
    } else if (millis() - ckT0 > 10000) {
      ckWifiOff();
      ckState = CK_FAILED;
    }
  }
  if (ckState != before) Serial.printf("[clock] state %d -> %d (wifi status %d)\n", (int)before, (int)ckState, (int)WiFi.status());
  return ckState != before;
}

// Short status for the Home footer.
void clockStatusText(char* out, int n) {
  switch (ckState) {
    case CK_NO_WIFI_CONFIG: snprintf(out, n, "Clock: no WiFi set up"); break;
    case CK_CONNECTING:
    case CK_WAIT_TIME: snprintf(out, n, "Clock: syncing..."); break;
    case CK_FAILED: snprintf(out, n, "Clock: sync failed - tap"); break;
    case CK_SYNCED: {
      struct tm lt;
      time_t now = time(nullptr);
      localtime_r(&now, &lt);
      snprintf(out, n, "Clock: synced %d:%02d %s", lt.tm_hour % 12 == 0 ? 12 : lt.tm_hour % 12, lt.tm_min, lt.tm_hour < 12 ? "AM" : "PM");
      break;
    }
  }
}

// Today's local date. If the clock has never synced, falls back to the firmware build date
// (approximate) and sets *approx so the UI can say so.
void clockToday(int& y, int& m, int& d, bool& approx) {
  if (ckState == CK_SYNCED) {
    struct tm lt;
    time_t now = time(nullptr);
    localtime_r(&now, &lt);
    y = lt.tm_year + 1900; m = lt.tm_mon + 1; d = lt.tm_mday;
    approx = false;
    return;
  }
  static const char* MON = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char* bd = __DATE__;  // "Nov 21 2026"
  m = 1;
  for (int i = 0; i < 12; i++)
    if (strncmp(bd, MON + i * 3, 3) == 0) m = i + 1;
  d = atoi(bd + 4);
  y = atoi(bd + 7);
  approx = true;
}
