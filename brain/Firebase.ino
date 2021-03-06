/**
 * Class that connects to Firebase and allows to read and write values from a specific
 * user. It's also able to send push notifications through cloud messaging
 * MET06 - Grupo 2
 * @authors - Joan Gomez, Jordi Malé, Toni Chico
 */

#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SERVER_KEY "key=AAAA_m05wUk:APA91bFgXESo0z90uIMlO0QrA5eHvw1jfd1EpBHGPsnsphXQzo8QYe6hUUNDSxoNdRHM4zTb8pZc03EYSZHq6VsOUy_aFMzFyciJsLzDHUcDDuq_n6XZioChoJDZSBVGUxvWUY94dmnP"

bool firebase_enabled = false;
String firebase_host;
String firebase_auth;

HTTPClient http;

/**
 * Inits Firebase module to be used later
 * @param host, firebase host key
 * @param auth, firebase authentication key
 * @param enable, enables or disables firebase communication. Usefull when no WiFi connection could be made
 */
void initFirebase(String host, String auth, bool enable) {
  firebase_enabled = enable;
  if (firebase_enabled) {
    firebase_host = host;
    firebase_auth = auth;
    Firebase.begin(host, auth);
  }
}

/**
 * Generic method for sending notifications to a specific username recognized by its serial_number
 * @param serial_num, serial number of the user to send the notification
 * @param title, title of the notification
 * @param body, body of the notification
 */
void sendNotification(String serial_num, String title, String body) {
  if (firebase_enabled) {
    // Get notification id
    String url = "/users/" + serial_num + "/id";
    String id = Firebase.getString(url);
    if (Firebase.failed()) {
      // Adding } else { error_ = FirebaseError(); } in FirebaseArduino.cpp -> FirebaseCall::analyzeError, reconnects to Firebase
      sendLog("Getting id failed, trying for a second time after reconnecting");
      id = Firebase.getString(url);
    }
    if (id.length() > 0) {
      // Send notification
      String data = "{ \"to\": \"" + id + "\",\"notification\": {\"title\": \"" + title + "\",\"body\":\"" + body + "\"}}";

      http.begin("https://fcm.googleapis.com/fcm/send", "95:59:16:B9:CC:DB:BD:8A:8A:60:FA:A9:4C:7B:B0:60:56:5B:34:CC");
      http.addHeader("Authorization", SERVER_KEY);
      http.addHeader("Host", "fcm.googleapis.com");
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Content-Length", String(data.length()));

      int respCode = http.POST(data);
      // Close connection and send message according to the response code
      http.end();
      if (respCode == 200) sendLog("Notification sent succesfully. Title: \"" + title + "\" Body: \"" + body + "\"");
      else sendLog("Error sending notification. HTTP resp code: " + String(respCode));

    } else {
      sendLog("Error sending notification, ID not found!");
    }
  }
}

/**
 * Sets the temperature of a user
 * @param serial_num, serial number of the user
 * @param value, new temperature to be set
 */
void setTemperature(String serial_num, int value) {
  if (firebase_enabled) {
    String url = "/users/" + serial_num + "/temperature";
    Firebase.setInt(url, value);
    if (Firebase.failed()) {
      // Adding } else { error_ = FirebaseError(); } in FirebaseArduino.cpp -> FirebaseCall::analyzeError, reconnects to Firebase
      Firebase.setInt("temperature", value);
      if (Firebase.failed()) sendLog("Updating temperature failed");
      else sendLog("Temperature updated");
    } else {
      sendLog("Temperature updated");
    }
  }
}


/**
 * Sets the presence of a user
 * @param serial_num, serial number of the user
 * @param value, new presence to be set
 */
void setPresence(String serial_num, String state) {
  if (firebase_enabled) {
    String url = "/users/" + serial_num + "/sensor";
    Firebase.setString(url, state);
    if (Firebase.failed()) {
      // Adding } else { error_ = FirebaseError(); } in FirebaseArduino.cpp -> FirebaseCall::analyzeError, reconnects to Firebase
      Firebase.setString(url, state);
      if (Firebase.failed()) sendLog("Updating presence failed");
      else sendLog("Presence updated");
    } else {
      sendLog("Presence updated");
    }
  }
}

/**
 * Sets the water level of a user
 * @param serial_num, serial number of the user
 * @param value, new water level to be set
 */
void setWaterLevel(String serial_num, int percentage) {
  if (firebase_enabled) {
    String url = "/users/" + serial_num + "/waterLevel";
    Firebase.setInt(url, percentage);
    if (Firebase.failed()) {
      // Adding } else { error_ = FirebaseError(); } in FirebaseArduino.cpp -> FirebaseCall::analyzeError, reconnects to Firebase
      Firebase.setInt(url, percentage);
      if (Firebase.failed()) sendLog("Updating water level failed");
      else sendLog("Water level updated");
    } else {
      sendLog("Water level updated");
    }
  }
}

/**
 * Gets the current water level of a user
 * @param serial_num, serial number of the user
 * @return int, value of the water level
 */
int getWaterLevelFirebase(String serial_num) {
  if (firebase_enabled) {
    String url = "/users/" + serial_num + "/waterLevel";
    int value = Firebase.getInt(url);
    if (Firebase.failed()) {
      // Adding } else { error_ = FirebaseError(); } in FirebaseArduino.cpp -> FirebaseCall::analyzeError, reconnects to Firebase
      value = Firebase.getInt(url);
      if (Firebase.failed()) return -1;
      else return value;
    }
    return value;
  }
}
