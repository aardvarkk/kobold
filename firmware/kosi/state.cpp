#include "globals.hpp"
#include "led.hpp"
#include "log.hpp"
#include "state.hpp"
#include "webserver.hpp"
#include "wifi.hpp"

void set_mode(RunMode mode) {
  _mode = mode;
}

void to_offline(unsigned long now) {
  _l("to_offline");

  reset_timers(now);
  set_blink(true, now);

  init_ap(_storage.ssid_internal, _storage.password_internal);
  init_webserver(_server);

  set_mode(RunMode::OFFLINE);
}

void to_online(unsigned long now) {
  _l("to_online");

  if (FORCE_OFFLINE) {
    _l("force_offline");
    to_offline(now);
  } else {
    reset_timers(now);
    digitalWrite(PIN_LED, INACTIVE);

    deinit_ap();
    deinit_webserver();

    if (init_sta(_storage.ssid_external, _storage.password_external)) {
      set_mode(RunMode::ONLINE);        
    } else {
      to_offline(now);
    }
  }
}

void reset_timers(unsigned long now) {
  _l("reset_timers");
  _last_wifi_scan     = now - PERIOD_WIFI_SCAN;
  _last_checked_temp  = now - PERIOD_CHECK_TEMP;
  _latest_user_action = now;
}
