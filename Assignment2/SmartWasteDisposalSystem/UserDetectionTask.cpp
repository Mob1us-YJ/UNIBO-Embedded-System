#include "UserDetectionTask.h"

// Constructor: accepts the pin number of the PIR sensor
UserDetectionTask::UserDetectionTask(int pin) : pirPin(pin), userDetected(false), lastDetectedTime(0) {}

void UserDetectionTask::init() {
    pinMode(pirPin, INPUT);         // Set PIR pin as input
}

// Check if a user is detected
bool UserDetectionTask::isUserDetected() {
    int sensorValue = digitalRead(pirPin);  // Read the PIR sensor state

    // If a user is detected (PIR high level), update user detection status and time
    if (sensorValue == HIGH) {
        userDetected = true;
        lastDetectedTime = millis();
    } else {
        userDetected = false;
    }
    return userDetected;
}

// Get the last time a user was detected
unsigned long UserDetectionTask::getLastDetectedTime() {
    return lastDetectedTime;
}
