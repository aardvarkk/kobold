#pragma once

#include "globals.hpp"

#define _l(x) {     \
  Serial.println(x); \
  _log_history[_log_history_idx] = String(x); \
  _log_history_idx = (_log_history_idx + 1) % LOG_HISTORY_LENGTH; \
  _valid_log_history = min(_valid_log_history + 1, LOG_HISTORY_LENGTH); \
}

String get_log_contents();