#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define ONE_WIRE_BUS 14

ADC_MODE(ADC_VCC);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  WiFi.begin("dlink-0AD8", "suvpl66760");

  sensors.begin();
  sensors.setResolution(12);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  HTTPClient http;
  http.begin("192.168.0.136", 3000);

  sensors.requestTemperatures();

  for (int i = 0; i < sensors.getDeviceCount(); ++i) {
    DeviceAddress deviceAddress;
    sensors.getAddress(deviceAddress, i);
    
    char address[2 * sizeof(DeviceAddress) / sizeof(*deviceAddress) + 1];
    sprintf(address, "%02X%02X%02X%02X%02X%02X%02X%02X",
      deviceAddress[0], deviceAddress[1], deviceAddress[2], deviceAddress[3], 
      deviceAddress[4], deviceAddress[5], deviceAddress[6], deviceAddress[7]);

    Serial.println(String(address) + " " + sensors.getTempC(deviceAddress) + " " + ESP.getVcc());

    String payload = String("") +
      "{" + 
        "\"sensor\":"      + "\"" + address + "\""           + ","
        "\"temperature\":" + sensors.getTempC(deviceAddress) + ","
        "\"voltage\":"     + ESP.getVcc() / 1000.f           +
      "}";

    Serial.println(payload);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(payload);
    Serial.println(code);
  }
}

void loop() {
}
