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
      auto code = report_temperature(_storage.report_url, _storage.token, temp, response);
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
          _storage.token = "";
          serialize_storage(_storage, true);
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
    auto status = WiFi.scanComplete();
    if (status == WIFI_SCAN_FAILED) {
      _l("scanning wifi");
      WiFi.scanNetworksAsync(on_scan_complete, SCAN_FOR_HIDDEN);
    }
    _last_wifi_scan = now;
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
    _l("retry online");
    if (is_online_possible(_storage) && !FORCE_OFFLINE) {
      to_online(now);
    } else {
      _l("online not possible");
    }
    _latest_user_action = now;
  }
}

void to_online(unsigned long now) {
  _l("to_online");

  if (FORCE_OFFLINE || !is_online_possible(_storage)) {
    _l("force_offline");
    to_offline(now);
    return;
  } 
  
  reset_timers(now);
  digitalWrite(PIN_LED, INACTIVE);

  deinit_ap();
  deinit_webserver();

  // Wait before going online so we can get back on normal network to start local server
  delay(TESTING_WAIT);

  if (init_sta(_storage.ssid_external, _storage.password_external)) {
    _l(_storage.token);

    if (!_storage.token.length()) {
      _l("no token, obtaining");

      auto code = link_device(
        _storage.report_url + "link", 
        _link_email, 
        _link_password, 
        _storage.token
      );

      // Immediately forget the linking regardless
      _link_email = "";
      _link_password = "";

      switch (code) {
        case 200:
          _l("successfully retrieved new token!");
          _l(_storage.token);
          set_mode(RunMode::ONLINE);
        break;

        default:
          _l("failed to retrieve new token!");
          _storage.token = "";
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

  init_ap(_storage.ssid_internal, _storage.password_internal);
  init_webserver(_server);

  set_mode(RunMode::OFFLINE);
}

void set_mode(RunMode mode) {
  _mode = mode;
}

// To go online, we need an external SSID & password,
// and EITHER a linking email and password to obtain token when going online 
//         OR a valid existing token
bool is_online_possible(Storage const& storage) {
  _l("is_online_possible");

  if (storage.ssid_external.length() &&
      storage.password_external.length() && (
        (_link_email.length() && _link_password.length()) ||
        storage.token.length()
      )) {
    _l("possible!")
    return true;
  }

  _l("impossible!");
  return false;
}