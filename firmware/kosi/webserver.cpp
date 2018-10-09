#include "constants.hpp"
#include "globals.hpp"
#include "log.hpp"
#include "state.hpp"
#include "storage.hpp"
#include "webserver.hpp"

void save_settings(ESP8266WebServer& server) {
  _l("/settings");

  String ssid_internal;
  String ssid_external, password_external;
  String report_url, setpoint;

  for (int i = 0; i < server.args(); ++i) {
    auto name = server.argName(i);
    auto val  = server.arg(i);

    _l(name);
    _l(val);

    if (name == "ssid-internal")     { ssid_internal = val; }
    if (name == "ssid-external")     { ssid_external = val; }
    if (name == "password-external") { password_external = val; }
    if (name == "setpoint")          { setpoint = val; }
    if (name == "report-url")        { report_url = val; }
  }

  bool dirty = false;

  if (ssid_internal.length()) {
    ssid_internal = AP_PREPEND + ssid_internal;    
    if (ssid_internal != _storage.ssid_internal) {
      _storage.ssid_internal = ssid_internal;
      dirty = true;
      to_offline(micros());
    }
  }

  if (ssid_external.length() && password_external.length()) {
    if (ssid_external     != _storage.ssid_external || 
        password_external != _storage.password_external) {
      _storage.ssid_external     = ssid_external;
      _storage.password_external = password_external;
      dirty = true;
    }
  }

  if (report_url.length()) {
    if (report_url != _storage.report_url) {
      _storage.report_url = report_url;
      dirty = true;
    }
  }
  
  if (setpoint.length()) {
    if (String(_storage.setpoint) != setpoint) {
      _storage.setpoint = setpoint.toFloat();
      dirty = true;
    }
  }

  if (dirty) {
    serialize_storage(_storage, true);
  }

  server.send(200);
}

void link_account(ESP8266WebServer& server, String& email, String& password) {
  _l("/link");

  email = "";
  password = "";

  for (int i = 0; i < server.args(); ++i) {
    auto name = server.argName(i);
    auto val  = server.arg(i);

    _l(name);
    _l(val);

    if (name == "email")    { email = val; }
    if (name == "password") { password = val; }
  }

  server.send(200);
  to_online(micros());
}

void init_webserver(ESP8266WebServer& server) {
  _l("init_webserver");

  _l("init OTA");
  _update_server.setup(&_server);

  server.begin(SERVER_PORT);

  server.on("/", [&server]() {
    _latest_user_action = micros();
    _l("/");
    server.send(200, "text/html", render_root());
  });

  server.on("/settings", HTTP_POST, [&server]() {
    _latest_user_action = micros();
    save_settings(server);
  });

  server.on("/link", HTTP_POST, [&server]() {
    _latest_user_action = micros();
    link_account(server, _storage.link_email, _storage.link_password);
  });

  server.on("/logs", [&server]() {
    _latest_user_action = micros();
    _l("/logs");
    server.send(200, "text/plain", get_log_contents());
  });

  server.on("/reset", [&server]() {
    _latest_user_action = micros();
    _l("/reset");
    first_time_storage(_storage);
    ESP.restart();
    server.send(200);
  });
}

void deinit_webserver() {
  _l("deinit_webserver");
  _server.close();
}

String render_ssid(
  Network const& network,
  String const& selected
) 
{
  String ssid;
  ssid += "<div><input type=\"radio\" name=\"ssid-external\" id=\"";
  ssid += network.ssid;
  ssid += "\" value=\"";
  ssid += network.ssid;
  ssid += "\"";
  if (network.ssid == selected) ssid += " checked";
  ssid += "/><label for=\"";
  ssid += network.ssid;
  ssid += "\">";
  ssid += network.ssid;
  ssid += "</label></div>";
  return ssid;
}

String render_ssids(Network const* networks, uint8_t count, String const& selected) {
  String ssids;
  for (auto i = 0; i < count; ++i) {
    ssids += render_ssid(networks[i], selected);
  }
  return ssids;
}

String render_root() {
  String html;

  html += "<html><head><title>kosi</title></head><body>";
  
  // Settings Form
  html += "<h1>Settings</h1>";

  html += "<form method=\"post\" action=\"/settings\">";

  html += "<fieldset><legend>Internal SSID</legend><input type=\"text\" name=\"ssid-internal\" value=\"";
  html += String(_storage.ssid_internal).substring(AP_PREPEND.length()) + "\"/></fieldset>";

  html += "<fieldset><legend>External SSID</legend>";
  html += render_ssids(_found_networks, _num_found_networks, _storage.ssid_external);
  html += "</fieldset>";

  html += "<fieldset><legend>External Password</legend><input type=\"password\" name=\"password-external\" value=\"";
  html += String(_storage.password_external) + "\"/></fieldset>";

  html += "<fieldset><legend>Report URL</legend><input type=\"url\" name=\"report-url\" value=\"";
  html += String(_storage.report_url) + "\"/></fieldset>";

  html += "<fieldset><legend>Set Point</legend><input type=\"number\" value=\"";
  html += String(_storage.setpoint) + "\" step=\"0.1\" min=\"0.0\" max=\"25.0\" name=\"setpoint\"/></fieldset>";

  html += "<input type=\"submit\"/></form>";

  // Link Form
  html += "<h1>Link Account</h1>";

  html += "<form method=\"post\" action=\"/link\">";

  html += "<fieldset><legend>Email</legend><input type=\"email\" name=\"email\"/></fieldset>";
  html += "<fieldset><legend>Password</legend><input type=\"password\" name=\"password\"/></fieldset>";

  html += "<input type=\"submit\"/></form>";

  html += "</body></html>";

  return html;
}