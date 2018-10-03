#define REQUIRESALARMS false

// Allow Web Browser Update
#define OTA_WEB_PUSH

#include "client.hpp"
#include "constants.hpp"
#include "control.hpp"
#include "sensors.hpp"
#include "globals.hpp"
#include "led.hpp"
#include "log.hpp"
#include "relay.hpp"
#include "state.hpp"
#include "storage.hpp"
#include "webserver.hpp"
#include "wifi.hpp"
#include "yield.hpp"

void init_serial() {
  Serial.begin(SERIAL_SPEED, SERIAL_CONFIG);
  while (!Serial) { yield_delay(); }
}

void init_pins() {
  _l("init_pins");
  pinMode(PIN_LED,      OUTPUT);
  pinMode(PIN_RELAY,    OUTPUT);
  pinMode(PIN_ONE_WIRE, INPUT);
}

void setup() {
  init_serial();
  init_pins();
  init_sensors();
  
  bool match = serialize_storage(_storage, false);
  if (!match) {
    first_time_storage(_storage);
  }

  if (internet_settings_exist(_storage)) {
    to_online(micros());
  } else {
    to_offline(micros());
  }
}

void loop() {
  switch (_mode) {
    case RunMode::ONLINE:
      process_online();
      break;
    case RunMode::OFFLINE:
      process_offline();
      break;
  }
}
