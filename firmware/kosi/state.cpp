#include "client.hpp"
#include "control.hpp"
#include "globals.hpp"
#include "led.hpp"
#include "log.hpp"
#include "relay.hpp"
#include "sensors.hpp"
#include "state.hpp"
#include "storage.hpp"
#include "timers.hpp"
#include "webserver.hpp"
#include "wifi.hpp"

void process_online() {
  auto now = micros();

  // Time to read from the sensor again
  if (period_elapsed(_last_checked_temp, now, PERIOD_CHECK_TEMP)) {
    float temp;
    auto valid_temp = process_conversion(now, _conversion_period, temp);

    if (valid_temp) {
      _last_checked_temp = now;

      String response;
      auto code = report_temperature(_storage.report_url, _token, temp, response);
      switch (code) {
        case 200:
          _l("cool");
          set_relay(false);
          break;
        case 201:
          _l("heat");
          set_relay(true);
          break;
        case 205:
          _l("manual disconnect request");
          to_offline(now);
          break;
        case 401:
          _l("bad token!");
          _token = "";
          to_offline(now);
          break;
        default:
          _l("unrecognized code");
          _l(code)
          to_offline(now);
          break;
      }
    }
  }
}

void process_offline() {
  auto now = micros();

  // Run the webserver
  _server.handleClient();

  // Time to scan for WiFi
  if (period_elapsed(_last_wifi_scan, now, PERIOD_WIFI_SCAN)) {
    _last_wifi_scan = now;
    auto status = WiFi.scanComplete();
    if (status == WIFI_SCAN_FAILED) {
      _l("scanning wifi");
      WiFi.scanNetworksAsync(on_scan_complete, SCAN_FOR_HIDDEN);
    }
  }

  // Time to read from the sensor again
  if (period_elapsed(_last_checked_temp, now, PERIOD_CHECK_TEMP)) {
    float temp;
    auto valid_temp = process_conversion(now, _conversion_period, temp);

    if (valid_temp) {
      _last_checked_temp = now;
      control_decision(temp, _storage.setpoint);
    }
  }

  // Time to blink again
  if (period_elapsed(_last_changed_blink, now, _blink_state ? PERIOD_BLINK_ON : PERIOD_BLINK_OFF)) {
    set_blink(!_blink_state, now);
  }

  // To retry online again
  if (period_elapsed(_latest_user_action, now, PERIOD_RETRY_ONLINE)) {
    _latest_user_action = now;
    _l("retry online");
    to_online(now);
  }
}

void to_online(unsigned long now) {
  _l("to_online");

  if (FORCE_OFFLINE) {
    _l("force_offline");
    to_offline(now);
    return;
  } 
  
  if (!is_online_possible(_storage)) {
    _l("online not possible!");
    to_offline(now);
    return;
  }

  reset_timers(now);
  digitalWrite(PIN_LED, INACTIVE);

  deinit_webserver();
  deinit_ap();

  if (init_sta(_storage.ssid_external, _storage.password_external)) {
    _l(_token);

    if (!_token.length()) {
      _l("no token, obtaining");

      auto code = link_device(
        _storage.report_url + "link", 
        _storage.link_email, 
        _storage.link_password, 
        _token
      );

      switch (code) {
        case 200:
          _l("successfully linked -- retrieved new token!");
          _l(_token);
          set_mode(RunMode::ONLINE);
        break;

        default:
          _l("failed to link!");
          _l("resetting auth settings");
          _token = "";
          serialize_storage(_storage, true);
          to_offline(now);
          break;
      }

      // Success or failure, we save storage to keep token accurate
      serialize_storage(_storage, true);    
    } else {
      _l("existing token");
      set_mode(RunMode::ONLINE);
    }
  } else {
    to_offline(now);
  }
}

void to_offline(unsigned long now) {
  _l("to_offline");

  reset_timers(now);
  set_blink(true, now);

  deinit_sta();
  init_ap(_storage.ssid_internal, _storage.password_internal);
  init_webserver(_server);

  set_mode(RunMode::OFFLINE);
}

void set_mode(RunMode mode) {
  _mode = mode;
}

bool wifi_setup(Storage const& storage) {
  return storage.ssid_external.length() && storage.password_external.length();
}

bool account_setup(Storage const& storage) {
  return storage.link_email.length() && storage.link_password.length();
}

bool token_exists() {
  return _token.length();
}

// To go online, we need an external SSID & password,
// and EITHER a linking email and password to obtain token when going online 
//         OR a valid existing token
bool is_online_possible(Storage const& storage) {
  _l("is_online_possible");

  if (wifi_setup(_storage) && (account_setup(_storage) || token_exists())) {
    _l("possible!")
    return true;
  } else {
    _l("impossible!");
    return false;
  }
}