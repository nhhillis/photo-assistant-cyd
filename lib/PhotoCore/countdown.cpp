#include "countdown.h"
#include <stdio.h>

namespace pc {

static const uint32_t MAX_MS = 99UL * 3600UL * 1000UL;

void cdSet(Countdown& c, float seconds) {
  float ms = seconds * 1000.f;
  if (ms < 0.f) ms = 0.f;
  if (ms > (float)MAX_MS) ms = (float)MAX_MS;
  c.totalMs = (uint32_t)(ms + 0.5f);
  c.remainingMs = c.totalMs;
  c.lastMs = 0;
  c.state = Countdown::Ready;
}

void cdStart(Countdown& c, uint32_t nowMs) {
  if (c.state != Countdown::Ready && c.state != Countdown::Paused) return;
  if (c.remainingMs == 0) return;
  c.lastMs = nowMs;
  c.state = Countdown::Running;
}

bool cdUpdate(Countdown& c, uint32_t nowMs) {
  if (c.state != Countdown::Running) return false;
  uint32_t dt = nowMs - c.lastMs;  // unsigned: survives millis() wraparound
  c.lastMs = nowMs;
  if (dt >= c.remainingMs) {
    c.remainingMs = 0;
    c.state = Countdown::Done;
    return true;
  }
  c.remainingMs -= dt;
  return false;
}

void cdPause(Countdown& c, uint32_t nowMs) {
  if (c.state != Countdown::Running) return;
  cdUpdate(c, nowMs);
  if (c.state == Countdown::Running) c.state = Countdown::Paused;
}

void cdReset(Countdown& c) {
  c.remainingMs = c.totalMs;
  c.state = Countdown::Ready;
}

float cdFraction(const Countdown& c) {
  if (c.totalMs == 0) return 1.f;
  return 1.f - (float)c.remainingMs / (float)c.totalMs;
}

void formatCountdown(uint32_t ms, char* out, int n) {
  if (ms < 10000UL) {
    uint32_t tenths = (ms + 99) / 100;  // round up
    snprintf(out, n, "%u.%u", (unsigned)(tenths / 10), (unsigned)(tenths % 10));
    return;
  }
  uint32_t s = (ms + 999) / 1000;  // round up to whole seconds
  if (s < 60) snprintf(out, n, "%u", (unsigned)s);
  else if (s < 3600) snprintf(out, n, "%u:%02u", (unsigned)(s / 60), (unsigned)(s % 60));
  else snprintf(out, n, "%u:%02u:%02u", (unsigned)(s / 3600), (unsigned)((s / 60) % 60), (unsigned)(s % 60));
}

}  // namespace pc
