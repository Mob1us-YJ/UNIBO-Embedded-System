#include "WindowController.h"
#include <Arduino.h>

WindowController::WindowController(int potPin, ButtonImpl* manualButton, 
                                  ServoMotorImpl* servoMotor, LcdDisplay* lcdDisplay)
    : potentiometerPin(potPin), manualButton(manualButton),
      servoMotor(servoMotor), lcdDisplay(lcdDisplay) {
    currentState = Auto_NORMAL;
    temperature = 0.0;
    openingLevel_manual = 0;
    openingLevel_auto = 0;
    lastTransitionTime = 0;
    manualModeActive = false;
}

void WindowController::init() {
    lcdDisplay->init();
    servoMotor->on();      
    pinMode(potentiometerPin, INPUT);
    servoMotor->setPosition(0);
    updateDisplay();  // 初始显示
    Serial.println("[System] WindowController initialized");
}



void WindowController::update() {
    if (manualButton->isPressed()) {
        manualModeActive = !manualModeActive;
        lastTransitionTime = millis();
        if (manualModeActive) {
            currentState = Manual;
            Serial.println("[Mode] Switched to Manual");
        } else {
            currentState = Auto_NORMAL;
            Serial.println("[Mode] Switched to Auto");
        }
        delay(300); 
    }

    if (manualModeActive) {
        // read potentiometer value in manual mode
        int potValue = analogRead(potentiometerPin);
        openingLevel_manual = map(potValue, 0, 1023, 0, 100);
    }
    handleState();
    updateDisplay();
}



// update serial information and display
void WindowController::updateData(float temp, int opening, String state) {
    temperature = temp;
    openingLevel_auto = opening;
    if (!manualModeActive) { //not update in manual mode
        if (state == "Auto_HOT") {
            currentState = Auto_HOT;
        } else if (state == "Auto_NORMAL") {
            currentState = Auto_NORMAL;
        } else if (state == "Auto_TOO_HOT") {
            currentState = Auto_TOO_HOT;
        } else if (state == "Auto_ALARM") {
            currentState = Auto_ALARM;
        } else if (state == "Manual") {
            currentState = Manual_Dashboard;
        }
         else {
            Serial.print("[ERROR] Unknown state: ");
            Serial.println(state);
        }
    }

    // force update display despite mode
    updateDisplay();
    

    Serial.print("%, State:");
    Serial.println(currentState);
}

// FSM control
void WindowController::handleState() {
    switch (currentState) {
        case Auto_HOT:
            updateWindow(openingLevel_auto);
            break;
            
        case Auto_NORMAL:
            updateWindow(0);
            break;
            
        case Auto_TOO_HOT:
            updateWindow(100);
            break;
            
        case Auto_ALARM:
            updateWindow(0);
            break;

        case Manual_Dashboard:
            updateWindow(openingLevel_auto);
            break;

        case Manual:
            handleManualMode();
            break;
    }
}

void WindowController::handleManualMode() {
    updateWindow(openingLevel_manual);
}

void WindowController::updateWindow(int targetOpening) {
    int angle = map(targetOpening, 0, 100, 0, 90);
    servoMotor->setPosition(angle);
    
    Serial.print("[Motor] Target:");
    Serial.print(targetOpening);
    Serial.print("%, Angle:");
    Serial.println(angle);
}

// 
// WindowController.cpp
void WindowController::updateDisplay() {
    const char* states[] = {
        "HOT",      // Auto_HOT 0
        "NORMAL",   // Auto_NORMAL 1
        "TOO_HOT",  // Auto_TOO_HOT 2
        "ALARM",    // Auto_ALARM 3
        "Manual_Dashboard",
        ""          // Manual 4
    };
    
    lcdDisplay->displayStatus(
        (currentState == Manual)||(currentState == Manual_Dashboard) ? "[Manual]" : "[Auto]",
        temperature,
        (currentState == Manual) ? openingLevel_manual : openingLevel_auto,
        states[currentState]
    );
}