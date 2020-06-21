/**
 * Allows the user to stablish a connection via WiFi with both a dynamic or static IP.
 * MET06 - Grupo 2
 * @authors - Joan Gomez, Jordi Malé, Toni Chico
 */

#include <ESP8266WiFi.h>

#define WAIT_TIME_MS 10000

/**
 * Init the Wifi module with dynamic IP
 * @param ssid_name, ssid of the network to connect
 * @param password, password of the network to connect
 */
int initWifi(String ssid_name, String password) {
  WiFi.begin(ssid_name.c_str(), password.c_str());
  return waitForConnection();
}

/**
 * Init the Wifi module with static IP
 * @param ssid_name, ssid of the network to connect
 * @param password, password of the network to connect
 * @param ip, static IP to be assigned
 * @param gateway, router gateway IP of the network
 * @param subnet, subnet mask of the network
 */
int initWifiStatic(String ssid_name, String password, const int ip[], const int gateway[], const int subnet[]) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_name.c_str(), password.c_str());
  // Sets up WiFi with static address. If it fails it does the same as initWifi()
  WiFi.config(IPAddress(ip[0], ip[1], ip[2], ip[3]),
        IPAddress(gateway[0], gateway[1], gateway[2], gateway[3]),
        IPAddress(subnet[0], subnet[1], subnet[2], subnet[3]));

  return waitForConnection();
}

/**
 * Waits until the WiFi is configured correctly or a time threshold is passed
 * @return int, 0 if the connection was successfull, -1 otherwise
 */
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

/**
 * Gets the configured IP in the network
 * @return String, IP cofigured
 */
String getIP() {
  return WiFi.localIP().toString();
}
