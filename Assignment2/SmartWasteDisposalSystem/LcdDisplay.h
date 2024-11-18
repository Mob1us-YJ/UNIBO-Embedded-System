#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LcdDisplay {
public:
    // Constructor: accepts LCD address, column count, and row count
    LcdDisplay(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows);

    // Initialize the LCD
    void init();

    // Display content for different states
    void showSystemReady();
    void showWasteEntry();
    void showWasteReceived();
    void showFullContainer();
    void showOverheat();
    void showEmptying();
    void showSleepMode();

    // Custom display message
    void showMessage(const String &message);

private:
    LiquidCrystal_I2C lcd;
    uint8_t lcdCols;
    uint8_t lcdRows;
};

#endif
