#include <Arduino.h>
#include <Server.h>
#include <HCSR04.h>
#include <TempSensor.h>
using namespace std;
String str = "";

#define TEMP_PIN A0            // 温度传感器引脚
#define TRIG_PIN 2             // 超声波传感器 Trig 引脚
#define ECHO_PIN 3             // 超声波传感器 Echo 引脚
UltraSonicDistanceSensor distanceSensor(TRIG_PIN, ECHO_PIN);
TempSensor tempSensor(TEMP_PIN);

String readLine(){
  String str = "";
  while (Serial.available()){
    str += char(Serial.read());
    delay(2);
  }
  return str;
}

void setup() {
  Serial.begin(9600);
  tempSensor.init();
  //pinMode(13, OUTPUT);
}

void loop() {
  float temperature = tempSensor.getTemperature();
  float distance = distanceSensor.measureDistanceCm();
  sendSensorData(distance, temperature);
  delay(500);

}

void sendSensorData(int distance, float temperature) {
    Serial.print("TEMP:");
    Serial.print(temperature);
    Serial.print(",DIST:");
    Serial.println(distance);
}
