#pragma once

#include <Arduino.h>

int report_temperature(String const& report_url, String const& token, float temp, String& response);

int link_device(String const& url, String const& email, String const& password, String& token);

String json_report_temperature(float temp);

String json_link_device(String const& email, String const& password);