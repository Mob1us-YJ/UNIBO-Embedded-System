#ifndef USER_DETECTION_TASK_H
#define USER_DETECTION_TASK_H

#include <Arduino.h>

class UserDetectionTask {
public:
    // Constructor, accepts PIR sensor pin
    UserDetectionTask(int pirPin);

    // Initialize the task, set sensor pin mode
    void init();

    // Update task, check if a user is detected
    bool isUserDetected();

    // Get the most recent time a user was detected
    unsigned long getLastDetectedTime();

private:
    int pirPin;                     // PIR sensor pin
    bool userDetected;              // Whether a user is currently detected
    unsigned long lastDetectedTime; // Last time a user was detected
};

#endif
