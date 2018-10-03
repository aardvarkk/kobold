#include "constants.hpp"
#include "globals.hpp"
#include "led.hpp"

// Turn on or off the LED
void set_blink(bool on, unsigned long now) {
  _blink_state = on;
  digitalWrite(PIN_LED, on ? ACTIVE : INACTIVE);
  _last_changed_blink = now;
}

