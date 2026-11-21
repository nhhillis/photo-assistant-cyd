// Numeric keypad entry (for typing a latitude / longitude). Pure C++ so it can be tested on a PC.
#pragma once
#include <stdint.h>

namespace pc {

struct NumEntry {
  char text[16];
  uint8_t len;
  float lo, hi;  // accepted range
};

void neInit(NumEntry& e, float value, float lo, float hi);  // shows value with up to 5 decimals
bool neKey(NumEntry& e, char c);   // '0'-'9', '.', '-' (toggles the sign). false if the key was ignored
void neBackspace(NumEntry& e);
bool neValue(const NumEntry& e, float& out);  // false if empty, incomplete ("-", ".") or out of range

}  // namespace pc
