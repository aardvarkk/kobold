#include "log.hpp"
#include "wifi.hpp"

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

void log_device_address(DeviceAddress& device_address) {
  char address[2 * sizeof(DeviceAddress) / sizeof(*device_address) + 1];
  sprintf(address, "%02X%02X%02X%02X%02X%02X%02X%02X",
    device_address[0], device_address[1], device_address[2], device_address[3],
    device_address[4], device_address[5], device_address[6], device_address[7]);
  _l(address);
}

void log_wifi(Network const& network) {
  String line =
    network.ssid + " " +
    "(" + encryption_type_string(network.encryption_type) + ") " +
    "RSSI: " + network.rssi + " " +
    "CHANNEL: " + network.channel + " " +
    (network.is_hidden ? "HIDDEN!" : "");
  _l(line);
}
