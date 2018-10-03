#pragma once

#include "types.hpp"

void init_webserver(ESP8266WebServer& server);
void deinit_webserver();
String render_ssid(Network const& network);
String render_ssids(Network const* networks, uint8_t count);
String render_root();