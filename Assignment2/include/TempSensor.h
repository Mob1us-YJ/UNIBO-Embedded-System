#ifndef __TEMP_SENSOR__
#define __TEMP_SENSOR__

class TempSensor {
public:
  TempSensor(int pin);
  void init();
  float getTemperature(); // 获取温度值

private:
  int pin;
};

#endif
