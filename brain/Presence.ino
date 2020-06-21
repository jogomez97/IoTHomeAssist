/**
   Presence TAD is able to detect fallings by setting up two PIR sensors in a high
   and low positions.
   MET06 - Grupo 2
   @authors - Joan Gomez, Jordi Malé, Toni Chico
*/

#define PRESENCE_ACK_TIME_MS  3000
#define NO_MOVE_TIME_MS       20000   // 20 seconds non detecting

// Presence cases
#define PRESENCE  "Movement"
#define FALL      "Fall"
#define NO_MOVE   "Nothing"

enum PresenceStates {
  PS_WAIT_DETECT,
  PS_ACK_DETECT,
  PS_WAIT_LEAVE,
  PS_POSSIBLE_FALL,
  PS_WAIT_FALL
};

int pinH;
int pinL;


/**
   Init of presence TAD
   @param pinHigh, pin where the higher sensor is connected
   @param pinLow, pin where the lower sensor is connected
*/
void initPresence(int pinHigh, int pinLow) {
  pinH = pinHigh;
  pinL = pinLow;
  pinMode(pinH, INPUT);
  pinMode(pinL, INPUT);
}

/**
   Motor loop for the presence sensor
*/
void loopPresence() {
  static int state = PS_WAIT_DETECT;
  static unsigned long startTime = 0;
  static int currentSensors = 0;
  static bool isUpdated = false;

  switch (state) {
    // Both sensors detect presence
    case PS_WAIT_DETECT:
      if (getActivationState() == 11) {
        state = PS_ACK_DETECT;
        startTime = millis();
      } else if (getActivationState() != 11 && millis() - startTime >= NO_MOVE_TIME_MS) {
        if (!isUpdated) {
          isUpdated = true;
          updatePresence(NO_MOVE, false);
        }
      }
      break;
    // Check if real presence
    case PS_ACK_DETECT:
      currentSensors = getActivationState();
      // Presence detected. Wait in the next state until some sensor changes
      if (millis() - startTime >= PRESENCE_ACK_TIME_MS && currentSensors == 11) {
        sendLog("Presence detected");
        state = PS_WAIT_LEAVE;
        // False positives
      } else if (millis() - startTime < PRESENCE_ACK_TIME_MS && currentSensors != 11) {
        state = PS_WAIT_DETECT;
        startTime = millis();
      } else if (millis() - startTime >= PRESENCE_ACK_TIME_MS && currentSensors != 11) {
        state = PS_WAIT_DETECT;
        startTime = millis();
      }
      break;
    // Wait until one or both sensors stop detecting presence
    case PS_WAIT_LEAVE:
      currentSensors = getActivationState();
      // No signal from both sensors or only signal from high
      if (currentSensors == 0 || currentSensors == 10) {
        updatePresence(PRESENCE, false);
        isUpdated = false;
        state = PS_WAIT_DETECT;
        startTime = millis();
        // Only receiving signal from low sensor, possible falling
      } else if (currentSensors == 1) {
        state = PS_POSSIBLE_FALL;
        startTime = millis();
      } // else if (currentSensors == 11) {} // Do nothing wait for some sensor signal to change
      break;
    // Confirm is a falling by checking the persistance of only the low sensor activation
    case PS_POSSIBLE_FALL:
      currentSensors = getActivationState();
      // Fall detected, send alarm
      if (millis() - startTime >= PRESENCE_ACK_TIME_MS && currentSensors == 1) {
        updatePresence(FALL, true);
        isUpdated = false;
        state = PS_WAIT_FALL;
        // False positives
      } else if (millis() - startTime < PRESENCE_ACK_TIME_MS && currentSensors != 1) {
        state = PS_WAIT_DETECT;
        startTime = millis();
      } else if (millis() - startTime >= PRESENCE_ACK_TIME_MS && currentSensors != 1) {
        state = PS_WAIT_DETECT;
        startTime = millis();
      }
      break;
    // Wait until status changes from fall
    case PS_WAIT_FALL:
      if (getActivationState() != 1) {
        state = PS_WAIT_DETECT;
        startTime = millis();
      }
      break;
    default:
      state = PS_WAIT_DETECT;
      startTime = millis();
      break;
  }
}

/**
 * Gets the current state of both the sensors in just 1 line of code.
 * @return int current status of the sensor.
 */
inline int getActivationState() {
  // The high sensor has a value of 10, the low sensor has a value of 1
  // We have four possible values: 0, 1, 10, 11
  return (digitalRead(pinH) == HIGH) * 10 + (digitalRead(pinL) == HIGH) * 1;
}
