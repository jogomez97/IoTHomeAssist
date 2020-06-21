#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SERVER_KEY "key=AAAA_m05wUk:APA91bFgXESo0z90uIMlO0QrA5eHvw1jfd1EpBHGPsnsphXQzo8QYe6hUUNDSxoNdRHM4zTb8pZc03EYSZHq6VsOUy_aFMzFyciJsLzDHUcDDuq_n6XZioChoJDZSBVGUxvWUY94dmnP"

bool firebase_enabled = false;
String firebase_host;
String firebase_auth;

HTTPClient http;

void initFirebase(String host, String auth, bool enable) {
  firebase_enabled = enable;
  if (firebase_enabled) {
    firebase_host = host;
    firebase_auth = auth;
    Firebase.begin(host, auth);
  }
}

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
