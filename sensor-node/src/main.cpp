#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>

const char *ssid = "Sensor-A1";
const char *password = "123456789";

void setup()
{
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();

  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println(WiFi.localIP());
}

void loop()
{
}
