#pragma once

#include "types.h"

String render_ssid(Network const& network);
String render_ssids(Network const* networks, uint8_t count);
String render_root();