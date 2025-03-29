#ifndef __TEMP_SENSOR__
#define __TEMP_SENSOR__

class TempSensor {
public:
  TempSensor(int pin);
  void init();
  float getTemperature(); // get the temperature

private:
  int pin;
};

#endif
