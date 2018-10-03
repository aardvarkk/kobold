#pragma once

#define REQUIRESALARMS false

#include <DallasTemperature.h>
// #include <ESP8266mDNS.h>
// #include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "types.hpp"

extern RunMode           _mode;
extern Storage           _storage;
extern unsigned long     _last_wifi_scan;
extern unsigned long     _last_checked_temp;
extern unsigned long     _last_changed_blink;
extern unsigned long     _latest_user_action;
extern unsigned long     _last_started_temp_req;
extern unsigned long     _conversion_period;
extern bool              _started_temp_req;
extern bool              _blink_state;
extern OneWire           _one_wire_bus;
extern DallasTemperature _sensor_interface;
extern DeviceAddress     _sensor_address;
extern ESP8266WebServer  _server;
extern ESP8266HTTPUpdateServer _update_server;
extern String            _log_history[LOG_HISTORY_LENGTH];
extern int               _log_history_idx;
extern int               _valid_log_history;
extern uint8_t           _num_found_networks;
extern Network           _found_networks[MAX_NETWORKS];
