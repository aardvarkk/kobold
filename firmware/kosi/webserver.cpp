#include "constants.hpp"
#include "globals.hpp"
#include "webserver.hpp"

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