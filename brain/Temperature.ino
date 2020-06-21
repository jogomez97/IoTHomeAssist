/**
   Measures the temperature of the digital termometer using the OneWire protocol
   MET06 - Grupo 2
   @authors - Joan Gomez, Jordi Malé, Toni Chico
*/

#include <DallasTemperature.h>
#include <OneWire.h>

enum TemperatureStates {
  TS_WAIT_TIME,
  TS_READ_VALUE,
  TS_SEND_VALUE
};

OneWire oneWire;
DallasTemperature tSensor;

int tRefreshTime;
int alarmThreshold = 40;

/**
   Init of temperature sensor TAD
   @param oneWirePin, pin where the onewire sensors will be connected
   @param refreshTimeSec, refresh time to check for temperature in seconds
   @param thresholdC, threshold for temperature alamrs in C
*/
void initTemperature(int oneWirePin, int refreshTimeSec, int thresholdC) {
  oneWire = OneWire(oneWirePin);
  tSensor = DallasTemperature(&oneWire);
  tSensor.begin();

  tRefreshTime = refreshTimeSec * 1000;    // From seconds to millis
  alarmThreshold = thresholdC;
}

/**
   Loop of the temperature sensor TAD
*/
void loopTemperature() {
  static int state = TS_READ_VALUE;
  static unsigned long startTime = 0;
  static int temperature = 25;

  switch (state) {
    // Wait for specified time
    case TS_WAIT_TIME:
      if (millis() - startTime >= tRefreshTime) {
        state = TS_READ_VALUE;
      }
      break;
    // Read the value with the one wire protocol in celsius
    case TS_READ_VALUE:
      tSensor.requestTemperatures();
      temperature = round(tSensor.getTempCByIndex(0));
      state = TS_SEND_VALUE;
      break;
    // Send Value and possible alarm
    case TS_SEND_VALUE:
      if (temperature >= alarmThreshold) updateTemperature(temperature, true, alarmThreshold);
      else updateTemperature(temperature, false, alarmThreshold);
      state = TS_WAIT_TIME;
      startTime = millis();
      break;
    default:
      state = TS_WAIT_TIME;
      startTime = millis();
      break;
  }
}
