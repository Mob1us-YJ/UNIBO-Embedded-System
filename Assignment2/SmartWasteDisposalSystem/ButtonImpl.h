#ifndef __BUTTONIMPL__
#define __BUTTONIMPL__

#include "Button.h"
#include <Arduino.h>  // Use Arduino's input/output macros

class ButtonImpl : public Button {
public: 
  ButtonImpl(int pin);
  bool isPressed();

private:
  int pin;
  bool buttonState;              // Current button state
  bool lastButtonState;          // Previous button state
  unsigned long lastDebounceTime; // Timestamp of the last state change
  static const unsigned long DEBOUNCE_DELAY = 50; // Debounce delay time in milliseconds
};

#endif
