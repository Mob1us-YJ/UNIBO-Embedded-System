#include "ButtonImpl.h"

ButtonImpl::ButtonImpl(int pin) : pin(pin), lastButtonState(HIGH), buttonState(HIGH), lastDebounceTime(0) {
    pinMode(pin, INPUT_PULLUP);  // Set to pull-up resistor mode
}

bool ButtonImpl::isPressed() {
    bool reading = digitalRead(pin);  // Read the current button state

    // Check if the button state has changed
    if (reading != lastButtonState) {
        lastDebounceTime = millis();  // Update the timestamp of the last state change
    }

    // Check if debounce delay has passed
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // If the state is stable, update the current state
        if (reading != buttonState) {
            buttonState = reading;

            // If the button is stable in the LOW state, it means the button is pressed
            if (buttonState == LOW) {
                lastButtonState = reading;  // Update the last button state
                return true;                // Button is pressed
            }
        }
    }

    lastButtonState = reading;  // Update the last read state
    return false;               // Not pressed or in debounce
}
