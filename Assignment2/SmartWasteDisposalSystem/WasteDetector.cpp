#include "WasteDetector.h"
#include "Arduino.h"

WasteDetector::WasteDetector(int trigPin, int echoPin) {
  this->trigPin = trigPin;
  this->echoPin = echoPin;
}

void WasteDetector::init() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

int WasteDetector::getWasteLevel() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // Distance calculation formula
  return distance;
}

bool WasteDetector::isFull(int threshold) {
  return getWasteLevel() < threshold; // If the distance is less than the threshold, it indicates full capacity
}
