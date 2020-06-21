/**
 * ButtonManager allows to manage multiple buttons with one single class
 * MET06 - Grupo 2
 * @authors - Joan Gomez, Jordi Malé, Toni Chico
 */

#define MAX_BUTTONS 10
#define FILTER_MS 150

enum ButtonStates {
  BS_WAIT_PRESS,
  BS_FILTER_PRESS,
  BS_WAIT_RELEASE,
  BS_FILTER_RELEASE
};

int nButtons;
int buttonArray[MAX_BUTTONS];
int buttonStates[MAX_BUTTONS];
void (*buttonFunction[MAX_BUTTONS])(void);
unsigned long buttonTimes[MAX_BUTTONS];

/**
 * Init of ButtonManager
 */
void initButtonManager() {
  nButtons = 0;
  for (int i = 0; i < MAX_BUTTONS; i++) {
    buttonArray[i] = -1;
    buttonStates[i] = BS_WAIT_PRESS;
    buttonTimes[i] = 0;
  }
}

/**
 * Loop of button manager
 */
void loopButtonManager() {
  for (int i = 0; i < nButtons; i++) {
    if (buttonArray[i] > 0) {
      loopButton(i);
    }
  }
}

/**
 * Adds a button to the button manager array to be managed.
 * @param pin, pin where the button is connected
 * @param func, function to be executed when button is released
 */
void addButton(int pin, void (*func)(void)) {
  pinMode(pin, INPUT_PULLUP);
  buttonArray[nButtons] = pin;
  buttonFunction[nButtons] = func;
  nButtons++;
}

/**
 * Loop of an individual button
 * @param pos, position inside the button manager logic
 */
void loopButton(int pos) {
  switch (buttonStates[pos]) {
    // Wait for button to be pressed
    case BS_WAIT_PRESS:
      if (digitalRead(buttonArray[pos]) == LOW) {
        buttonStates[pos] = BS_FILTER_PRESS;
        buttonTimes[pos] = millis();
      }
      break;
    // Filter bouncing
    case BS_FILTER_PRESS:
      if (millis() - buttonTimes[pos] > FILTER_MS) {
        buttonStates[pos] = BS_WAIT_RELEASE;
      }
      break;
    // Wait until button is released
    case BS_WAIT_RELEASE:
      if (digitalRead(buttonArray[pos]) == HIGH) {
        // Do button associated action
        buttonFunction[pos]();
        buttonStates[pos] = BS_FILTER_RELEASE;
        buttonTimes[pos] = millis();
      }
      break;
    // Filter bouncing
    case BS_FILTER_RELEASE:
      if (millis() - buttonTimes[pos] > FILTER_MS) {
        buttonStates[pos] = BS_WAIT_PRESS;
      }
      break;
    default:
      buttonStates[pos] = BS_WAIT_PRESS;
      break;
  }
}
