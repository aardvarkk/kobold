#pragma once

#include <Arduino.h>

#include "types.hpp"

void first_time_storage(Storage& storage);

bool serialize_storage(Storage& storage, bool write);

void serialize_uint8_t(uint8_t& val, int& address, bool write);
void serialize_string(String& val, int& address, bool write);
void serialize_char(char& val, int& address, bool write);
void serialize_float(float& val, int& address, bool write);

bool internet_settings_exist(Storage const& storage);
