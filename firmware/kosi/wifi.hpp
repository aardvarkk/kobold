#pragma once

#include <Arduino.h>

void deinit_ap();
String wifi_status(int code);
bool init_sta(String const& ssid, String const& password);
void init_ap(String const& ssid, String const& password);