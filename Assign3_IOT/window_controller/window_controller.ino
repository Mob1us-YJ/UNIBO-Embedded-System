#include "WindowController.h"
#include <ArduinoJson.h>
#include "LcdDisplay.h"

// Define pins
#define MOTOR_PIN 5                // Motor pin
#define MANUAL_BUTTON_PIN 7        // Manual button pin
#define POTENTIOMETER_PIN A1       // Potentiometer pin
#define LCD_ADDR 0x27              
#define LCD_COLS 16                
#define LCD_ROWS 2                 

// Create instances
ButtonImpl manualButton(MANUAL_BUTTON_PIN);                  
ServoMotorImpl servoMotor(MOTOR_PIN);                        
LcdDisplay lcdDisplay(LCD_ADDR, LCD_COLS, LCD_ROWS);         
WindowController windowController(POTENTIOMETER_PIN, &manualButton, &servoMotor, &lcdDisplay);

void setup() {
    Serial.begin(115200);          
    windowController.init();      
}



void loop() {
    Serial.setTimeout(200);  
    String receivedData = Serial.readStringUntil('\n');

    // if (receivedData.length() > 0) {  
        receivedData.trim();
        Serial.print("Received: ");
        Serial.println(receivedData);

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, receivedData);

        if (!error) {
            float temperature = doc["temperature"];
            int windowOpening = doc["window_opening"];
            String state = doc["current_state"];

            windowController.updateData(temperature, windowOpening, state);
        } else {
            Serial.println("Failed to parse JSON");
        }
    // }

    windowController.update();  // Update window controller
}




