#include <Arduino.h>

#include "constants.hpp"
#include "yield.hpp"

void yield_delay() {
  delay(YIELD_DELAY_MS);
}

