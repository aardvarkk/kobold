#define REQUIRESALARMS false

// Allow Arduino OTA
//#define OTA_ARDUINO

// Allow Web Browser Update
#define OTA_WEB_PUSH

#ifdef OTA_ARDUINO
#include <ArduinoOTA.h>
#endif

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

#ifdef OTA_ARDUINO
void init_ota() {
  _l("init_ota");

  ArduinoOTA.onStart([]() {
    _l("ArduinoOTA.onStart");
  });

  ArduinoOTA.onEnd([]() {
    _l("ArduinoOTA.onEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    _l("ArduinoOTA.onProgress");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    _l("ArduinoOTA.onError");

    if (error == OTA_AUTH_ERROR) {
      _l("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      _l("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      _l("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      _l("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      _l("End Failed");
    }
  });

  ArduinoOTA.begin();
}
#endif

void setup() {
  #ifdef OTA_ARDUINO
  init_ota();
  #endif

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

  #ifdef OTA_ARDUINO
  ArduinoOTA.handle();
  #endif

  switch (_mode) {
    case RunMode::ONLINE:
      process_online();
      break;
    case RunMode::OFFLINE:
      process_offline();
      break;
  }
}
