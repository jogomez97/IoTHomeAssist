#define DEBUG true

// Credentials
#define WIFI_SSID     "MOVISTAR_5198"
#define WIFI_PASSWORD "kMP58hNoXuYwENsaMTtt"
#define FIREBASE_HOST "met06-grupo-2-arduino.firebaseio.com"
#define FIREBASE_AUTH "bemEQ4JMytrKJAX3twchCxs1zWtoPfcdO2xgiVjR"

// Serial number
#define SERIAL_NUM    "1111-1111-1111-1112"

// Pins
#define PIN_SERVO       D1  // PWM pin: [D1..D8]
#define PIN_PULS_ALARM  D4  // Pull-up resistor
#define PIN_PULS_OPEN   D5  // Pull-up resistor
#define PIN_PULS_CLOSE  D6  // Pull-up resistor
#define PIN_ONE_WIRE    D2  

// Temperature sensor constant
#define T_REFRESH_SEC   30
#define T_MAX           28

const int ip[4] = {192, 168, 1, 220};
const int gateway[4] = {192, 168, 1, 1};
const int subnet[4] = {255, 255, 255, 0};

bool wifi_enabled = false;

void setup() {
  // Inits
  //if (initWifiStatic(WIFI_SSID, WIFI_PASSWORD, ip, gateway, subnet) < 0) wifi_enabled = false;
  if (initWifi(WIFI_SSID, WIFI_PASSWORD) < 0) wifi_enabled = false;
  else wifi_enabled = true;

  initDebugger(DEBUG, 1);
  initMotor(PIN_SERVO);
  initButtonManager();
  addButton(PIN_PULS_OPEN, openValve);
  addButton(PIN_PULS_CLOSE, closeValve);
  addButton(PIN_PULS_ALARM, sendButtonAlarm);
  initFirebase(FIREBASE_HOST, FIREBASE_AUTH, wifi_enabled);
  initTemperature(PIN_ONE_WIRE, T_REFRESH_SEC, T_MAX);

  // Welcome messages
  sendLog("\n\n");
  sendLog("---- GrandWatch ESP8266 ----");
  if (wifi_enabled) {
    String msg = "WiFi connection ready on " + getIP() + "!";
    sendLog(msg);
  } else {
    sendLog("Error configurating WiFi");
  }
}

void loop() {
  loopMotor();
  loopButtonManager();
  loopTemperature();
}

void openValve() {
  setValveOrder(1);
}

void closeValve() {
  setValveOrder(2);
}

void sendButtonAlarm() {
  sendLog("Alarm button pressed!");
  sendNotification(SERIAL_NUM, "Alarm", "Alarm button pressed");
  setValveOrder(3);
}

void updateTemperature(int value, bool sendAlarm, int threshold) {
  setTemperature(SERIAL_NUM, value);
  if (sendAlarm) {
    sendNotification(SERIAL_NUM, "Alarm", "Temperature is higher than " + String(threshold) + "ºC");
    setValveOrder(3);
  }
}
