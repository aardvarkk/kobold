#include <EEPROM.h>

#include "log.hpp"
#include "magic.hpp"
#include "serialize.hpp"

// Read/write a single uint8_t value
void serialize_uint8_t(uint8_t& val, int& address, bool write) {
//  _l("serialize_uint8_t");
//  _l(address);
  if (write) {
    EEPROM.write(address, val);
//    _l(val);
  } else {
    val = EEPROM.read(address);
//    _l(val);
  }
  address += sizeof(val);
}

// Read/write a string
void serialize_string(String& val, int& address, bool write) {
//  _l("serialize_string");
  if (write) {
    uint8_t len = static_cast<uint8_t>(val.length());
//    _l(len);
//    _l(val);
    serialize_uint8_t(len, address, write);
    for (auto i = 0; i < val.length(); ++i) {
      serialize_char(val[i], address, write);
    }
  } else {
    uint8_t len;
    serialize_uint8_t(len, address, write);
//    _l(len);
    val = "";
    for (auto i = 0; i < len; ++i) {
      char c;
      serialize_char(c, address, write);
      val += c;
    }      
//    _l(val);
  }
}

// Read/write a single char value
void serialize_char(char& val, int& address, bool write) {
//  _l("serialize_char");
//  _l(address);
  if (write) {
    EEPROM.write(address, val);
//    _l(val);
  } else {
    val = EEPROM.read(address);
//    _l(val);
  }
  address += sizeof(val);
}

// Read/write a single float value
void serialize_float(float& val, int& address, bool write) {
  if (write) {
    EEPROM.put(address, val);
  } else {
    EEPROM.get(address, val);
  }
  address += sizeof(val);
}

// Serialize the entirety of EEPROM to/from Storage
bool serialize_storage(Storage& storage, bool write) {
  _l("serialize_storage");
  _l(write);

  _l("begin");
  EEPROM.begin(EEPROM_SIZE);

  int address = 0;

  for (auto i = 0; i < MAGIC_SIZE; ++i) {
    serialize_char(storage.magic[i], address, write);
  }

  if (!write && !magic_match(storage)) {
    _l("end");
    EEPROM.end();
    return false;
  }
  
  serialize_uint8_t(storage.version, address, write);

  serialize_string(storage.ssid_internal, address, write);
  serialize_string(storage.password_internal, address, write);
  serialize_string(storage.ssid_external, address, write);
  serialize_string(storage.password_external, address, write);
  serialize_string(storage.report_url, address, write);
  serialize_string(storage.token, address, write);

  serialize_float(storage.setpoint, address, write);

  _l("end");
  EEPROM.end();

  return true;
}
