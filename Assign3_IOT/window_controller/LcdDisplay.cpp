// #include "LcdDisplay.h"

// LcdDisplay::LcdDisplay(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows) 
//     : lcd(lcdAddr, lcdCols, lcdRows), lcdCols(lcdCols), lcdRows(lcdRows) {}

// void LcdDisplay::init() {
//     lcd.init();
//     lcd.begin(lcdCols, lcdRows);  // Pass the correct number of columns and rows
//     lcd.backlight();              // Turn on the backlight
//     lcd.clear();                  // Clear the contents on the LCD
// }

// // Display methods for each state
// void LcdDisplay::Auto_NORMAL() {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("closed");
//     lcd.setCursor(0, 1);
//     lcd.print("Auto");
// }

// void LcdDisplay::Auto_HOT(int openingLevel_auto) {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("openingLevel_auto");
//     lcd.setCursor(0, 1);
//     lcd.print("Auto");
// }

// void LcdDisplay::Auto_TOO_HOT() {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("fully open");
//     lcd.setCursor(0, 1);
//     lcd.print("Auto");
// }

// void LcdDisplay::Auto_ALARM() {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("ALARM state");
//     lcd.setCursor(0, 1);
//     lcd.print("Auto");
// }

// // void LcdDisplay::Manual() {
// //     lcd.clear();
// //     lcd.setCursor(0, 0);
// //     lcd.print("opening level:");
// //     lcd.setCursor(0, 1);
// //     lcd.print("Manual");
// // }

// // **新增** 手动模式 LCD 显示当前开度
// void LcdDisplay::Manual(int openingLevel_manual, float temperature) {
//     lcd.clear();
//     lcd.setCursor(0, 1);
//     lcd.print("Manual");
    
//     lcd.setCursor(0, 0);
//     // lcd.print("Open: ");
//     lcd.print(openingLevel_manual * 90 / 100); // 计算角度
//     lcd.print(" deg");

//     lcd.setCursor(0, 2);
//     // lcd.print("Temp: ");
//     lcd.print(temperature, 2);
// }

#include "LcdDisplay.h"
#include <Arduino.h>
#include <Wire.h>

LcdDisplay::LcdDisplay(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows) 
    : lcd(lcdAddr, lcdCols, lcdRows), cols(lcdCols), rows(lcdRows) {}

void LcdDisplay::init() {
    lcd.init();
    lcd.begin(cols, rows);
    lcd.backlight();
    lcd.clear();
}

void LcdDisplay::displayStatus(const char* mode, float temperature, int opening, const char* state) {
    lcd.clear();
    
    // first line: mode + temperature
    lcd.setCursor(0, 0);
    lcd.print(mode);
    lcd.print(" ");
    lcd.print(temperature, 1); 
    lcd.print("C");

    // second line: opening + state
    lcd.setCursor(0, 1);
    lcd.print("Open:");
    lcd.print(opening);
    lcd.print("% ");
    
    // print state
    int remainingSpace = cols - (String(opening).length() + 7); 
    if (remainingSpace > 0) {
        lcd.print(state);
    }
}