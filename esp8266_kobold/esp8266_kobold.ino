// DEFINES
#define REQUIRESALARMS false

#define log(x) {     \
  Serial.println(x); \
  _log_history[_log_history_idx] = String(x); \
  _log_history_idx = (_log_history_idx + 1) % LOG_HISTORY_LENGTH; \
  _valid_log_history = min(_valid_log_history + 1, LOG_HISTORY_LENGTH); \
}

#include <assert.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// CONSTANTS
const uint8_t  PIN_RELAY = 12;
const uint8_t  PIN_LED = 13;
const uint8_t  PIN_ONE_WIRE = 14;
const long     SERIAL_SPEED = 115200;
const SerialConfig SERIAL_CONFIG = SERIAL_8N1;
const int      MAGIC_SIZE = 4;
const char     MAGIC[MAGIC_SIZE] = { 'K', 'B', 'L', 'D' };
const uint8_t  VERSION = 1;
const int      SSID_MAX = 32;
const int      PASSPHRASE_MAX = 64;
const int      EEPROM_OFFSET = 0xF; // Had trouble reading address 1 after restart, so offset from start a bit
const int      EEPROM_SIZE = 512;
const float    DEFAULT_TEMP = 18.0;
const uint8_t  SENSOR_ADC_BITS = 12;
const char*    AP_SSID = "abcd";
const char*    AP_PASSPHRASE = "thereisnospoon";
const int      YIELD_DELAY_MS = 20;
const uint16_t SERVER_PORT = 80;
const int      LOG_HISTORY_LENGTH = 0x20;
const uint8_t  MAX_NETWORKS = 0x40;
const bool     SCAN_FOR_HIDDEN = false;

const unsigned long PERIOD_WIFI_SCAN = 3e7; // 3e7 = 30 seconds
const unsigned long PERIOD_BLINK_OFF = 800000;
const unsigned long PERIOD_BLINK_ON  = 100000;
//const unsigned long PERIOD_CHECK_TEMP = 5e6; // 5e6 = 5 seconds
const unsigned long PERIOD_CHECK_TEMP = 3e7; // 3e7 = 30 seconds
//const unsigned long PERIOD_CHECK_TEMP = 3e8; // 3e8 = 5 minutes
const unsigned long PERIOD_RETRY_ONLINE = 6e7; // 6e7 = 1 minute

// TYPES
enum class RunMode {
  OFFLINE,
  ONLINE
};

struct Storage {
  char    magic[MAGIC_SIZE];
  uint8_t version;
  char    ssid[SSID_MAX];
  char    passphrase[PASSPHRASE_MAX];
  float   saved_temp;
};

struct Network {
  String   ssid;
  uint8_t  encryption_type;
  int32_t  rssi;
  uint8_t* bssid;
  int32_t  channel;
  bool     is_hidden;  
};

// GLOBAL VARIABLES
RunMode           _mode;
Storage           _storage;
unsigned long     _last_wifi_scan     = -PERIOD_WIFI_SCAN;
unsigned long     _last_checked_temp  = -PERIOD_CHECK_TEMP;
unsigned long     _last_changed_blink = -PERIOD_BLINK_OFF;
unsigned long     _latest_user_action = -PERIOD_RETRY_ONLINE;
unsigned long     _conversion_period;
unsigned long     _last_started_temp_req;
bool              _started_temp_req;
bool              _blink_state;
OneWire           _one_wire_bus(PIN_ONE_WIRE);
DallasTemperature _sensor_interface(&_one_wire_bus);
DeviceAddress     _sensor_address;
ESP8266WebServer  _server;
String            _log_history[LOG_HISTORY_LENGTH];
int               _log_history_idx = 0;
int               _valid_log_history = 0;
uint8_t           _num_found_networks = 0;
Network           _found_networks[MAX_NETWORKS];

void yield_delay() {
  delay(YIELD_DELAY_MS);
}

bool internet_settings_exist(Storage const& storage) {
  return storage.ssid[0] != 0;
}

// Read/write a single uint8_t value
void serialize_uint8_t(uint8_t& val, int& address, bool write) {
  //  log("serialize_uint8_t");
  //  log(address);
  assert(sizeof(val) == sizeof(uint8_t));
  if (write) {
    //    log(val);
    EEPROM.write(val, address);
  } else {
    val = EEPROM.read(address);
    //    log(val);
  }
  address += sizeof(val);
}

// Read/write a single char value
void serialize_char(char& val, int& address, bool write) {
//  log("serialize_char");
//  log(address);
  assert(sizeof(val) == sizeof(uint8_t));
  if (write) {
//    log(val);
    EEPROM.write(address, val);
  } else {
    val = EEPROM.read(address);
//    log(val);
  }
  address += sizeof(val);
}

// Read/write a single float value
void serialize_float(float& val, int& address, bool write) {
  assert(sizeof(val) == 4 * sizeof(uint8_t));
  if (write) {
    for (auto i = 0; i < 4; ++i) {
      EEPROM.write(address, static_cast<uint8_t>(static_cast<uint32_t>(val) >> (8 * sizeof(uint8_t) * i)));
      address += sizeof(uint8_t);
    }
  } else {
    uint32_t val_temp = 0;
    for (auto i = 0; i < 4; ++i) {
      val_temp |= EEPROM.read(address) << (8 * sizeof(uint8_t) * i);
      address += sizeof(uint8_t);
    }
    val = static_cast<float>(val_temp);
  }
}

// Serialize the entirety of EEPROM to/from Storage
void serialize_storage(Storage& storage, bool write) {
  log("serialize_storage");
  log(write);
  
  EEPROM.begin(EEPROM_SIZE);
  
  int address = EEPROM_OFFSET;

  for (auto i = 0; i < MAGIC_SIZE; ++i) {
    serialize_char(storage.magic[i], address, write);
  }

  serialize_uint8_t(storage.version, address, write);

  for (auto i = 0; i < SSID_MAX; ++i) {
    serialize_char(storage.ssid[i], address, write);
  }

  for (auto i = 0; i < PASSPHRASE_MAX; ++i) {
    serialize_char(storage.passphrase[i], address, write);
  }

  serialize_float(storage.saved_temp, address, write);

  if (write) {
    log("end");
    EEPROM.end();
  } else {
    log("saved temp");
    log(storage.saved_temp);
  }
}

bool magic_match(Storage const& storage) {
  log("magic_match");

  for (auto i = 0; i < MAGIC_SIZE; ++i) {
//    log(storage.magic[i]);
//    log(MAGIC[i]);
    if (storage.magic[i] != MAGIC[i]) {
      log("no match!");
      return false;
    }
  }

  log("matched!");
  return true;
}

// Set up and save to storage the initial settings for the device
void first_time_storage(Storage& storage) {
  log("first_time_storage");
  for (auto i = 0; i < MAGIC_SIZE; ++i) {
    storage.magic[i] = MAGIC[i];
  }
  storage.version = VERSION;
  memset(storage.ssid, 0, sizeof(storage.ssid));
  memset(storage.passphrase, 0, sizeof(storage.passphrase));
  storage.saved_temp = DEFAULT_TEMP;
  serialize_storage(storage, true);
}

void deinit_ap() {
  log("deinit_ap");
  WiFi.softAPdisconnect(true);
}

void deinit_webserver() {
  log("deinit_webserver");
  _server.close();
}

void set_mode(RunMode mode) {
  _mode = mode;  
}

bool init_sta(char const* ssid, char const* passphrase) {
  log("init_sta");
  log(ssid);
  log(passphrase);

  WiFi.mode(WIFI_STA); 
  auto status = WiFi.begin(ssid, passphrase);

  while (status == WL_IDLE_STATUS) {
    yield_delay();
    status = WiFi.status();
  }

  log("got status");
  log(status);

  if (status == WL_CONNECTED) {
    log("init_sta success");
    return true;
  } else {
    log("init_sta failure");
    return false;
  }
}

void to_online() {
  log("to_online");
  set_blink(false, micros());

  deinit_ap();
  deinit_webserver();
  
  if (init_sta(_storage.ssid, _storage.passphrase)) {
    set_mode(RunMode::ONLINE);    
  } else {
    to_offline();
  }    
}

void init_ap() {
  log("init_ap");
  log(WiFi.mode(WIFI_AP_STA));

  if (WiFi.softAP(AP_SSID, AP_PASSPHRASE)) {
    log("init_ap success");
  } else {
    log("init_ap failure");
  }
}

// Concatenate all recent logs into a single string
String get_log_contents() {
  String contents;
  int idx = _log_history_idx - _valid_log_history;
  if (idx < 0) idx += LOG_HISTORY_LENGTH;
  for (int i = 0; i < _valid_log_history; ++i) {
    contents += _log_history[(idx + i) % LOG_HISTORY_LENGTH];
    contents += "\n";
  }  
  return contents;
}

String render_ssid(Network const& network) {
  String ssid;
  ssid += "<div><input type=\"radio\" name=\"ssid\" id=\"";
  ssid += network.ssid;
  ssid += "\" value=\"";
  ssid += network.ssid;
  ssid += "\"/><label for=\"";
  ssid += network.ssid;
  ssid += "\">";
  ssid += network.ssid;
  ssid += "</label></div>";
  return ssid;
}

String render_ssids(Network const* networks, uint8_t count) {
  String ssids;
  for (auto i = 0; i < count; ++i) {
    ssids += render_ssid(networks[i]);
  }
  return ssids;
}

String render_root() {
  String html;

  html += "<html><head><title>Kobold</title></head><body><form method=\"post\" action=\"/settings\">";
  html += "<fieldset><legend>SSID</legend>";
  html += render_ssids(_found_networks, _num_found_networks);
  html += "</fieldset><fieldset><legend>Password</legend><input type=\"password\" name=\"password\"/></fieldset>";
  html += "<input type=\"submit\"/></form></body></html>";

  return html;
}

void init_webserver(ESP8266WebServer& server) {
  log("init_webserver");
  server.begin(SERVER_PORT);

  server.on("/", [&server]() {
    log("/");
    server.send(200, "text/html", render_root());    
  });

  server.on("/settings", HTTP_POST, [&server]() {
    log("/settings");

    int args = server.args();
    for (int i = 0; i < args; ++i) {
      log(server.argName(i));
      log(server.arg(i));
    }

    server.send(200);
  });
  
  server.on("/logs", [&server]() {
    log("/logs");
    server.send(200, "text/plain", get_log_contents());
  });

  server.on("/reset", [&server]() {
    log("/reset");
    first_time_storage(_storage);
    server.send(200);
  });
}

void to_offline() {
  log("to_offline");
  
  set_blink(true, micros());

  init_ap();
  init_webserver(_server);

  set_mode(RunMode::OFFLINE);
}

void process_online() {

}

// Return true when we've waited long enough since a previous occurrence of an event
bool period_elapsed(unsigned long last_occurrence, unsigned long now, unsigned long period) {
  return abs(now - last_occurrence) >= period;
}

// Enable or disable the relay
void set_relay(bool enabled) {
//  log("set_relay");
//  log(enabled);
  log(enabled ? "relay ON" : "relay OFF");
  digitalWrite(PIN_RELAY, enabled ? HIGH : LOW);
}

// Turn on or off the LED
void set_blink(bool on, unsigned long now) {
  _blink_state = on ? true : false;
  digitalWrite(PIN_LED, on ? HIGH : LOW);
  _last_changed_blink = now;
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
        log("conversion complete");
        temp = _sensor_interface.getTempC(_sensor_address);
        return true;
      } else {
        log("conversion incomplete");
      }
    }
  } 
  // We haven't yet started conversion
  else {
    log("requestTemperatures");
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
  log(line);
}

// Callback for networks being found
void on_scan_complete(int found) {
  log("on_scan_complete");

  _num_found_networks = found;
  log(_num_found_networks);

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
  log(decision);  
  
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
      log("scanning wifi");
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
      control_decision(temp, _storage.saved_temp);
    }
  }

  // Time to blink again
  if (period_elapsed(_last_changed_blink, now, _blink_state ? PERIOD_BLINK_OFF : PERIOD_BLINK_ON)) {
    set_blink(!_blink_state, now);
  }

  // To retry online again
  if (period_elapsed(_latest_user_action, now, PERIOD_RETRY_ONLINE)) {
    log("retry online");
    if (internet_settings_exist(_storage)) {
      log("internet settings exist");
      to_online();    
    } else {
      log("no internet settings");
    }
    _latest_user_action = now;
  }
}

void init_serial() {
  Serial.begin(SERIAL_SPEED, SERIAL_CONFIG);
  while (!Serial) { yield_delay(); }
}

void init_pins() {
  log("init_pins");
  pinMode(PIN_LED,      OUTPUT);
  pinMode(PIN_RELAY,    OUTPUT);
  pinMode(PIN_ONE_WIRE, INPUT);
}

void log_device_address(DeviceAddress& device_address) {
  char address[2 * sizeof(DeviceAddress) / sizeof(*device_address) + 1];
  sprintf(address, "%02X%02X%02X%02X%02X%02X%02X%02X",
    device_address[0], device_address[1], device_address[2], device_address[3], 
    device_address[4], device_address[5], device_address[6], device_address[7]);
  log(address);
}

void init_sensors() {
  _sensor_interface.begin();
  _sensor_interface.setResolution(SENSOR_ADC_BITS);
  _sensor_interface.setWaitForConversion(false);
  
  memset(_sensor_address, 0, sizeof(_sensor_address));
  for (auto i = 0; i < _sensor_interface.getDeviceCount(); ++i) {
    if (_sensor_interface.getAddress(_sensor_address, i)) {
      log("temperature address found");
      log_device_address(_sensor_address);
    } else {
      log("temperature address not found")
    }
  }

  _conversion_period = _sensor_interface.millisToWaitForConversion(SENSOR_ADC_BITS) * 1000;
  reset_conversion();
}

void setup() {
  init_serial();
  init_pins();
  init_sensors();
  serialize_storage(_storage, false);

  if (!magic_match(_storage)) {
    first_time_storage(_storage);
  }

  if (internet_settings_exist(_storage)) {
    to_online();
  } else {
    to_offline();
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

