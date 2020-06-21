#include <ESP8266WiFi.h>

#define WAIT_TIME_MS 10000

int initWifi(String ssid_name, String password) {
  WiFi.begin(ssid_name, password);
  return waitForConnection();
}

int initWifiStatic(String ssid_name, String password, const int ip[], const int gateway[], const int subnet[]) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_name, password);
  // Sets up WiFi with static address. If it fails it does the same as initWifi()
  WiFi.config(IPAddress(ip[0], ip[1], ip[2], ip[3]),
        IPAddress(gateway[0], gateway[1], gateway[2], gateway[3]),
        IPAddress(subnet[0], subnet[1], subnet[2], subnet[3]));

  return waitForConnection();
}

int waitForConnection() {
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WAIT_TIME_MS) {
    delay(500);
  }

  // Check connection
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(WiFi.localIP());
    return 0;
  }
  return -1;
}

String getIP() {
  return WiFi.localIP().toString();
}
