#pragma once

#include <Arduino.h>

// BURN IN
const String KEY    = "D82V8IDgiJUgPwj9ZbbXcS3r002kgiUX"; // BURN IN (use first AP_KEY_CHARS characters for internal SSID)
const String SECRET = "NEqaSnzcX-rWRFGhZXoFEro8e-EwGK8J"; // BURN IN (use first AP_SECRET_CHARS characters for internal password)

const bool     FORCE_OFFLINE = false;
const String   UPDATE_HOST = "kosi"; // http://kosi.local/update
const String   AP_PREPEND  = "kosi-";
const String   REPORT_URL_DEFAULT = "http://kosi.ca/";
const int      AP_KEY_CHARS    = 6;
const int      AP_SECRET_CHARS = 8;
const uint8_t  PIN_RELAY = 12;
const uint8_t  PIN_LED = 13;
const uint8_t  PIN_ONE_WIRE = 14;
const long     SERIAL_SPEED = 115200;
const SerialConfig SERIAL_CONFIG = SERIAL_8N1;
const int      MAGIC_SIZE = 4;
const char     MAGIC[MAGIC_SIZE] = { 'K', 'B', 'L', 'D' };
const uint8_t  VERSION = 1;
const int      EEPROM_SIZE = 512;
const float    DEFAULT_SETPOINT = 18.0;
const uint8_t  SENSOR_ADC_BITS = 12;
const int      YIELD_DELAY_MS = 20;
const uint16_t SERVER_PORT = 80;
const int      LOG_HISTORY_LENGTH = 0x20;
const uint8_t  MAX_NETWORKS = 0x40;
const bool     SCAN_FOR_HIDDEN = false;
const uint8_t  ACTIVE = LOW; // Active LOW (both LED and relay)
const uint8_t  INACTIVE = HIGH;
const int      TESTING_WAIT = 10000;
const int      JSON_PARSE_BYTES = 0xFF;

const unsigned long PERIOD_WIFI_SCAN = 3e7; // 3e7 = 30 seconds
const unsigned long PERIOD_BLINK_OFF = 950000;
const unsigned long PERIOD_BLINK_ON  =  50000;
// const unsigned long PERIOD_CHECK_TEMP = 1e7; // 1e7 = 10 seconds
const unsigned long PERIOD_CHECK_TEMP = 2e7; // 2e7 = 20 seconds
//const unsigned long PERIOD_CHECK_TEMP = 3e8; // 3e8 = 5 minutes
const unsigned long PERIOD_RETRY_ONLINE = 3e7; // 3e7 = 30 seconds
// const unsigned long PERIOD_RETRY_ONLINE = 6e7; // 6e7 = 1 minute
