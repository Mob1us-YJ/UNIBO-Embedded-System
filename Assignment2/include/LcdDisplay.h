#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LcdDisplay {
public:
    // 构造函数：接受 LCD 地址、列数和行数
    LcdDisplay(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows);

    // 初始化 LCD
    void init();

    // 不同状态下的显示内容
    void showSystemReady();
    void showWasteEntry();
    void showWasteReceived();
    void showFullContainer();
    void showOverheat();
    void showEmptying();
    void showSleepMode();

    // 自定义显示信息
    void showMessage(const String &message);

private:
    LiquidCrystal_I2C lcd;
    uint8_t lcdCols;
    uint8_t lcdRows;
};

#endif
