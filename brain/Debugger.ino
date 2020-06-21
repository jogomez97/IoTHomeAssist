/**
 * Debuggler class allows to send log message for debugging through serial monitor,
 * TCP connection on port 23 or both. Can be disabled with a master enable.
 * MET06 - Grupo 2
 * @authors - Joan Gomez, Jordi Malé, Toni Chico
 */


#include <ESP8266WiFi.h>

#define DEBUGGER_CL   1 // Debug via command line
#define DEBUGGER_TCP  2 // Debug via tcp connection
#define DEBUGGER_ALL  3 // Debug via command line AND tcp connection

bool debugger_enabled = false;
int debugger_mode = DEBUGGER_CL;

WiFiServer wifiServer(23);
WiFiClient wifiClient;

/**
 * Init of the debugger
 * @param enabled, master enable of all the TAD. If false no messages will be sent despite the interface.
 * @param interface, interface from which we will communicate. Must be one of DEBUGGER_* values. 
 *        With DEBUGGER_ALL the TAD will send logs to both channels
 */
void initDebugger(bool enabled, int interface) {
  if (interface != DEBUGGER_TCP && interface != DEBUGGER_CL && interface != DEBUGGER_ALL) {
    debugger_enabled = false;
  } else {
    debugger_enabled = enabled;
    debugger_mode = interface;
  }

  if (interface == DEBUGGER_CL || interface == DEBUGGER_ALL) Serial.begin(9600);
  if (interface == DEBUGGER_TCP || interface == DEBUGGER_ALL) {
    wifiServer.begin();
    wifiServer.setNoDelay(true);
  }
}

/**
 * Send a log to the specified interface(s) if enabled.
 * @param l, String log to be sent
 */
void sendLog(String l) {
  if (debugger_enabled) {
    if (debugger_mode == DEBUGGER_CL || debugger_mode == DEBUGGER_ALL) {
      Serial.println(l);
    }
    if (debugger_mode == DEBUGGER_TCP || debugger_mode == DEBUGGER_ALL) {
      setUpClientConnection();                                          // Check if there is a client to be connected
      if (wifiClient && wifiClient.connected()) wifiClient.println(l);  // Send data to client if connected
    }
  }
}

/**
 * Check if a client is available or connected to be able to send the logs
 */
void setUpClientConnection() {
  if (wifiServer.hasClient()) {
    if (!wifiClient || !wifiClient.connected()) {
      if (wifiClient) {
        wifiClient.stop();
      }
      wifiClient = wifiServer.available();
      wifiClient.flush();
    }
  }
}
