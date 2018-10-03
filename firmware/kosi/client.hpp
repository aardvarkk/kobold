#pragma once

#include <Arduino.h>

String report_json(float temp);
int report_temperature(String const& report_url, String const& token, float temp, String& response);
int refresh_token(String const& url, String const& username, String const& password, String& token);