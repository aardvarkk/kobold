#include "log.hpp"

// Concatenate all recent logs into a single string
String get_log_contents() {
  String contents;
  int idx = _log_history_idx - _valid_log_history;
  if (idx < 0) idx += LOG_HISTORY_LENGTH;
  for (int i = 0; i < _valid_log_history; ++i) {
    contents += _log_history[(idx + i) % LOG_HISTORY_LENGTH];
    contents += "\n";
  }
  return contents;
}
