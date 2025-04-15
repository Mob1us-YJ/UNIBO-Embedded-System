// #ifndef LCD_DISPLAY_H
// #define LCD_DISPLAY_H

// #include <LiquidCrystal_I2C.h>

// class LcdDisplay {
// private:
//     LiquidCrystal_I2C lcd; // 使用 I2C 接口的 LCD 对象
//     uint8_t lcdCols;       // LCD 列数
//     uint8_t lcdRows;       // LCD 行数

// public:
//     // 构造函数：初始化 LCD 地址、列数和行数
//     LcdDisplay(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows);

//     // 初始化 LCD 屏幕
//     void init();

//     // 显示 Auto_NORMAL 状态
//     void Auto_NORMAL();

//     // 显示 Auto_HOT 状态
//     void Auto_HOT(int openingLevel_auto);

//     // 显示 Auto_TOO_HOT 状态
//     void Auto_TOO_HOT();

//     // 显示 Auto_ALARM 状态
//     void Auto_ALARM();

//     // 显示 Manual 状态
//     void Manual(int openingLevel_manual, float temperature);

//     // 显示窗口的开度和温度信息
//     void showWindowStatus(int windowOpening, float temperature);

//     // // 手动模式 LCD 显示，显示当前窗户开度
//     // void showManualMode(int openingLevel);

//     // 清空显示屏内容
//     void clearDisplay();
// };

// #endif
#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LcdDisplay {
public:
    LcdDisplay(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows);
    void init();
    void displayStatus(const char* mode, float temperature, int opening, const char* state);

private:
    LiquidCrystal_I2C lcd;  
    uint8_t cols;
    uint8_t rows;
};

#endif