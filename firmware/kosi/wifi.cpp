#include "log.hpp"
#include "types.hpp"
#include "wifi.hpp"
#include "timers.hpp"

bool init_sta(String const& ssid, String const& password) {
  _l("init_sta");
  _l(ssid);
  _l(password);

  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);

  if (WiFi.mode(WIFI_STA)) {
    _l("sta mode success");
  } else {
    _l("sta mode failure");
  }

  auto status = WiFi.begin(ssid.c_str(), password.c_str());
  _l("begin status");
  _l(status);

  while (status == WL_DISCONNECTED) {
    yield_delay();
    status = WiFi.status();
  }

  _l("got status");
  _l(status);
  _l(wifi_status(status));

  if (status == WL_CONNECTED) {
    _l("init_sta success");
    return true;
  } else {
    _l("init_sta failure");
    return false;
  }
}

void deinit_sta() {
  _l("deinit_sta");
  
  if (WiFi.disconnect(true)) {
    _l("disconnect success");
  } else {
    _l("disconnect failure");
  }
}

void init_ap(String const& ssid, String const& password) {
  _l("init_ap");
  
  WiFi.persistent(false);

  if (WiFi.mode(WIFI_AP_STA)) {
    _l("ap mode success");
  } else {
    _l("ap mode failure");
  }

  if (WiFi.softAP(ssid.c_str(), password.c_str())) {
    _l("init_ap success");

    _l("ip address");
    _l(WiFi.softAPIP().toString());
    _l("mac address");
    _l(WiFi.softAPmacAddress());
    _l("ssid");
	  _l(WiFi.softAPSSID());
	  _l("psk");
    _l(WiFi.softAPPSK());
  } else {
    _l("init_ap failure");
  }
}

void deinit_ap() {
  _l("deinit_ap");
  
  if (WiFi.softAPdisconnect(true)) {
    _l("softAPdisconnect success");
  } else {
    _l("softAPdisconnect failure");
  }
}

// Callback for networks being found
void on_scan_complete(int found) {
  _l("on_scan_complete");

  _num_found_networks = found;
  _l(_num_found_networks);

  for (int i = 0; i < _num_found_networks; ++i) {
    Network& network = _found_networks[i];
    WiFi.getNetworkInfo(
      i,
      network.ssid,
      network.encryption_type,
      network.rssi,
      network.bssid,
      network.channel,
      network.is_hidden
    );
    log_wifi(network);
  }

  WiFi.scanDelete();
}

String wifi_status(int code) {
  switch (code) {
    case WL_NO_SHIELD:       return "WL_NO_SHIELD";
    case WL_IDLE_STATUS:     return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:   return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:  return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:       return "WL_CONNECTED";
    case WL_CONNECT_FAILED:  return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:    return "WL_DISCONNECTED";
    default:                 return "UNKNOWN";
  }
}

String encryption_type_string(uint8_t encryption_type) {
  switch (encryption_type) {
    case ENC_TYPE_WEP:  return "WEP";
    case ENC_TYPE_TKIP: return "TKIP";
    case ENC_TYPE_CCMP: return "CCMP";
    case ENC_TYPE_NONE: return "NONE";
    case ENC_TYPE_AUTO: return "AUTO";
    default: return "UNKNOWN";
  }
}
