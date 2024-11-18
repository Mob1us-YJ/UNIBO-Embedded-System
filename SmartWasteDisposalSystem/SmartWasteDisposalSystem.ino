#include <Arduino.h>
#include "LcdDisplay.h"
#include "Button.h"
#include "ButtonImpl.h"       
#include "Led.h"
#include "WasteDetector.h"
#include "TempSensor.h"
#include "servo_motor.h"       
#include "servo_motor_impl.h"  
#include "MsgService.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>

// LCD 
#define LCD_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

// pin
#define LED_L1_PIN 8           // Green LED pin
#define LED_L2_PIN 9           // Red LED pin
#define PIR_PIN 2              // PIR sensor pin
#define TRIG_PIN 4             // Ultrasonic sensor Trig pin
#define ECHO_PIN 3             // Ultrasonic sensor Echo pin
#define TEMP_PIN A0            // Temperature sensor pin
#define SERVO_PIN 5            // Servo motor pin
#define OPEN_BUTTON_PIN 6      // Open button pin
#define CLOSE_BUTTON_PIN 7     // Close button pin

// system parameter
#define WASTE_FULL_THRESHOLD 10 // Full threshold in cm
#define MAXTEMP 24              // Maximum temperature threshold
#define MAXTEMPTIME 5000        // Maximum duration of overheat (milliseconds)
#define EMPTYING_DURATION 3000  // Emptying duration (milliseconds)
#define TSLEEP 5000             // System sleep time after inactivity (milliseconds)
#define T1 5000  // Timeout for entering Waste Received state, e.g., 5000 milliseconds
#define T2 2000  // Duration to display "WASTE RECEIVED" message, e.g., 2000 milliseconds
#define T3 3000  // Emptying duration, e.g., 3000 milliseconds
#define MAX_DISTANCE 200  // Set maximum distance according to your needs (e.g., 200 cm)


// Create instances
LcdDisplay lcdDisplay(LCD_ADDR, LCD_COLS, LCD_ROWS);
WasteDetector wasteDetector(TRIG_PIN, ECHO_PIN);
TempSensor tempSensor(TEMP_PIN);
ServoMotorImpl servoMotor(SERVO_PIN);     // Use ServoMotorImpl instance
ButtonImpl openButton(OPEN_BUTTON_PIN);   // Use ButtonImpl instance
ButtonImpl closeButton(CLOSE_BUTTON_PIN); // Use ButtonImpl instance
Led greenLed(LED_L1_PIN);
Led redLed(LED_L2_PIN);

// Define states
enum SystemState {
    SYSTEM_READY,
    WASTE_ENTRY,
    WASTE_RECEIVED,
    FULL_CONTAINER,
    OVERHEAT,
    SLEEP_MODE,
    EMPTYING
};

SystemState currentState = SYSTEM_READY; // Initial state
unsigned long lastTransitionTime = 0;    // Timer for tracking state duration
unsigned long lastUserDetectedTime = 0;  // Record last user detection time
unsigned long overheatStartTime = 0;  // Record overheat start time
int previousState;

void setup() {
    Serial.begin(9600);
    lcdDisplay.init();                // Initialize LCD
    wasteDetector.init();             // Initialize waste detector
    tempSensor.init();                // Initialize temperature sensor
    servoMotor.on();                  // Start the servo motor

    greenLed.switchOn();              // Set LED for System Ready state
    redLed.switchOff();
    lcdDisplay.showSystemReady();     // Display initial message
    servoMotor.setPosition(0);        // Close door
    lastUserDetectedTime = millis();  // Initialize last user detection time
    Serial.println("System Initialized and in SYSTEM_READY state");
}

void loop() {
    // Get sensor data
    int distance = wasteDetector.getWasteLevel();
    float temperature = tempSensor.getTemperature();
    bool userDetected = digitalRead(PIR_PIN);
    // Calculate fill percentage, ensuring it is within 0-100 range
    int fillPercentage = max(0, min(100, 100 - (distance * 100 / MAX_DISTANCE)));

    // Format message string
    String message = "FILL:" + String(fillPercentage) + "% TEMP:" + String(temperature) + "C";

    // Send data to Java GUI using MsgService
    MsgService.sendMsg(message);

    switch (currentState) {
        case SYSTEM_READY:
            handleSystemReady();
            break;

        case WASTE_ENTRY:
            handleWasteEntry();
            break;

        case WASTE_RECEIVED:
            handleWasteReceived();
            break;

        case FULL_CONTAINER:
            handleFullContainer();
            break;

        case OVERHEAT:
            handleOverheat();
            break;

        case SLEEP_MODE:
            handleSleepMode();
            break;

        case EMPTYING:
            handleEmptying();
            break;
    }
}

// Handle System Ready state
void handleSystemReady() {
    // Initial state setup, display System Ready message and turn on green LED
    greenLed.switchOn();
    redLed.switchOff();
    lcdDisplay.showSystemReady();
    servoMotor.setPosition(0); // Ensure door is closed
    previousState=0;

    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // End handling if in OVERHEAT state

    // Check for user detection (PIR sensor detects user presence)
    bool userDetected = digitalRead(PIR_PIN);

    // If user is detected
    if (userDetected) {
        lastUserDetectedTime = millis();  // Update last user detection time
        currentState = SYSTEM_READY;
        //Serial.println("userDetected");
        // Check if the open button is pressed
        if (openButton.isPressed()) {
            currentState = WASTE_ENTRY;
            lastTransitionTime = millis();
            Serial.println("Transitioning to WASTE_ENTRY");
        }
    }

    // Enter SLEEP_MODE if no user is detected for Tsleep time
    if (millis() - lastUserDetectedTime >= TSLEEP) {
        currentState = SLEEP_MODE;
        Serial.println("Transitioning to SLEEP_MODE due to inactivity");
    }
}

// Handle Waste Entry state
void handleWasteEntry() {
    lcdDisplay.showWasteEntry();
    servoMotor.setPosition(90); // Open door
    greenLed.switchOn();

    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // End handling if in OVERHEAT state

    // Check for timeout
    if (millis() - lastTransitionTime >= T1) {
        // Enter WASTE_RECEIVED state after timeout
        currentState = WASTE_RECEIVED;
        lastTransitionTime = millis();
        Serial.println("Timeout reached, transitioning to WASTE_RECEIVED");
        return;
    }

    // Check if distance sensor detects container is full
    if (wasteDetector.isFull(WASTE_FULL_THRESHOLD)) {
        // Enter FULL_CONTAINER state if container is full
        currentState = FULL_CONTAINER;
        lastTransitionTime = millis();
        Serial.println("Container full, transitioning to FULL_CONTAINER");
        return;
    }

    // Check if the close button is pressed
    if (closeButton.isPressed()) {
        // Enter WASTE_RECEIVED state when close button is pressed
        currentState = WASTE_RECEIVED;
        lastTransitionTime = millis();
        Serial.println("Close button pressed, transitioning to WASTE_RECEIVED");
    }
}

// Handle Waste Received state
void handleWasteReceived() {
    // Close the door and set motor to 0°
    servoMotor.setPosition(0); 
    lcdDisplay.showWasteReceived();
    greenLed.switchOn();  // Display green LED in Waste Received state

    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // End handling if in OVERHEAT state

    // Wait for T2 seconds
    if (millis() - lastTransitionTime < T2) {
        // Display "WASTE RECEIVED" for T2 seconds
        return;
    }

    // After T2 seconds, check if container is full
    if (wasteDetector.isFull(WASTE_FULL_THRESHOLD)) {
        // Enter FULL_CONTAINER state if container is full
        currentState = FULL_CONTAINER;
        lastTransitionTime = millis();
        Serial.println("Container full, transitioning to FULL_CONTAINER");
    } else {
        // Return to SYSTEM_READY state if container is not full
        currentState = SYSTEM_READY;
        lastTransitionTime = millis();
        Serial.println("System ready for next user, transitioning to SYSTEM_READY");
    }
}

// Handle Full Container state
void handleFullContainer() {
    greenLed.switchOff();     // Turn off green LED L1
    redLed.switchOn();        // Turn on red LED L2
    lcdDisplay.showFullContainer();  // Display "CONTAINER FULL"
    servoMotor.setPosition(0);
    previousState=1;
    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // End handling if in OVERHEAT state

    // Check for user activity (e.g., PIR sensor detects user presence)
    bool userDetected = digitalRead(PIR_PIN);

    // Reset timer if user activity is detected
    if (userDetected) {
        lastUserDetectedTime = millis();
    }

    // Enter SLEEP_MODE if no user is detected for Tsleep time
    if (millis() - lastUserDetectedTime >= TSLEEP) {
        currentState = SLEEP_MODE;
        Serial.println("Transitioning to SLEEP_MODE due to inactivity");
        return;
    }

    // Check if empty command is received (via MsgService)
    if (MsgService.isMsgAvailable()) {
        Msg* msg = MsgService.receiveMsg();  // Receive message
        if (msg->getContent() == "EMPTY") {   // Check message content
            MsgService.sendMsg("receviced");
            currentState = EMPTYING;
            lastTransitionTime = millis();
            Serial.println("Emptying triggered by GUI, transitioning to EMPTYING");
        }
        delete msg;  // Release message object
    }
}

// Handle Emptying state
void handleEmptying() {
    // Rotate servo motor to -90° to simulate emptying process
    servoMotor.setPosition(-90);
    lcdDisplay.showEmptying();
    Serial.println("Motor rotated to -90° for emptying process");

    // Check if emptying duration T3 has been reached
    if (millis() - lastTransitionTime >= T3) {
        // After emptying, close the door to 0° position
        servoMotor.setPosition(0);
        greenLed.switchOn();
        redLed.switchOff();
        currentState = SYSTEM_READY;  // Return to SYSTEM_READY state
        lastTransitionTime = millis();  // Update state transition time
        Serial.println("Emptying completed, transitioning to SYSTEM_READY");
    }
}


// Handle Overheat state
void handleOverheat() {
    redLed.switchOn();         // Turn on red LED
    greenLed.switchOff();      // Turn off green LED
    lcdDisplay.showOverheat(); // Display "PROBLEM DETECTED"
    servoMotor.setPosition(0);

    // Send overheat warning message to Java GUI
    MsgService.sendMsg("OVERHEAT_WARNING");

    // Check if restore command is received
    if (MsgService.isMsgAvailable()) {
        Msg* msg = MsgService.receiveMsg();
        if (msg->getContent() == "RESTORE") {
            currentState = SYSTEM_READY;
            lastTransitionTime = millis();
            Serial.println("System restored, transitioning to SYSTEM_READY");
            MsgService.sendMsg("RESTORE_ACK");
        }
        delete msg;  // Release message object
    }
}


void wakeUpNow() {
    
}

// Handle Sleep Mode state
void handleSleepMode() {
    // Maintain LED status prior to entering SLEEP_MODE
    if (previousState==1) {
        redLed.switchOn();      // Keep red LED on if entering from FULL_CONTAINER state
        greenLed.switchOff();
    } else {
        greenLed.switchOn();    // Otherwise, keep green LED on
        redLed.switchOff();
    }
    lcdDisplay.showSleepMode();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Set sleep mode to power down mode
    sleep_enable();  // Enable sleep mode
    
    // Enable interrupts before entering sleep
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), wakeUpNow, HIGH);  // Set interrupt to wake up on PIR_PIN detecting low level

    sleep_mode();  // Enter sleep mode

    // Code continues here after sleep ends
    sleep_disable();  // Disable sleep mode
    detachInterrupt(digitalPinToInterrupt(PIR_PIN));  // Detach interrupt

    // Detect user activity
    bool userDetected = digitalRead(PIR_PIN);
    // If user is detected, wake up system and return to SYSTEM_READY or FULL_CONTAINER
    if (userDetected) {
        if (previousState==1) {
            currentState = FULL_CONTAINER;
            Serial.println("User detected, waking up from SLEEP_MODE to FULL_CONTAINER");
        } else {
            currentState = SYSTEM_READY;
            Serial.println("User detected, waking up from SLEEP_MODE to SYSTEM_READY");
        }
        lastUserDetectedTime = millis();  // Update user detection time
    }
}


void checkOverheat(float temperature) {
    if (temperature > MAXTEMP) {
        // Record start time if temperature exceeds threshold and timing has not started
        if (overheatStartTime == 0) {
            overheatStartTime = millis();
        }
        // Enter Overheat state if maximum duration is exceeded
        else if (millis() - overheatStartTime > MAXTEMPTIME) {
            currentState = OVERHEAT;
            overheatStartTime = 0; // Reset timing
            lastTransitionTime = millis();
            Serial.println("Overheat detected, transitioning to OVERHEAT");
        }
    } else {
        // Reset overheat start time if temperature is no longer above threshold
        overheatStartTime = 0;
    }
}
