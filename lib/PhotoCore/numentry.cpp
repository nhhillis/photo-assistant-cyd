#include "numentry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace pc {

static const int MAX_LEN = 12;
static const int MAX_DECIMALS = 5;

void neInit(NumEntry& e, float value, float lo, float hi) {
  e.lo = lo;
  e.hi = hi;
  snprintf(e.text, sizeof e.text, "%.5f", (double)value);
  // trim trailing zeros, then a dangling '.'
  int n = (int)strlen(e.text);
  while (n > 0 && e.text[n - 1] == '0' && strchr(e.text, '.')) e.text[--n] = 0;
  if (n > 0 && e.text[n - 1] == '.') e.text[--n] = 0;
  if (strcmp(e.text, "-0") == 0) strcpy(e.text, "0");
  e.len = (uint8_t)strlen(e.text);
}

bool neKey(NumEntry& e, char c) {
  if (c == '-') {
    if (e.text[0] == '-') {
      memmove(e.text, e.text + 1, e.len);  // includes the terminator
      e.len--;
      return true;
    }
    if (e.len >= MAX_LEN) return false;
    memmove(e.text + 1, e.text, e.len + 1);
    e.text[0] = '-';
    e.len++;
    return true;
  }
  if (c == '.') {
    if (strchr(e.text, '.') || e.len >= MAX_LEN - 1) return false;
    if (e.len == 0 || (e.len == 1 && e.text[0] == '-')) {
      e.text[e.len++] = '0';
    }
    e.text[e.len++] = '.';
    e.text[e.len] = 0;
    return true;
  }
  if (c < '0' || c > '9') return false;
  if (e.len >= MAX_LEN) return false;
  const char* dot = strchr(e.text, '.');
  if (dot && (int)strlen(dot + 1) >= MAX_DECIMALS) return false;
  // a lone leading zero is replaced, so "0" then "5" gives "5", not "05"
  if (strcmp(e.text, "0") == 0) e.len = 0;
  else if (strcmp(e.text, "-0") == 0) e.len = 1;
  e.text[e.len++] = c;
  e.text[e.len] = 0;
  return true;
}

void neBackspace(NumEntry& e) {
  if (e.len > 0) e.text[--e.len] = 0;
}

bool neValue(const NumEntry& e, float& out) {
  if (e.len == 0) return false;
  char* end = nullptr;
  double v = strtod(e.text, &end);
  if (end == e.text || *end != 0) return false;
  if (e.len == 1 && (e.text[0] == '-' || e.text[0] == '.')) return false;
  if (v < e.lo || v > e.hi) return false;
  out = (float)v;
  return true;
}

}  // namespace pc
