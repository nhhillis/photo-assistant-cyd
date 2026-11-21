// Exposure countdown timer state. Pure C++: the caller passes in millis(), so it can be tested on a PC.
#pragma once
#include <stdint.h>

namespace pc {

struct Countdown {
  enum State : uint8_t { Ready, Running, Paused, Done };
  State state;
  uint32_t totalMs;
  uint32_t remainingMs;
  uint32_t lastMs;  // millis() at the last update while Running
};

void cdSet(Countdown& c, float seconds);  // Ready with the full time (capped at 99 h)
void cdStart(Countdown& c, uint32_t nowMs);   // Ready or Paused -> Running
void cdPause(Countdown& c, uint32_t nowMs);   // Running -> Paused (time up to now is counted)
void cdReset(Countdown& c);                   // back to Ready with the full time
bool cdUpdate(Countdown& c, uint32_t nowMs);  // true exactly once, when the time runs out
float cdFraction(const Countdown& c);         // 0..1 elapsed

// "4:05", "1:12:00", "45", and tenths under 10 s ("9.4"). Rounds UP so it never shows 0 before it is done.
void formatCountdown(uint32_t ms, char* out, int n);

}  // namespace pc
