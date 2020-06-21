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

void initTemperature(int oneWirePin, int refreshTimeSec, int thresholdC) {
  oneWire = OneWire(oneWirePin);
  tSensor = DallasTemperature(&oneWire);
  tSensor.begin();

  tRefreshTime = refreshTimeSec * 1000;    // From seconds to millis
  alarmThreshold = thresholdC;
}

void loopTemperature() {
  static int state = TS_READ_VALUE;
  static unsigned long startTime = 0;
  static int temperature = 25;

  switch (state) {
    case TS_WAIT_TIME:
      if (millis() - startTime >= tRefreshTime) {
        state = TS_READ_VALUE;
      }
      break;
    case TS_READ_VALUE:
      tSensor.requestTemperatures();
      temperature = round(tSensor.getTempCByIndex(0));
      state = TS_SEND_VALUE;
      break;
    case TS_SEND_VALUE:
      // Send Value and possible alarm
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
