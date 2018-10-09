#pragma once

#include "constants.hpp"

enum class RunMode {
  OFFLINE,
  ONLINE
};

struct Storage {
  char    magic[MAGIC_SIZE];
  
  uint8_t version;

  String  ssid_internal;
  String  password_internal;
  
  String  ssid_external;
  String  password_external;
  
  String  report_url;

  String  link_email;
  String  link_password;
  
  float   setpoint;
};

struct Network {
  String   ssid;
  uint8_t  encryption_type;
  int32_t  rssi;
  uint8_t* bssid;
  int32_t  channel;
  bool     is_hidden;
};
