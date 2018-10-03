#include "constants.hpp"
#include "log.hpp"
#include "relay.hpp"

// Enable or disable the relay
void set_relay(bool enabled) {
//  _l("set_relay");
//  _l(enabled);
  _l(enabled ? "relay ON" : "relay OFF");
  digitalWrite(PIN_RELAY, enabled ? ACTIVE : INACTIVE);
}

