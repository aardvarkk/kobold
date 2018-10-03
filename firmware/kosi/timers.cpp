#include <Arduino.h>

#include "constants.hpp"
#include "globals.hpp"
#include "log.hpp"
#include "timers.hpp"

void yield_delay() {
  delay(YIELD_DELAY_MS);
}

void reset_timers(unsigned long now) {
  _l("reset_timers");
  _last_wifi_scan     = now - PERIOD_WIFI_SCAN;
  _last_checked_temp  = now - PERIOD_CHECK_TEMP;
  _latest_user_action = now;
}

// Return true when we've waited long enough since a previous occurrence of an event
bool period_elapsed(unsigned long last_occurrence, unsigned long now, unsigned long period) {
  return abs(now - last_occurrence) >= period;
}
