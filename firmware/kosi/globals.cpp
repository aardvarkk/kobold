#include "constants.hpp"
#include "globals.hpp"

RunMode           _mode;
Storage           _storage;
unsigned long     _last_wifi_scan;
unsigned long     _last_checked_temp;
unsigned long     _last_changed_blink;
unsigned long     _latest_user_action;
unsigned long     _last_started_temp_req;
unsigned long     _conversion_period;
bool              _started_temp_req;
bool              _blink_state;
OneWire           _one_wire_bus = OneWire(PIN_ONE_WIRE);
DallasTemperature _sensor_interface = DallasTemperature(&_one_wire_bus);
DeviceAddress     _sensor_address;
ESP8266WebServer  _server;
ESP8266HTTPUpdateServer _update_server;
String            _log_history[LOG_HISTORY_LENGTH];
int               _log_history_idx = 0;
int               _valid_log_history = 0;
uint8_t           _num_found_networks = 0;
Network           _found_networks[MAX_NETWORKS];
