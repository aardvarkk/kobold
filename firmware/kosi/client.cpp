#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#include "client.hpp"
#include "constants.hpp"
#include "log.hpp"
#include "types.hpp"

bool parse_json(String const& response, JsonObject& root) {
  _l("parse_json");

  DynamicJsonDocument doc;
  auto error = deserializeJson(doc, response);

  if (error) {
    _l("parse_json error");
    _l(error.c_str());
    return false;
  }

  root = doc.as<JsonObject>();
  return true;
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
    code = client.POST(json_report_temperature(temp));
    _l(code);
    if (code < 0) {
      _l("http request failed");
      _l(HTTPClient::errorToString(code));
    } else {
      _l("http request succeeded");
      response = client.getString();
      _l(response);
    }
  } else {
    _l("failed starting http client");
  }

  digitalWrite(PIN_LED, INACTIVE);

  client.end();
  return code;
}

int link_device(String const& url, String const& email, String const& password, String& token) {
  _l("link_device");
  _l(url);
  _l(email);
  _l(password);
  
  int code = 0;
  token = "";

  digitalWrite(PIN_LED, ACTIVE);

  HTTPClient client;
  auto success = client.begin(url);
  if (success) {
    client.addHeader("Content-Type", "application/json");
    code = client.POST(json_link_device(email, password));
    _l(code);
    if (code < 0) {
      _l("http request failed");
      _l(HTTPClient::errorToString(code));
    } else {
      _l("http request succeeded");
      
      JsonObject json;
      switch (code) {
        case 200:
          if (parse_json(client.getString(), json)) {
            token = json.get<String>("token");
          }
          break;
        default:
          _l("unrecognized code");
          break;
      }
    }
  } else {
    _l("failed to start http client");
  }

  digitalWrite(PIN_LED, INACTIVE);

  client.end();
  return code;
}

String json_report_temperature(float temp) {
  String json;
  json += "{";
  json += "\"temperature\":" + String(temp);
  json += "}";
  _l(json);
  return json;  
}

String json_link_device(String const& email, String const& password) {
  String json;
  json += "{";
  json += "\"key\":\"" + KEY + "\",";
  json += "\"secret\":\"" + SECRET + "\",";
  json += "\"email\":\"" + email + "\",";
  json += "\"password\":\"" + password + "\"";
  json += "}";
  _l(json);
  return json;  
}