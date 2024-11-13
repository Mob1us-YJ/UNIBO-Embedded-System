#include <Arduino.h>
#include "LcdDisplay.h"
#include "Button.h"
#include "ButtonImpl.h"       
#include "Led.h"
#include "WasteDetector.h"
#include "TempSensor.h"
#include "servo_motor.h"       
#include "servo_motor_impl.h"  
#include "MsgService.h"

// LCD 参数
#define LCD_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

// 引脚定义
#define LED_L1_PIN 8           // 绿色 LED 引脚
#define LED_L2_PIN 9           // 红色 LED 引脚
#define PIR_PIN 4              // PIR 传感器引脚
#define TRIG_PIN 2             // 超声波传感器 Trig 引脚
#define ECHO_PIN 3             // 超声波传感器 Echo 引脚
#define TEMP_PIN A0            // 温度传感器引脚
#define SERVO_PIN 5            // 伺服电机引脚
#define OPEN_BUTTON_PIN 6      // 开门按钮引脚
#define CLOSE_BUTTON_PIN 7     // 关门按钮引脚

// 系统参数
#define WASTE_FULL_THRESHOLD 10 // 满载阈值，单位为厘米
#define MAXTEMP 40              // 最大温度阈值
#define MAXTEMPTIME 5000        // 超温最大持续时间（毫秒）
#define EMPTYING_DURATION 3000  // 清空持续时间（毫秒）
#define TSLEEP 5000             // 系统在无活动后进入睡眠的时间（毫秒）
#define T1 5000  // 设置进入 Waste Received 状态的超时时间，例如 5000 毫秒
#define T2 2000  // 设置显示 "WASTE RECEIVED" 信息的持续时间，例如 2000 毫秒
#define T3 3000  // 清空持续时间，例如 3000 毫秒

// 创建实例
LcdDisplay lcdDisplay(LCD_ADDR, LCD_COLS, LCD_ROWS);
WasteDetector wasteDetector(TRIG_PIN, ECHO_PIN);
TempSensor tempSensor(TEMP_PIN);
ServoMotorImpl servoMotor(SERVO_PIN);     // 使用 ServoMotorImpl 的实例
ButtonImpl openButton(OPEN_BUTTON_PIN);   // 使用 ButtonImpl 的实例
ButtonImpl closeButton(CLOSE_BUTTON_PIN); // 使用 ButtonImpl 的实例
Led greenLed(LED_L1_PIN);
Led redLed(LED_L2_PIN);

// 定义状态
enum SystemState {
    SYSTEM_READY,
    WASTE_ENTRY,
    WASTE_RECEIVED,
    FULL_CONTAINER,
    OVERHEAT,
    SLEEP_MODE,
    EMPTYING
};

SystemState currentState = SYSTEM_READY; // 初始状态
unsigned long lastTransitionTime = 0;    // 用于跟踪状态持续时间的计时
unsigned long lastUserDetectedTime = 0;  // 记录最后检测到用户的时间
unsigned long overheatStartTime = 0;  // 记录超温开始时间

void setup() {
    Serial.begin(9600);
    lcdDisplay.init();                // 初始化 LCD
    wasteDetector.init();             // 初始化废物检测传感器
    tempSensor.init();                // 初始化温度传感器
    servoMotor.on();                  // 启动伺服电机

    greenLed.switchOn();              // 设置 System Ready 状态的 LED
    redLed.switchOff();
    lcdDisplay.showSystemReady();     // 显示初始信息
    servoMotor.setPosition(0);        // 关闭门
    lastUserDetectedTime = millis();  // 初始化最后用户检测时间
    Serial.println("System Initialized and in SYSTEM_READY state");
}

void loop() {
    // 获取传感器数据
    int distance = wasteDetector.getWasteLevel();
    float temperature = tempSensor.getTemperature();
    bool userDetected = digitalRead(PIR_PIN);
    // 向 GUI 发送温度和距离信息
    sendSensorData(distance, temperature);

    switch (currentState) {
        case SYSTEM_READY:
            handleSystemReady();
            break;

        case WASTE_ENTRY:
            handleWasteEntry();
            break;

        case WASTE_RECEIVED:
            handleWasteReceived();
            break;

        case FULL_CONTAINER:
            handleFullContainer();
            break;

        case OVERHEAT:
            handleOverheat();
            break;

        case SLEEP_MODE:
            handleSleepMode();
            break;

        case EMPTYING:
            handleEmptying();
            break;
    }

    if (MsgService.isMsgAvailable()) {
        Msg* msg = MsgService.receiveMsg();
        String content = msg->getContent();
        delete msg;

        if (content == "EMPTY") {
            emptyContainer();
        } else if (content == "RESTORE") {
            restoreSystem();
        }
    }
}

//------------------------GUI-----------------------------------
// 向 GUI 发送温度和距离信息
void sendSensorData(int distance, float temperature) {
    Serial.print("TEMP:");
    Serial.print(temperature);
    Serial.print(",DIST:");
    Serial.println(distance);
}

// 清空垃圾桶
void emptyContainer() {
    if (currentState == FULL_CONTAINER || currentState == SYSTEM_READY) {
        currentState = EMPTYING;
        lastTransitionTime = millis();
        Serial.println("Emptying triggered by GUI, transitioning to EMPTYING");
    }
}
// 重启垃圾桶
void restoreSystem() {
    if (currentState == OVERHEAT) {
        currentState = SYSTEM_READY;
        lastTransitionTime = millis();
        Serial.println("System restored, transitioning to SYSTEM_READY");
    }
}
//-------------------------STATE--------------------------
// 处理 System Ready 状态
void handleSystemReady() {
    // 初始状态设置，显示 System Ready 信息并打开绿色 LED
    greenLed.switchOn();
    redLed.switchOff();
    lcdDisplay.showSystemReady();
    servoMotor.setPosition(0); // 确保门关闭

    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // 若进入 OVERHEAT 状态，则结束处理

    // 检查用户检测（PIR 传感器是否检测到用户）
    bool userDetected = digitalRead(PIR_PIN);

    // 如果检测到用户
    if (userDetected) {
        lastUserDetectedTime = millis();  // 更新最后检测到用户的时间
        currentState = SYSTEM_READY;
        // 检查开门按钮是否按下
        if (openButton.isPressed()) {
            currentState = WASTE_ENTRY;
            lastTransitionTime = millis();
            Serial.println("Transitioning to WASTE_ENTRY");
        }
    }

    // 如果超过 Tsleep 时间未检测到用户，进入 SLEEP_MODE 状态
    if (millis() - lastUserDetectedTime >= TSLEEP) {
        currentState = SLEEP_MODE;
        Serial.println("Transitioning to SLEEP_MODE due to inactivity");
    }
}

// 处理 Waste Entry 状态
void handleWasteEntry() {
    lcdDisplay.showWasteEntry();
    servoMotor.setPosition(90); // 打开门
    greenLed.switchOn();

    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // 若进入 OVERHEAT 状态，则结束处理

    // 检查是否超时
    if (millis() - lastTransitionTime >= T1) {
        // 超时进入 WASTE_RECEIVED 状态
        currentState = WASTE_RECEIVED;
        lastTransitionTime = millis();
        Serial.println("Timeout reached, transitioning to WASTE_RECEIVED");
        return;
    }

    // 检查距离传感器是否检测到容器已满
    if (wasteDetector.isFull(WASTE_FULL_THRESHOLD)) {
        // 检测到容器满，进入 FULL_CONTAINER 状态
        currentState = FULL_CONTAINER;
        lastTransitionTime = millis();
        Serial.println("Container full, transitioning to FULL_CONTAINER");
        return;
    }

    // 检查是否按下 Close 按钮
    if (closeButton.isPressed()) {
        // 按下关闭按钮，进入 WASTE_RECEIVED 状态
        currentState = WASTE_RECEIVED;
        lastTransitionTime = millis();
        Serial.println("Close button pressed, transitioning to WASTE_RECEIVED");
    }
}

// 处理 Waste Received 状态
void handleWasteReceived() {
    // 关闭门并将电机设置到 0° 位置
    servoMotor.setPosition(0); 
    lcdDisplay.showWasteReceived();
    greenLed.switchOn();  // 在 Waste Received 状态下显示绿色 LED

    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // 若进入 OVERHEAT 状态，则结束处理

    // 等待 T2 秒
    if (millis() - lastTransitionTime < T2) {
        // T2 秒内保持显示 "WASTE RECEIVED"
        return;
    }

    // T2 秒后，检查容器是否已满
    if (wasteDetector.isFull(WASTE_FULL_THRESHOLD)) {
        // 如果容器已满，则进入 FULL_CONTAINER 状态
        currentState = FULL_CONTAINER;
        lastTransitionTime = millis();
        Serial.println("Container full, transitioning to FULL_CONTAINER");
    } else {
        // 容器未满，返回到 SYSTEM_READY 状态
        currentState = SYSTEM_READY;
        lastTransitionTime = millis();
        Serial.println("System ready for next user, transitioning to SYSTEM_READY");
    }
}

// 处理 Full Container 状态
void handleFullContainer() {
    greenLed.switchOff();     // 关闭绿色 LED L1
    redLed.switchOn();        // 点亮红色 LED L2
    lcdDisplay.showFullContainer();  // 显示 "CONTAINER FULL"

    checkOverheat(tempSensor.getTemperature());

    if (currentState == OVERHEAT) return; // 若进入 OVERHEAT 状态，则结束处理

    // 检查用户活动（例如 PIR 传感器是否检测到用户）
    bool userDetected = digitalRead(PIR_PIN);

    // 如果检测到用户活动，重置计时
    if (userDetected) {
        lastUserDetectedTime = millis();
    }

    // 如果超过 Tsleep 时间没有检测到用户，进入 SLEEP_MODE 状态
    if (millis() - lastUserDetectedTime >= TSLEEP) {
        currentState = SLEEP_MODE;
        Serial.println("Transitioning to SLEEP_MODE due to inactivity");
        return;
    }

    // 检查是否收到清空命令（通过 MsgService）
    if (MsgService.isMsgAvailable()) {
        Msg* msg = MsgService.receiveMsg();  // 接收消息
        if (msg->getContent() == "EMPTY") {   // 检查消息内容
            currentState = EMPTYING;
            lastTransitionTime = millis();
            Serial.println("Emptying triggered by GUI, transitioning to EMPTYING");
        }
        delete msg;  // 释放消息对象
    }
}

// 处理 Emptying 状态
void handleEmptying() {
    // 将伺服电机旋转到 -90° 以模拟清空过程
    servoMotor.setPosition(-90);
    lcdDisplay.showEmptying();
    Serial.println("Motor rotated to -90° for emptying process");

    // 检查是否已达到清空的持续时间 T3
    if (millis() - lastTransitionTime >= T3) {
        // 清空完成，将门关闭回到 0° 位置
        servoMotor.setPosition(0);
        greenLed.switchOn();
        redLed.switchOff();
        currentState = SYSTEM_READY;  // 返回到 SYSTEM_READY 状态
        lastTransitionTime = millis();  // 更新状态转换时间
        Serial.println("Emptying completed, transitioning to SYSTEM_READY");
    }
}


// 处理 Overheat 状态
void handleOverheat() {
    redLed.switchOn();         // 打开红色 LED
    greenLed.switchOff();      // 关闭绿色 LED
    lcdDisplay.showOverheat(); // 显示 "PROBLEM DETECTED"
    servoMotor.setPosition(0);

    // 检查是否收到恢复命令
    if (MsgService.isMsgAvailable()) {
        Msg* msg = MsgService.receiveMsg();
        if (msg->getContent() == "RESTORE") {
            currentState = SYSTEM_READY;
            lastTransitionTime = millis();
            Serial.println("System restored, transitioning to SYSTEM_READY");
        }
        delete msg;  // 释放消息对象
    }
}


// 处理 Sleep Mode 状态
void handleSleepMode() {
    // 保持进入 SLEEP_MODE 前的 LED 状态
    if (currentState == FULL_CONTAINER) {
        redLed.switchOn();      // 如果在 FULL_CONTAINER 状态进入，则保持红色 LED 亮
        greenLed.switchOff();
    } else {
        greenLed.switchOn();    // 其他情况下保持绿色 LED 亮
        redLed.switchOff();
    }

    lcdDisplay.showSleepMode();  // 显示 "Sleep Mode"

    // 检测用户活动
    bool userDetected = digitalRead(PIR_PIN);

    // 如果检测到用户，则唤醒系统并返回到 SYSTEM_READY 或 FULL_CONTAINER
    if (userDetected) {
        if (currentState == FULL_CONTAINER) {
            currentState = FULL_CONTAINER;
            Serial.println("User detected, waking up from SLEEP_MODE to FULL_CONTAINER");
        } else {
            currentState = SYSTEM_READY;
            Serial.println("User detected, waking up from SLEEP_MODE to SYSTEM_READY");
        }
        lastUserDetectedTime = millis();  // 更新用户检测时间
    }
}


void checkOverheat(float temperature) {
    if (temperature > MAXTEMP) {
        // 如果温度超过阈值并且还没有开始计时，记录开始时间
        if (overheatStartTime == 0) {
            overheatStartTime = millis();
        }
        // 如果超过最大持续时间，进入 Overheat 状态
        else if (millis() - overheatStartTime > MAXTEMPTIME) {
            currentState = OVERHEAT;
            overheatStartTime = 0; // 重置计时
            lastTransitionTime = millis();
            Serial.println("Overheat detected, transitioning to OVERHEAT");
        }
    } else {
        // 如果温度不再超过阈值，重置超温开始时间
        overheatStartTime = 0;
    }
}
