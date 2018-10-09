#pragma once

#include <Arduino.h>

bool init_sta(String const& ssid, String const& password);
void deinit_sta();

void init_ap(String const& ssid, String const& password);
void deinit_ap();

void on_scan_complete(int found);

String wifi_status(int code);
String encryption_type_string(uint8_t encryption_type);