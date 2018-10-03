#pragma once

#include "types.hpp"

String render_ssid(Network const& network);
String render_ssids(Network const* networks, uint8_t count);
String render_root();