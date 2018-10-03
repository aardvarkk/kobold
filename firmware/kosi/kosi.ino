#define REQUIRESALARMS false

// Allow Arduino OTA
//#define OTA_ARDUINO

// Allow Web Browser Update
#define OTA_WEB_PUSH

#ifdef OTA_ARDUINO
#include <ArduinoOTA.h>
#endif

#include "constants.hpp"
#include "globals.hpp"
#include "led.hpp"
#include "log.hpp"
#include "state.hpp"
#include "storage.hpp"
#include "webserver.hpp"
#include "wifi.hpp"
#include "yield.hpp"

bool internet_settings_exist(Storage const& storage) {
  return storage.ssid_external[0] != 0;
}

String report_json(float temp) {
  String json;
  json += "{";
  json += "\"temp\":";
  json += String(temp);
  json += "}";
  _l(json);
  return json;  
}

int report_temperature(String const& report_url, String const& token, float temp, String& response) {
  _l("report_temperature");
  _l(report_url);
  _l(temp);

  int code = -1;

  digitalWrite(PIN_LED, ACTIVE);

  HTTPClient client;
  auto success = client.begin(report_url);
  if (success) {
    client.addHeader("Authorization", String("Bearer ") + token);
    client.addHeader("Content-Type", "application/json");
    code = client.POST(report_json(temp));
    _l(code);
    if (code < 0) {
      _l("http request failed");
      _l(HTTPClient::errorToString(code));
    } else {
      _l("http request succeeded");
      response = client.getString();
    }
  } else {
    _l("failed starting http client");
  }

  digitalWrite(PIN_LED, INACTIVE);

  client.end();
  return code;
}

int refresh_token(String const& url, String const& username, String const& password, String& token) {
  _l("refresh_token");
  _l(url);
//  _l(username);
//  _l(password);
  
  int code = -1;
  token = "";

  digitalWrite(PIN_LED, ACTIVE);

  HTTPClient client;
  auto success = client.begin(url);
  if (success) {
    client.setAuthorization(username.c_str(), password.c_str());
    code = client.POST("");
    _l(code);
    if (code < 0) {
      _l("http request failed");
      _l(HTTPClient::errorToString(code));
    } else {
      _l("http request succeeded");
      token = client.getString();
      _l(token);
    }
  } else {
    _l("failed to start HTTP client");
  }

  digitalWrite(PIN_LED, INACTIVE);

  client.end();
  return code;
}

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
      if (code < 0) {
        to_offline(now);
      } else {
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
            code = refresh_token(_storage.report_url + "token", KEY, SECRET, _storage.token);
            if (code == 200) {
              _l("successfully retrieved new token!");
              _l(_storage.token);
              serialize_storage(_storage, true);
            } else {
              _l("failed to retrieve new token!");
            }
            break;
          case 403:
            _l("no account");
            to_offline(now);
            break;
          default:
            _l("unrecognized code");
            to_offline(now);
            break;
        }
      }
    }
  }
}

// Return true when we've waited long enough since a previous occurrence of an event
bool period_elapsed(unsigned long last_occurrence, unsigned long now, unsigned long period) {
  return abs(now - last_occurrence) >= period;
}

// Enable or disable the relay
void set_relay(bool enabled) {
//  _l("set_relay");
//  _l(enabled);
  _l(enabled ? "relay ON" : "relay OFF");
  digitalWrite(PIN_RELAY, enabled ? ACTIVE : INACTIVE);
}

void reset_conversion() {
  _started_temp_req = false;
  _last_started_temp_req = 0;
}

bool process_conversion(unsigned long now, unsigned long conversion_period, float& temp) {
  // We're waiting for conversion to complete
  if (_started_temp_req) {
    if (period_elapsed(_last_started_temp_req, now, conversion_period)) {
      reset_conversion();

      if (_sensor_interface.isConversionComplete()) {
        _l("conversion complete");
        temp = _sensor_interface.getTempC(_sensor_address);
        return true;
      } else {
        _l("conversion incomplete");
      }
    }
  }
  // We haven't yet started conversion
  else {
    _l("requestTemperatures");
    _sensor_interface.requestTemperatures();
    _last_started_temp_req = now;
    _started_temp_req = true;
  }

  return false;
}

String encryption_type_string(uint8_t encryption_type) {
  switch (encryption_type) {
    case ENC_TYPE_WEP:  return "WEP";
    case ENC_TYPE_TKIP: return "TKIP";
    case ENC_TYPE_CCMP: return "CCMP";
    case ENC_TYPE_NONE: return "NONE";
    case ENC_TYPE_AUTO: return "AUTO";
    default: return "UNKNOWN";
  }
}

void log_wifi(Network const& network) {
  String line =
    network.ssid + " " +
    "(" + encryption_type_string(network.encryption_type) + ") " +
    "RSSI: " + network.rssi + " " +
    "CHANNEL: " + network.channel + " " +
    (network.is_hidden ? "HIDDEN!" : "");
  _l(line);
}

// Callback for networks being found
void on_scan_complete(int found) {
  _l("on_scan_complete");

  _num_found_networks = found;
  _l(_num_found_networks);

  for (int i = 0; i < _num_found_networks; ++i) {
    Network& network = _found_networks[i];
    WiFi.getNetworkInfo(
      i,
      network.ssid,
      network.encryption_type,
      network.rssi,
      network.bssid,
      network.channel,
      network.is_hidden
    );
    log_wifi(network);
  }

  WiFi.scanDelete();
}

void control_decision(float& cur_temp, float& tgt_temp) {
  bool less_than = cur_temp < tgt_temp;

  String decision;
  decision += String(cur_temp);
  decision += String(" is ") + (less_than ? "less than" : "greater than") + " ";
  decision += String(tgt_temp);
  _l(decision);

  set_relay(less_than);
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
    if (internet_settings_exist(_storage) && !FORCE_OFFLINE) {
      _l("internet settings exist");
      to_online(now);
    } else {
      _l("no internet settings");
    }
    _latest_user_action = now;
  }
}

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

void log_device_address(DeviceAddress& device_address) {
  char address[2 * sizeof(DeviceAddress) / sizeof(*device_address) + 1];
  sprintf(address, "%02X%02X%02X%02X%02X%02X%02X%02X",
    device_address[0], device_address[1], device_address[2], device_address[3],
    device_address[4], device_address[5], device_address[6], device_address[7]);
  _l(address);
}

void init_sensors() {
  _sensor_interface.begin();
  _sensor_interface.setResolution(SENSOR_ADC_BITS);
  _sensor_interface.setWaitForConversion(false);

  memset(_sensor_address, 0, sizeof(_sensor_address));
  for (auto i = 0; i < _sensor_interface.getDeviceCount(); ++i) {
    if (_sensor_interface.getAddress(_sensor_address, i)) {
      _l("temperature address found");
      log_device_address(_sensor_address);
    } else {
      _l("temperature address not found")
    }
  }

  _conversion_period = _sensor_interface.millisToWaitForConversion(SENSOR_ADC_BITS) * 1000;
  reset_conversion();
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
