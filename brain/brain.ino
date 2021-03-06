/**
 * Main class of GranWatcher
 * MET06 - Grupo 2
 * @authors - Joan Gomez, Jordi Malé, Toni Chico
 */

#define DEBUG true

// Credentials
#define WIFI_SSID     "MOVISTAR_5198"
#define WIFI_PASSWORD "kMP58hNoXuYwENsaMTtt"
#define FIREBASE_HOST "met06-grupo-2-arduino.firebaseio.com"
#define FIREBASE_AUTH "bemEQ4JMytrKJAX3twchCxs1zWtoPfcdO2xgiVjR"

// Serial number
#define SERIAL_NUM    "1111-1111-1111-1112"

// Pins
#define PIN_SERVO       D1  // PWM pin
#define PIN_ONE_WIRE    D2
#define PIN_PRESENCE_H  D7
#define PIN_PRESENCE_L  D8
#define PIN_PULS_ALARM  D4  // Pull-up resistor
#define PIN_PULS_OPEN   D5  // Pull-up resistor
#define PIN_PULS_CLOSE  D6  // Pull-up resistor

// Temperature sensor constant
#define T_REFRESH_SEC   30
#define T_MAX           29

const int ip[4] = {192, 168, 1, 220};
const int gateway[4] = {192, 168, 1, 1};
const int subnet[4] = {255, 255, 255, 0};

bool wifi_enabled = false;

/**
 * Setup with all the necessary inits
 */
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
  initPresence(PIN_PRESENCE_H, PIN_PRESENCE_L);

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

/**
 * Loop of the program
 */
void loop() {
  loopButtonManager();
  loopMotor();
  loopMotorFirebase();
  loopTemperature();
  loopPresence();
}

/**
 * Sends an order to the motor TAD to open one step
 */
void openValve() {
  setValveOrder(1);
}

/**
 * Sends an order to the motor TAD to close one step
 */
void closeValve() {
  setValveOrder(2);
}

/**
 * Sends the alarm corresponding to the pressing of the alarm button
 */
void sendButtonAlarm() {
  sendLog("Alarm button pressed!");
  sendNotification(SERIAL_NUM, "Alarm", "Alarm button pressed");
  setValveOrder(3);
}

/**
 * Calls the update of the firebase temperature and sends alarm if needed
 * @param value, value of the new temperature
 * @param sendAlarm, true if the threshold has been passed and an alarm needs to be sent
 * @param threshold, max temperature of the system
 */
void updateTemperature(int value, bool sendAlarm, int threshold) {
  setTemperature(SERIAL_NUM, value);
  if (sendAlarm) {
    sendLog("Temperature is too high");
    sendNotification(SERIAL_NUM, "Alarm", "Temperature is higher than " + String(threshold) + "ºC");
    setValveOrder(3);
  }
}

/**
 * Calls the update of the firebase presence and sends alarm if needed
 * @param state, sensor status
 * @param sendAlarm, true if alarm needs to be sent
 */
void updatePresence(String state, bool sendAlarm) {
  setPresence(SERIAL_NUM, state);
  if (sendAlarm) {
    sendLog("Falling detected!");
    sendNotification(SERIAL_NUM, "Alarm", "Falling detected");
    setValveOrder(3);
  } else {
    sendLog("Movement sensors detected: " + state);
  }
}

/**
 * Calls the update of the firebase sensor
 * @param pos, current position of the motor (valve). Must be between 0 and 10 (0% and 100%)
 */
void updateWaterLevel(int pos) {
  setWaterLevel(SERIAL_NUM, pos * 10); // *10 for %
}

/**
 * Gets the water level from firebase and converts to motor steps
 * @return int, water level represented in valve steps
 */
int getWaterLevel() {
  return getWaterLevelFirebase(SERIAL_NUM) / 10;  // From % to pos
}
