#ifndef WINDOW_CONTROLLER_H
#define WINDOW_CONTROLLER_H

#include <Arduino.h>
#include "ButtonImpl.h"
#include "servo_motor_impl.h"
#include "LcdDisplay.h"

// 窗口控制状态枚举
enum WindowState {
    Auto_HOT,
    Auto_NORMAL,
    Auto_TOO_HOT,
    Auto_ALARM,
    Manual_Dashboard,
    Manual
};

class WindowController {
private:
    WindowState currentState;         // current state
    int potentiometerPin;             // potentiometer pin
    ButtonImpl* manualButton;         // manual button
    ServoMotorImpl* servoMotor;       // servo motor
    LcdDisplay* lcdDisplay;           // LCD 
    float temperature;                
    int openingLevel_manual;          
    int openingLevel_auto;            
    unsigned long lastTransitionTime; 
    bool manualModeActive;            // manual mode active
    String lastReceivedState;         // last received state

    WindowState parseWindowState(String s);

    void handleState();               // control window according to state
    void handleManualMode();          // manual mode control
    void updateWindow(int targetOpening); 
    void updateDisplay();             

public:
    WindowController(int potPin, ButtonImpl* manualButton, 
                     ServoMotorImpl* servoMotor, LcdDisplay* lcdDisplay);
    void init();                    
    void update();                  
    void updateData(float temp, int opening, String state); 
};

#endif
