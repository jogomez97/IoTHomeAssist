#include <Servo.h>

#define INIT_TIME_MS 4000   // Bigger than rotate time as we don't know the initial position
#define ROT_TIME_MS  1000

#define GET_POS_FIREBASE_MS 1000

// Motor MAX speeds and still
#define OPEN 180
#define CLOSE 0
#define STOP 90

#define SLOW 20 // Slows the speed of the opening and closing of the motor must be between 80 (min) and 0 (max)

#define ORDER_NO        0
#define ORDER_OPEN      1
#define ORDER_CLOSE     2
#define ORDER_CLOSE_ALL 3
#define ORDER_APP       4

enum MotorStates {
  MS_INIT_GO,
  MS_INIT_WAIT,
  MS_WAIT_ORDER,
  MS_ORDER_DELAY,
  MS_CLOSE_ALL,
  MS_CLOSE_ALL_DELAY,
  MS_ORDER_APP,
  MS_ORDER_APP_DELAY
};

enum MotorFirebaseStates {
  MFS_WAIT,
  MFS_GET_DATA
};

Servo servoMotor;
int servoOrder;
int timesOpened;
int firebaseOpened;
int maxOpen = 10;
bool listenFirebase;

void initMotor(int pin) {
  pinMode(pin, OUTPUT);
  servoMotor.attach(pin);
  servoOrder = ORDER_NO;
  timesOpened = 0;
  listenFirebase = false;
}

void initMotor(int pin, int limit) {
  pinMode(pin, OUTPUT);
  servoMotor.attach(pin);
  servoOrder = ORDER_NO;
  timesOpened = 0;
  firebaseOpened = 0;
  maxOpen = limit;
}

void loopMotor() {
  static int state = MS_INIT_GO;
  static unsigned long startTime = 0;

  switch (state) {
    // Init closing
    case MS_INIT_GO:
      servoMotor.write(CLOSE + SLOW);
      startTime = millis();
      sendLog("Closing water valve...");
      state = MS_INIT_WAIT;
      break;
    case MS_INIT_WAIT:
      if (millis() - startTime >= INIT_TIME_MS) {
        servoMotor.write(STOP);
        updateWaterLevel(0);
        sendLog("Water valve closed");
        state = MS_WAIT_ORDER;
        servoOrder = ORDER_NO;
        listenFirebase = true;
      }
      break;
    // "Loop" waiting for order and closing/opening
    case MS_WAIT_ORDER:
      switch (servoOrder) {
        case ORDER_NO: // Do nothing
          break;
        case ORDER_OPEN:
          if (timesOpened < maxOpen) {
            servoMotor.write(OPEN - SLOW);
            startTime = millis();
            state = MS_ORDER_DELAY;
            timesOpened++;
            listenFirebase = false;
          }
          break;
        case ORDER_CLOSE:
          if (timesOpened > 0) {
            servoMotor.write(CLOSE + SLOW);
            startTime = millis();
            state = MS_ORDER_DELAY;
            timesOpened--;
            listenFirebase = false;
          }
          break;
        case ORDER_CLOSE_ALL:
          state = MS_CLOSE_ALL;
          listenFirebase = false;
          break;
        case ORDER_APP:
          sendLog("Got valve order from the app");
          state = MS_ORDER_APP;
          listenFirebase = false;
          break;
        default: // Do nothing
          break;
      }
      servoOrder = ORDER_NO;
      break;
    // Execute single order
    case MS_ORDER_DELAY:
      if (millis() - startTime >= ROT_TIME_MS) {
        servoMotor.write(STOP);
        updateWaterLevel(timesOpened);
        state = MS_WAIT_ORDER;
        listenFirebase = true;
      }
      break;
    // Close for alarm all the opened times
    case MS_CLOSE_ALL:
      if (timesOpened > 0) {
        servoMotor.write(CLOSE + SLOW);
        startTime = millis();
        state = MS_CLOSE_ALL_DELAY;
        timesOpened--;
      } else {
        state = MS_WAIT_ORDER;
        sendLog("Valve has been closed due to an emergency");
        updateWaterLevel(0);
        listenFirebase = true;
      }
      break;
    case MS_CLOSE_ALL_DELAY:
      if (millis() - startTime >= ROT_TIME_MS) {
        servoMotor.write(STOP);
        state = MS_CLOSE_ALL;
      }
      break;
    case MS_ORDER_APP:
      // Need to open
      if (firebaseOpened > timesOpened) {
        timesOpened++;
        servoMotor.write(OPEN - SLOW);
        state = MS_ORDER_APP_DELAY;
        startTime = millis();
        // Need to close
      } else if (firebaseOpened < timesOpened) {
        timesOpened--;
        servoMotor.write(CLOSE + SLOW);
        state = MS_ORDER_APP_DELAY;
        startTime = millis();
      } else {
        state = MS_WAIT_ORDER;
        listenFirebase = true;
      }
      break;
    case MS_ORDER_APP_DELAY:
      if (millis() - startTime >= ROT_TIME_MS) {
        servoMotor.write(STOP);
        state = MS_ORDER_APP;
      }
      break;
    default:
      state = MS_WAIT_ORDER;
      break;
  }
}

void loopMotorFirebase() {
  static int stateFB = MFS_WAIT;
  static unsigned long startTimeFB = millis();

  switch (stateFB) {
    case  MFS_WAIT:
      if (millis() - startTimeFB >= GET_POS_FIREBASE_MS && listenFirebase) {
        stateFB = MFS_GET_DATA;
      }
      break;
    case MFS_GET_DATA:
      firebaseOpened = getWaterLevel();
      if (firebaseOpened != timesOpened) {
        if (firebaseOpened >= 0 && firebaseOpened <= 10) {
          servoOrder = ORDER_APP;
        }
      }
      stateFB = MFS_WAIT;
      startTimeFB = millis();
      break;
    default:
      stateFB = MFS_WAIT;
      startTimeFB = millis();
      break;
  }
}

void setValveOrder(int order) {
  servoOrder = order;
}
