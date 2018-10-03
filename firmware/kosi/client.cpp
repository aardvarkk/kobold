#include <ESP8266HTTPClient.h>

#include "client.hpp"
#include "constants.hpp"
#include "log.hpp"
#include "types.hpp"

String report_json(float temp) {
  String json;
  json += "{";
  json += "\"temp\":";
  json += String(temp);
  json += "}";
  _l(json);
  return json;  
}

int report_temperature(String const& report_url, String const& token, float temp, String& response) {
  _l("report_temperature");
  _l(report_url);
  _l(temp);

  int code = -1;

  digitalWrite(PIN_LED, ACTIVE);

  HTTPClient client;
  auto success = client.begin(report_url);
  if (success) {
    client.addHeader("Authorization", String("Bearer ") + token);
    client.addHeader("Content-Type", "application/json");
    code = client.POST(report_json(temp));
    _l(code);
    if (code < 0) {
      _l("http request failed");
      _l(HTTPClient::errorToString(code));
    } else {
      _l("http request succeeded");
      response = client.getString();
    }
  } else {
    _l("failed starting http client");
  }

  digitalWrite(PIN_LED, INACTIVE);

  client.end();
  return code;
}

int refresh_token(String const& url, String const& username, String const& password, String& token) {
  _l("refresh_token");
  _l(url);
//  _l(username);
//  _l(password);
  
  int code = -1;
  token = "";

  digitalWrite(PIN_LED, ACTIVE);

  HTTPClient client;
  auto success = client.begin(url);
  if (success) {
    client.setAuthorization(username.c_str(), password.c_str());
    code = client.POST("");
    _l(code);
    if (code < 0) {
      _l("http request failed");
      _l(HTTPClient::errorToString(code));
    } else {
      _l("http request succeeded");
      token = client.getString();
      _l(token);
    }
  } else {
    _l("failed to start HTTP client");
  }

  digitalWrite(PIN_LED, INACTIVE);

  client.end();
  return code;
}
