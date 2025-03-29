#include "LcdDisplay.h"

LcdDisplay::LcdDisplay(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows) 
    : lcd(lcdAddr, lcdCols, lcdRows), lcdCols(lcdCols), lcdRows(lcdRows) {}

void LcdDisplay::init() {
    lcd.init();
    lcd.begin(lcdCols, lcdRows);  // Pass the correct number of columns and rows
    lcd.backlight();              // Turn on the backlight
    lcd.clear();                  // Clear the contents on the LCD
}

// Display methods for each state
void LcdDisplay::showSystemReady() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Ready:");
    lcd.setCursor(0, 1);
    lcd.print("Press Open to Enter");
}

void LcdDisplay::showWasteEntry() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door Opened:");
    lcd.setCursor(0, 1);
    lcd.print("Enter Waste");
}

void LcdDisplay::showWasteReceived() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waste Received");
    lcd.setCursor(0, 1);
    lcd.print("Thank You!");
}

void LcdDisplay::showFullContainer() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Container Full");
    lcd.setCursor(0, 1);
    lcd.print("Wait for Emptying");
}

void LcdDisplay::showOverheat() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Warning:");
    lcd.setCursor(0, 1);
    lcd.print("Overheat Detected!");
}

void LcdDisplay::showEmptying() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Emptying...");
    lcd.setCursor(0, 1);
    lcd.print("Please Wait");
}

void LcdDisplay::showSleepMode() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System in Sleep");
    lcd.setCursor(0, 1);
    lcd.print("Waiting for User");
}

void LcdDisplay::showMessage(const String &message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}
