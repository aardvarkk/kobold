#include "constants.hpp"
#include "globals.hpp"
#include "log.hpp"
#include "state.hpp"
#include "storage.hpp"
#include "webserver.hpp"

void init_webserver(ESP8266WebServer& server) {
  _l("init_webserver");

  #ifdef OTA_WEB_PUSH
  _l("ota web push");
  MDNS.begin(UPDATE_HOST.c_str());
  _update_server.setup(&_server);
  MDNS.addService("http", "tcp", 80);
  #endif

  server.begin(SERVER_PORT);

  server.on("/", [&server]() {
    _l("/");
    server.send(200, "text/html", render_root());
  });

  server.on("/settings", HTTP_POST, [&server]() {
    _l("/settings");

    String ssid_internal;//, password_internal;
    String ssid_external, password_external;
    String report_url, setpoint;

    for (int i = 0; i < server.args(); ++i) {
      auto name = server.argName(i);
      auto val  = server.arg(i);

      _l(name);
      _l(val);

      if (name == "ssid-internal") {
        ssid_internal = val;
      }

//      if (name == "password-internal") {
//        password_internal = val;
//      }

      if (name == "ssid-external") {
        ssid_external = val;
      }

      if (name == "password-external") {
        password_external = val;
      }

      if (name == "setpoint") {
        setpoint = val;
      }

      if (name == "report-url") {
        report_url = val;
      }
    }

    if (ssid_internal.length()/* && password_internal.length()*/) {
      ssid_internal = AP_PREPEND + ssid_internal;
      // Only save these values if they're different (so we don't reboot for nothing!)
      if (ssid_internal     != _storage.ssid_internal/* || password_internal != _storage.password_internal*/) {
        _storage.ssid_internal = ssid_internal;
//        _storage.password_internal = password_internal;
        serialize_storage(_storage, true);
        to_offline(micros());
      }
    }

    if (ssid_external.length() && password_external.length()) {
      _storage.ssid_external = ssid_external;
      _storage.password_external = password_external;
      serialize_storage(_storage, true);
    }

    if (report_url.length()) {
      _storage.report_url = report_url;
      serialize_storage(_storage, true);
    }
    
    if (setpoint.length()) {
      _storage.setpoint = setpoint.toFloat();
      serialize_storage(_storage, true);
    }

    server.send(200);
  });

  server.on("/logs", [&server]() {
    _l("/logs");
    server.send(200, "text/plain", get_log_contents());
  });

  server.on("/reset", [&server]() {
    _l("/reset");
    first_time_storage(_storage);
    server.send(200);
  });
}

void deinit_webserver() {
  _l("deinit_webserver");
  _server.close();
}

String render_ssid(Network const& network) {
  String ssid;
  ssid += "<div><input type=\"radio\" name=\"ssid-external\" id=\"";
  ssid += network.ssid;
  ssid += "\" value=\"";
  ssid += network.ssid;
  ssid += "\"/><label for=\"";
  ssid += network.ssid;
  ssid += "\">";
  ssid += network.ssid;
  ssid += "</label></div>";
  return ssid;
}

String render_ssids(Network const* networks, uint8_t count) {
  String ssids;
  for (auto i = 0; i < count; ++i) {
    ssids += render_ssid(networks[i]);
  }
  return ssids;
}

String render_root() {
  String html;

  html += "<html><head><title>kosi</title></head><body><form method=\"post\" action=\"/settings\">";
  html += "<fieldset><legend>Internal SSID</legend><input type=\"text\" name=\"ssid-internal\" value=\"";
  html += String(_storage.ssid_internal).substring(AP_PREPEND.length()) + "\"/></fieldset>";
//  html += "<fieldset><legend>Internal Password</legend><input type=\"password\" name=\"password-internal\" value=\"";
//  html += String(_storage.password_internal) + "\"/></fieldset>";
  html += "<fieldset><legend>External SSID</legend>";
  html += render_ssids(_found_networks, _num_found_networks);
  html += "</fieldset>";
  html += "<fieldset><legend>External Password</legend><input type=\"password\" name=\"password-external\"/></fieldset>";
  html += "<fieldset><legend>Report URL</legend><input type=\"url\" name=\"report-url\" value=\"";
  html += String(_storage.report_url) + "\"/></fieldset>";
  html += "<fieldset><legend>Set Point</legend><input type=\"number\" value=\"";
  html += String(_storage.setpoint) + "\" step=\"0.1\" min=\"0.0\" max=\"25.0\" name=\"setpoint\"/></fieldset>";
  html += "<input type=\"submit\"/></form>";
  html += "<div>Key: <pre>" + KEY + "</pre></div>";
  html += "<div>Secret: <pre>" + SECRET + "</pre></div>";
  html += "</body></html>";

  return html;
}