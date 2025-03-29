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
  float voltage = analogValue * (5.0 / 1023.0); // Convert analog value to voltage

  // Calculate temperature based on sensor type (assuming LM35)
  float temperature = voltage * 100.0;

  return temperature;
}
