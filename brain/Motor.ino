#include <Servo.h>

#define INIT_TIME_MS 4000   // Bigger than rotate time as we don't know the initial position
#define ROT_TIME_MS  1000

#define OPEN 180
#define CLOSE 0
#define STOP 90

#define SLOW 20 // Slows the speed of the opening and closing of the motor must be between 80 (min) and 0 (max)

#define ORDER_NO        0
#define ORDER_OPEN      1
#define ORDER_CLOSE     2
#define ORDER_CLOSE_ALL 3

enum MotorStates {
  MS_INIT_GO,
  MS_INIT_WAIT,
  MS_WAIT_ORDER,
  MS_ORDER_DELAY,
  MS_CLOSE_ALL,
  MS_CLOSE_ALL_DELAY
};

Servo servoMotor;
int servoOrder;
int timesOpened;
int max_open = 10;

void initMotor(int pin) {
  pinMode(pin, OUTPUT);
  servoMotor.attach(pin);
  servoOrder = ORDER_NO;
  timesOpened = 0;
}

void initMotor(int pin, int limit) {
  pinMode(pin, OUTPUT);
  servoMotor.attach(pin);
  servoOrder = ORDER_NO;
  timesOpened = 0;
  max_open = limit;
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
        sendLog("Water valve closed");
        state = MS_WAIT_ORDER;
        servoOrder = ORDER_NO;
      }
      break;
    // "Loop" waiting for order and closing/opening
    case MS_WAIT_ORDER:
      switch (servoOrder) {
        case ORDER_NO: // Do nothing
          break;
        case ORDER_OPEN:
          if (timesOpened < max_open) {
            servoMotor.write(OPEN - SLOW);
            startTime = millis();
            state = MS_ORDER_DELAY;
            timesOpened++;
          }
          break;
        case ORDER_CLOSE:
          if (timesOpened > 0) {
            servoMotor.write(CLOSE + SLOW);
            startTime = millis();
            state = MS_ORDER_DELAY;
            timesOpened--;
          }
          break;
        case ORDER_CLOSE_ALL:
          state = MS_CLOSE_ALL;
          break;
        default: // Do nothing
          break;
      }
      servoOrder = ORDER_NO;
      break;
    case MS_ORDER_DELAY:
      if (millis() - startTime >= ROT_TIME_MS) {
        servoMotor.write(STOP);
        state = MS_WAIT_ORDER;
      }
      break;
    case MS_CLOSE_ALL:
      if (timesOpened > 0) {
        servoMotor.write(CLOSE + SLOW);
        startTime = millis();
        state = MS_CLOSE_ALL_DELAY;
        timesOpened--;
      } else {
        state = MS_WAIT_ORDER;
        sendLog("Valve has been closed due to an emergency");
      }
      break;
    case MS_CLOSE_ALL_DELAY:
      if (millis() - startTime >= ROT_TIME_MS) {
        servoMotor.write(STOP);
        state = MS_CLOSE_ALL;
      }
      break;
    default:
      state = MS_WAIT_ORDER;
      break;
  }
}

void setValveOrder(int order) {
  servoOrder = order;
}
