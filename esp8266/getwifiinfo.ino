#include <ESP8266WiFi.h>
#include <ESPping.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "test";
const char* password = "12345678";
const char* externalServer = "http://123.30.105.197:1514";

unsigned int totalPings = 0;
unsigned int successfulPings = 0;

WiFiClient wifiClient;

String getEncryptionType(uint8_t encryptionType) {
  switch (encryptionType) {
    case ENC_TYPE_WEP:
      return "WEP";
    case ENC_TYPE_TKIP:
      return "WPA";
    case ENC_TYPE_CCMP:
      return "WPA2";
    case ENC_TYPE_NONE:
      return "Open";
    default:
      return "Unknown";
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  delay(1000);
}

void loop() {
  // Get the gateway IP address
  IPAddress gatewayIP = WiFi.gatewayIP();

  // Perform ping to the gateway
  int pingTime = Ping.ping(gatewayIP);

  if (pingTime >= 0) {
    Serial.print("Ping to gateway successful. Time: ");
    Serial.print(pingTime);
    Serial.println(" ms");

    successfulPings++;
  } else {
    Serial.println("Ping to gateway failed");
  }

  totalPings++;

  // Print the total number of pings
  Serial.print("Total Pings: ");
  Serial.println(totalPings);

  // Calculate and print PDR
  float pdr = (float)successfulPings / totalPings * 100;
  Serial.print("Packet Delivery Ratio (PDR): ");
  Serial.print(pdr);
  Serial.println("%");

  // Retrieve WiFi information
  String ssid = WiFi.SSID();
  int32_t rssi = WiFi.RSSI();
  int channel = WiFi.channel();
  String bssid = WiFi.BSSIDstr();

  // Get encryption type for the connected network
  uint8_t encryptionType = WiFi.encryptionType(WiFi.scanNetworks()); // Scan for networks to get encryption type

  // Make HTTP POST request with the combined JSON payload
  HTTPClient http;
  http.begin(wifiClient, externalServer);
  http.addHeader("Content-Type", "application/json");

  // Construct the JSON payload with ping result, count, and gateway IP, and WiFi information
  String jsonPayload = "{\"pingTime\":" + String(pingTime) + "ms" +
                      ",\"pdr\":" + String(pdr) + "%" +
                      ",\"totalPings\":" + String(totalPings) +
                      ",\"gatewayIP\":\"" + gatewayIP.toString() + "\"" +
                      ",\"SSID\":\"" + ssid +
                      "\",\"Channel\":" + String(channel) +
                      ",\"SignalStrength\":" + String(rssi) +"dbm" +
                      ",\"MAC\":\"" + bssid +
                      "\",\"EncryptionType\":\"" + getEncryptionType(encryptionType) + "\"}";

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP POST successful. Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("HTTP POST failed. Response code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  delay(5000);  // Delay before the next round of pings and HTTP POST
}
