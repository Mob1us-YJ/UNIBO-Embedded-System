#include "UserDetectionTask.h"

// 构造函数：传入 PIR 传感器的引脚号
UserDetectionTask::UserDetectionTask(int pin) : pirPin(pin), userDetected(false), lastDetectedTime(0) {}

void UserDetectionTask::init() {
    pinMode(pirPin, INPUT);         // 设置 PIR 引脚为输入
}

// 检查是否检测到用户
bool UserDetectionTask::isUserDetected() {
    int sensorValue = digitalRead(pirPin);  // 读取 PIR 传感器状态

    // 若检测到用户（PIR 高电平），更新用户检测状态和时间
    if (sensorValue == HIGH) {
        userDetected = true;
        lastDetectedTime = millis();
    } else {
        userDetected = false;
    }
    return userDetected;
}

// 获取上次检测到用户的时间
unsigned long UserDetectionTask::getLastDetectedTime() {
    return lastDetectedTime;
}
