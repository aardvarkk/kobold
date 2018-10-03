#include <Arduino.h>

#include "control.hpp"
#include "log.hpp"
#include "relay.hpp"

void control_decision(float& cur_temp, float& tgt_temp) {
  bool less_than = cur_temp < tgt_temp;

  String decision;
  decision += String(cur_temp);
  decision += String(" is ") + (less_than ? "less than" : "greater than") + " ";
  decision += String(tgt_temp);
  _l(decision);

  set_relay(less_than);
}
