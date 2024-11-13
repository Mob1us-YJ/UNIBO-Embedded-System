#include "TempSensor.h"
#include "Arduino.h"

TempSensor::TempSensor(int pin) {
  this->pin = pin;
}

void TempSensor::init() {
  pinMode(pin, INPUT);
}

float TempSensor::getTemperature() {
  int analogValue = analogRead(pin);
  float voltage = analogValue * (5.0 / 1023.0); // 将模拟值转换为电压


  // 根据传感器类型计算温度（假设为LM35）
  float temperature = voltage * 100.0;

  return temperature;
}


