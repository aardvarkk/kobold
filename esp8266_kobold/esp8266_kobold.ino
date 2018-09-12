#include <assert.h>
#include <EEPROM.h>

// CONSTANTS
const uint8_t PIN_RELAY = 12;
const long    SERIAL_SPEED = 115200;
const SerialConfig SERIAL_CONFIG = SERIAL_8N1;
const size_t  MAGIC_SIZE = 4;
const char    MAGIC[MAGIC_SIZE] = { 'K', 'B', 'L', 'D' };
const uint8_t VERSION = 1;
const size_t  SSID_MAX = 32;
const size_t  PASSPHRASE_MAX = 64;
const size_t  EEPROM_SIZE = 512;
const float   DEFAULT_TEMP = 18.0;

const unsigned long PERIOD_CHECK_TEMP = 5e6; // 5e6 = 5 seconds
//const unsigned long PERIOD_CHECK_TEMP = 3e8; // 3e8 = 5 minutes

// TYPES
enum class RunMode {
  OFFLINE,
  ONLINE
};

struct Storage {
  char magic[MAGIC_SIZE];
  uint8_t version;
  char ssid[SSID_MAX];
  char passphrase[PASSPHRASE_MAX];
  float saved_temp;
};

// GLOBAL VARIABLES
RunMode _mode;
Storage _storage;
unsigned long _last_checked_temp = -PERIOD_CHECK_TEMP;

#define log(x) {     \
  Serial.println(x); \
}

bool internet_settings_exist(Storage const& storage) {
  return storage.ssid[0] != 0;
}

// Initialize storage
void init_storage() {
  log("init_storage");
  EEPROM.begin(EEPROM_SIZE);
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
      EEPROM.write(address, static_cast<uint8_t>(static_cast<uint32_t>(val) >> (sizeof(uint8_t) * i)));
      address += sizeof(uint8_t);
    }
  } else {
    uint32_t val_temp;
    for (auto i = 0; i < 4; ++i) {
      val_temp |= EEPROM.read(address) << (sizeof(uint8_t) * i);
      address += sizeof(uint8_t);
    }
    val = static_cast<float>(val_temp);
  }
}

// Serialize the entirety of EEPROM to/from Storage
void serialize_storage(Storage& storage, bool write) {
  log("serialize_storage");
  int address = 0;

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
}

bool magic_match(Storage const& storage) {
  log("magic_match");

  for (auto i = 0; i < MAGIC_SIZE; ++i) {
    log(storage.magic[i]);
    log(MAGIC[i]);
    if (storage.magic[i] != MAGIC[i]) {
      return false;
    }
  }
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

float read_temperature() {
  log("read_temperature");
  return random(15,25);
}

void to_online() {
  log("to_online");

  // TODO: Try to connect to the Internet...
  _mode = RunMode::ONLINE;
}

void to_offline() {
  log("to_offline");

  _mode = RunMode::OFFLINE;
}

void process_online() {

}

// Return true when we've waited long enough since a previous occurrence of an event
bool period_elapsed(unsigned long last_occurrence, unsigned long now, unsigned long period) {
  return abs(now - last_occurrence) >= period;
}

// Enable or disable the relay
void set_relay(bool enabled) {
  log("set_relay");
  log(enabled);
  digitalWrite(PIN_RELAY, enabled ? HIGH : LOW);
}

void process_offline() {
  auto now = micros();

  // TODO: Set up as AP and run server

  // Time to read from the sensor again
  if (period_elapsed(_last_checked_temp, now, PERIOD_CHECK_TEMP)) {
    log("check temperature");
    _last_checked_temp = now;
    auto temp = read_temperature();
    log(temp);
    set_relay(temp < _storage.saved_temp);
  }

  // TODO: Timeout has elapsed, so try going back to online
//  if (timeout_expired() && internet_settings_exist()) {
//     to_online();
//  }
}

void init_serial() {
  Serial.begin(SERIAL_SPEED, SERIAL_CONFIG);
  while (!Serial) {}
}

void init_pins() {
  log("init_pins");
  pinMode(PIN_RELAY, OUTPUT);
}

void setup() {
  init_serial();
  init_pins();
  init_storage();
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

